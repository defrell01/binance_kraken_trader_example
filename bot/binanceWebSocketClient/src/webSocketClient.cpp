#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <bot/binanceWebSocketClient/webSocketClient.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

using namespace boost;
using beast_tcp_stream = beast::tcp_stream;
using beast_ssl_stream = beast::ssl_stream<beast_tcp_stream>;
using websocket_stream = beast::websocket::stream<beast_ssl_stream>;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

WebSocketClient::WebSocketClient(asio::io_context& ioc,
						   asio::ssl::context& ssl_ctx,
						   std::shared_ptr<OrderBook> orderBook)
	: io_context_(ioc)
	, ws_(beast_ssl_stream(beast_tcp_stream(ioc), ssl_ctx))
	, orderBook_(orderBook)
{
	ws_.binary(false);
}

asio::awaitable<void> WebSocketClient::connect(const std::string& host,
									  const std::string& port,
									  const std::string& target)
{
	try
	{
		tcp::resolver resolver(io_context_);
		auto const results = co_await resolver.async_resolve(host, port, asio::use_awaitable);

		auto& lowest_layer = beast::get_lowest_layer(ws_);
		for(auto const& endpoint : results)
		{
			try
			{
				co_await lowest_layer.async_connect(endpoint, asio::use_awaitable);
				break;
			}
			catch(const std::exception& ex)
			{
				spdlog::error("Exception {}", ex.what());
			}
		}
		co_await ws_.next_layer().async_handshake(asio::ssl::stream_base::client,
										  asio::use_awaitable);

		co_await ws_.async_handshake(host, target, asio::use_awaitable);

          spdlog::info("Connected to {} {}", host, target);
	}
	catch(const std::exception& e)
	{
          spdlog::error("Connection err {}", e.what());
	}
}

asio::awaitable<void> WebSocketClient::read_loop()
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
               auto start = std::chrono::steady_clock::now();
			orderBook_->updateOrderBook(json_msg);
               auto end = std::chrono::steady_clock::now();
               auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
               if (durationUs > 50)
               {
                    spdlog::warn("OrderBook update latency: {} us", durationUs);
               }
		}
	}
	catch(const std::exception& e)
	{
          spdlog::error("Read loop error: {}", e.what());
	}
}

asio::awaitable<void> WebSocketClient::write(const std::string& message)
{
	try
	{
		co_await ws_.async_write(asio::buffer(message), asio::use_awaitable);
	}
	catch(const std::exception& e)
	{
          spdlog::error("Write error: {}", e.what());
	}
}
