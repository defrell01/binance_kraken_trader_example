#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <bot/binanceWebSocketClient/webSocketClient.hpp>
#include <bot/krakenWebSocketClient/krakenWebSocketClient.hpp>
#include <bot/orderBook/orderBook.hpp>
#include <bot/marketMakingEngine/MarketMakingEngine.hpp>
#include <bot/arbitrageEngine/ArbitrageEngine.hpp>
#include <bot/logger/logger.hpp>
#include <iostream>

using namespace boost;

int main()
{
	try
	{
          initLogger();

		auto binanceOrderBook = std::make_shared<OrderBook>();
		auto krakenOrderBook = std::make_shared<OrderBook>();

		asio::io_context ioc;
		asio::ssl::context ssl_ctx(asio::ssl::context::tlsv12_client);

		ssl_ctx.set_default_verify_paths();
		ssl_ctx.set_verify_mode(asio::ssl::verify_none);

		WebSocketClient binanceClient(ioc, ssl_ctx, binanceOrderBook);
		KrakenWebSocketClient krakenClient(ioc, ssl_ctx, krakenOrderBook);

		spdlog::info("Starting Binance connection...");
          asio::co_spawn(
          ioc,
          [&binanceClient]() -> asio::awaitable<void> {
               try {
                    spdlog::info("Attempting to connect to Binance...");
                    co_await binanceClient.connect("fstream.binance.com", "443", "/ws/btcusdt@depth5@100ms");
                    co_await binanceClient.read_loop();
               } catch (const std::exception& e) {
                    spdlog::error("Binance connection failed: {}", e.what());
               }
               co_return;
          },
          asio::detached);

          spdlog::info("Starting Kraken connection...");
          asio::co_spawn(
          ioc,
          [&krakenClient]() -> asio::awaitable<void> {
               try {
                    spdlog::info("Attempting to connect to Kraken...");
                    co_await krakenClient.connect("ws.kraken.com", "443", "/v2");
                    co_await krakenClient.read_loop();
               } catch (const std::exception& e) {
                    spdlog::error("Kraken connection failed: {}", e.what());
               }
               co_return;
          },
          asio::detached);


		ArbitrageEngine arbitrageEngine(binanceOrderBook, krakenOrderBook);
		MarketMakingEngine marketMakingEngine(binanceOrderBook);

		asio::co_spawn(ioc, arbitrageEngine.run(), asio::detached);
		asio::co_spawn(ioc, marketMakingEngine.run(), asio::detached);

		spdlog::info("Running IO context...");
          ioc.run();
          spdlog::info("IO context finished...");

	}
	catch(const std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
