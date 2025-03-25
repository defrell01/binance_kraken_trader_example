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
                    spdlog::info("✅ Connected to: {} : {}{}", host, port, target);
				break;
			}
			catch(const std::exception& ex)
			{
                    spdlog::error("Connect err {}", ex.what());
			}
		}

		// Установка SNI (для SSL рукопожатия)
		if(!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host.c_str()))
			throw beast::system_error(beast::error_code(static_cast<int>(::ERR_get_error()),
											    asio::error::get_ssl_category()),
								 "Failed to set SNI Hostname");

		co_await ws_.next_layer().async_handshake(asio::ssl::stream_base::client,
										  asio::use_awaitable);

          spdlog::info("✅ SSL handshake successful");

		try
		{
               spdlog::info("Attempting to perform WebSocket handshake with host: {} and target {}", host, target);

			co_await ws_.async_handshake(host, target, asio::use_awaitable);
			spdlog::info("✅ WebSocket handshake successful");
		}
		catch(const std::exception& e)
		{
               spdlog::error("Error attempting to perform WebSocket handshake with host: {} and target {}", host, target);    
		}

          spdlog::info("✅ Connected to Kraken WS");

		nlohmann::json subscribe_msg = {
			{"method", "subscribe"},
			{"params", {{"channel", "book"}, {"symbol", nlohmann::json::array({"BTC/USD"})}}}};

		co_await write(subscribe_msg.dump());
          spdlog::info("✅ Subscribed to XBT/USD order book on Kraken");
	}
	catch(const std::exception& e)
	{
          spdlog::error("Kraken connection error {}", e.what());
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

			if(json_msg.is_discarded())
			{
                    spdlog::error("Invalid Kraken JSON: {}", msg);
				continue;
			}

			if(json_msg.contains("type") & json_msg["channel"] != "status")
			{
				auto type = json_msg["type"].get<std::string>();
				if(type == "snapshot")
				{
					auto& bids = json_msg["data"][0]["bids"];
					auto& asks = json_msg["data"][0]["asks"];

					orderBook_->clear();
					for(const auto& bid : bids)
					{
                              auto start = std::chrono::steady_clock::now();
						
                              orderBook_->updateBid(bid["price"].get<double>(),
										  bid["qty"].get<double>());
                              
                              auto end = std::chrono::steady_clock::now();
                              auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                              if (durationUs > 50)
                              {
                                   spdlog::warn("OrderBook update latency: {} us", durationUs);
                              }                      

					}
					for(const auto& ask : asks)
					{
                              auto start = std::chrono::steady_clock::now();

						orderBook_->updateAsk(ask["price"].get<double>(),
										  ask["qty"].get<double>());

                              auto end = std::chrono::steady_clock::now();
                              auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                              if (durationUs > 50)
                              {
                                   spdlog::warn("OrderBook update latency: {} us", durationUs);
                              }
                              
					}
				}
				else if(type == "update")
				{
					auto& bids = json_msg["data"][0]["bids"];
					auto& asks = json_msg["data"][0]["asks"];

					for(const auto& bid : bids)
					{
                              auto start = std::chrono::steady_clock::now();

						orderBook_->updateBid(bid["price"].get<double>(),
										  bid["qty"].get<double>());

                              auto end = std::chrono::steady_clock::now();
                              auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                              if (durationUs > 50)
                              {
                                   spdlog::warn("OrderBook update latency: {} us", durationUs);
                              }
					}
					for(const auto& ask : asks)
					{
                              auto start = std::chrono::steady_clock::now();

						orderBook_->updateAsk(ask["price"].get<double>(),
										  ask["qty"].get<double>());

                              auto end = std::chrono::steady_clock::now();
                              auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                              if (durationUs > 50)
                              {
                                   spdlog::warn("OrderBook update latency: {} us", durationUs);
                              }
					}
				}
			}
		}
	}
	catch(const std::exception& e)
	{
          spdlog::error("Kraken read loop error {}", e.what());
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
          spdlog::error("Kraken WS write error: {}", e.what());
	}
}
