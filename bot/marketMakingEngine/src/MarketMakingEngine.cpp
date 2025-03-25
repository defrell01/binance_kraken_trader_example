#include <bot/marketMakingEngine/MarketMakingEngine.hpp>

MarketMakingEngine::MarketMakingEngine(std::shared_ptr<OrderBook> book)
	: book_(book)
{ }

boost::asio::awaitable<void> MarketMakingEngine::run()
{
	using namespace std::chrono_literals;
	while(true)
	{
		simulateMarketMaking_();
		boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
          timer.expires_after(500ms);
          co_await timer.async_wait(boost::asio::use_awaitable);
	}
}

void MarketMakingEngine::simulateMarketMaking_()
{
	double bestBid = book_->getBestBid();
	double bestAsk = book_->getBestAsk();

	if(bestBid == 0.0 || bestAsk == 0.0)
		return;

	double midPrice = (bestBid + bestAsk) / 2.0;
	double myBid = midPrice - spread / 2.0;
	double myAsk = midPrice + spread / 2.0;

     spdlog::info("📊 Market Making: place BID at {} and ASK at {} with size {}", myBid, myAsk, orderSize);
}
