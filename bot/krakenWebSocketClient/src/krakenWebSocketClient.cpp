#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <bot/krakenWebSocketClient/krakenWebSocketClient.hpp>
#include <iostream>

using namespace boost;
using beast_tcp_stream = beast::tcp_stream;
using beast_ssl_stream = beast::ssl_stream<beast_tcp_stream>;
using websocket_stream = beast::websocket::stream<beast_ssl_stream>;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

KrakenWebSocketClient::KrakenWebSocketClient(asio::io_context& ioc,
									asio::ssl::context& ssl_ctx,
									std::shared_ptr<OrderBook> orderBook)
	: io_context_(ioc)
	, ws_(beast_ssl_stream(beast_tcp_stream(ioc), ssl_ctx))
	, orderBook_(orderBook)
{
	ws_.binary(false);
}

asio::awaitable<void> KrakenWebSocketClient::connect(const std::string& host,
										   const std::string& port,
										   const std::string& target)
{
	try
	{
		tcp::resolver resolver(io_context_);
		auto results = co_await resolver.async_resolve(host, port, asio::use_awaitable);
		auto& lowest_layer = beast::get_lowest_layer(ws_);

		for(auto const& endpoint : results)
		{
			try
			{
				co_await lowest_layer.async_connect(endpoint, asio::use_awaitable);
				std::cout << "✅ Connected to: " << host << ":" << port << target << std::endl;
				break;
			}
			catch(const std::exception& ex)
			{
				std::cerr << "Connect error: " << ex.what() << "\n";
			}
		}

		// Установка SNI (для SSL рукопожатия)
		if(!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str()))
			throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
											    asio::error::get_ssl_category()),
								 "Failed to set SNI Hostname");

		co_await ws_.next_layer().async_handshake(asio::ssl::stream_base::client,
										  asio::use_awaitable);

		std::cout << "✅ SSL handshake successful" << std::endl;

		try
		{
			std::cout << "Attempting to perform WebSocket handshake with host: " << host
					<< " and target: " << target << std::endl;
			co_await ws_.async_handshake(host, target, asio::use_awaitable);
			std::cout << "✅ WebSocket handshake successful" << std::endl;
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error during WebSocket handshake with host: " << host
					<< " and target: " << target << ": " << e.what() << std::endl;
		}

		std::cout << "✅ Connected to Kraken WS " << host << target << std::endl;

		// Подписка на канал
		nlohmann::json subscribe_msg = {
			{"method", "subscribe"},
			{"params", {{"channel", "book"}, {"symbol", nlohmann::json::array({"BTC/USD"})}}}};

		co_await write(subscribe_msg.dump());
		std::cout << "✅ Subscribed to XBT/USD order book on Kraken" << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Kraken connection error: " << e.what() << std::endl;
	}
}

asio::awaitable<void> KrakenWebSocketClient::read_loop()
{
	try
	{
		beast::flat_buffer buffer;
		for(;;)
		{
			buffer.clear();
			co_await ws_.async_read(buffer, asio::use_awaitable);
			std::string msg(beast::buffers_to_string(buffer.data()));

			auto json_msg = nlohmann::json::parse(msg, nullptr, false);

			std::cout << "kraken msg: " << json_msg.dump(-1) << "\n";

			if(json_msg.is_discarded())
			{
				std::cerr << "Invalid Kraken JSON: " << msg << std::endl;
				continue;
			}

			if(json_msg.contains("type") & json_msg["channel"] != "status")
			{
				auto type = json_msg["type"].get<std::string>();
				if(type == "snapshot")
				{
					auto& bids = json_msg["data"]["bids"];
					auto& asks = json_msg["data"]["asks"];

					orderBook_->clear();
					for(const auto& bid : bids)
					{
						orderBook_->update_bid(std::stod(bid[0].get<std::string>()),
										   std::stod(bid[1].get<std::string>()));
					}
					for(const auto& ask : asks)
					{
						orderBook_->update_ask(std::stod(ask[0].get<std::string>()),
										   std::stod(ask[1].get<std::string>()));
					}
				}
				else if(type == "update")
				{
					auto& bids = json_msg["data"]["bids"];
					auto& asks = json_msg["data"]["asks"];

					for(const auto& bid : bids)
					{
						orderBook_->update_bid(std::stod(bid[0].get<std::string>()),
										   std::stod(bid[1].get<std::string>()));
					}
					for(const auto& ask : asks)
					{
						orderBook_->update_ask(std::stod(ask[0].get<std::string>()),
										   std::stod(ask[1].get<std::string>()));
					}
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << "Kraken read loop error: " << e.what() << std::endl;
	}
}

asio::awaitable<void> KrakenWebSocketClient::write(const std::string& message)
{
	try
	{
		co_await ws_.async_write(asio::buffer(message), asio::use_awaitable);
	}
	catch(const std::exception& e)
	{
		std::cerr << "Kraken WS write error: " << e.what() << std::endl;
	}
}
