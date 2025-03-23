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

		// Загружаем корневые сертификаты (если нужно)
		ssl_ctx.set_default_verify_paths();
		ssl_ctx.set_verify_mode(asio::ssl::verify_peer);

		WebSocketClient client(ioc, ssl_ctx);

		asio::co_spawn(
			ioc,
			[&client]() -> asio::awaitable<void> {
				co_await client.connect("echo.websocket.events", "443", "/");
				co_await client.write("Hello from Boost.Beast SSL WebSocket!");
				co_await client.read_loop(); // будет бесконечно читать
				co_return;
			},
			asio::detached);

		ioc.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
