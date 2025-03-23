#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <bot/binanceWebSocketClient/webSocketClient.hpp>
#include <bot/krakenWebSocketClient/krakenWebSocketClient.hpp>
#include <bot/order_book/orderBook.hpp>
#include <iostream>

using namespace boost;

int main()
{
	try
	{
		auto binanceOrderBook = std::make_shared<OrderBook>();
		auto krakenOrderBook = std::make_shared<OrderBook>();

		asio::io_context ioc;
		asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);

		ssl_ctx.set_default_verify_paths();
		ssl_ctx.set_verify_mode(asio::ssl::verify_none);

		WebSocketClient binanceClient(ioc, ssl_ctx, binanceOrderBook);
		KrakenWebSocketClient krakenClient(ioc, ssl_ctx, krakenOrderBook);

		asio::co_spawn(
			ioc,
			[&binanceClient]() -> asio::awaitable<void> {
				co_await binanceClient.connect(
					"fstream.binance.com", "443", "/ws/btcusdt@depth5@100ms");
				co_await binanceClient.read_loop();
				co_return;
			},
			asio::detached);

		asio::co_spawn(
			ioc,
			[&krakenClient]() -> asio::awaitable<void> {
				co_await krakenClient.connect("ws.kraken.com", "443", "/v2");
				co_await krakenClient.read_loop();
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
