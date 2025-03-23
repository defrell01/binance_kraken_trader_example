
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <iostream>
#include <web_client/webSocketClient/webSocketClient.hpp>

using namespace boost;

int main()
{
	try
	{
		asio::io_context ioc;
		asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);
          
		ssl_ctx.set_default_verify_paths();
		// ssl_ctx.set_verify_mode(asio::ssl::verify_peer);
          ssl_ctx.set_verify_mode(asio::ssl::verify_none);

		WebSocketClient client(ioc, ssl_ctx);

		asio::co_spawn(
			ioc,
			[&client]() -> asio::awaitable<void> {
				co_await client.connect("fstream.binance.com", "443", "/ws/btcusdt@bookTicker");

				std::string subscribe_message = R"({"method": "SUBSCRIBE", "params": ["btcusdt@bookTicker"], "id": 1})";
				co_await client.write(subscribe_message);

				co_await client.read_loop();
				co_return;
			},
			asio::detached);

		ioc.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}