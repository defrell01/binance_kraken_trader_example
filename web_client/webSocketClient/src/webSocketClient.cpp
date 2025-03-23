#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <web_client/webSocketClient/webSocketClient.hpp>

using namespace boost;
using beast_tcp_stream = beast::tcp_stream;
using beast_ssl_stream = beast::ssl_stream<beast_tcp_stream>;
using websocket_stream = beast::websocket::stream<beast_ssl_stream>;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

WebSocketClient::WebSocketClient(asio::io_context& ioc, asio::ssl::context& ssl_ctx)
	: io_context_(ioc)
	, ws_(beast_ssl_stream(beast_tcp_stream(ioc), ssl_ctx))
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
		for (auto const& endpoint : results)
		{
			try
			{
				co_await lowest_layer.async_connect(endpoint, asio::use_awaitable);
				break;
			}
			catch (const std::exception& ex)
			{
                    std::cerr << "Exception accured << " << ex.what << "\n";
			}
		}
		co_await ws_.next_layer().async_handshake(asio::ssl::stream_base::client, asio::use_awaitable);

		co_await ws_.async_handshake(host, target, asio::use_awaitable);

		std::cout << "Connected to " << host << target << std::endl;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Connection error: " << e.what() << std::endl;
	}
}

asio::awaitable<void> WebSocketClient::read_loop()
{
	try
	{
		beast::flat_buffer buffer;
		for (;;)
		{
			buffer.clear();
			co_await ws_.async_read(buffer, asio::use_awaitable);
			std::string msg(beast::buffers_to_string(buffer.data()));

			std::cout << "Received: " << msg << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Read loop error: " << e.what() << std::endl;
	}
}

asio::awaitable<void> WebSocketClient::write(const std::string& message)
{
	try
	{
		co_await ws_.async_write(asio::buffer(message), asio::use_awaitable);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Write error: " << e.what() << std::endl;
	}
}
