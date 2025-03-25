#include <bot/arbitrageEngine/ArbitrageEngine.hpp>
#include <chrono>
#include <iostream>
#include <thread>

ArbitrageEngine::ArbitrageEngine(std::shared_ptr<OrderBook> book1,
						   std::shared_ptr<OrderBook> book2)
	: book1_(book1)
	, book2_(book2)
{ }

boost::asio::awaitable<void> ArbitrageEngine::run()
{
	using namespace std::chrono_literals;
	while(true)
	{
		checkArbitrage_();
		boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor);
          timer.expires_after(500ms);
          co_await timer.async_wait(boost::asio::use_awaitable);
	}
}

void ArbitrageEngine::checkArbitrage_()
{
	double b1Bid = book1_->getBestBid();
	double b1Ask = book1_->getBestAsk();
	double b2Bid = book2_->getBestBid();
	double b2Ask = book2_->getBestAsk();

	if((b2Bid - b1Ask) > (b1Ask * fee * 2))
	{
          spdlog::info("📈 Arbitrage Opportunity! Buy on Exchange1 at {} and sell on Exchange2 at {}", b1Ask, b2Bid);
	}

	if((b1Bid - b2Ask) > (b2Ask * fee * 2))
	{
          spdlog::info("📈 Arbitrage Opportunity! Buy on Exchange2 at {} and sell on Exchange1 at {}", b2Ask, b1Bid);
          
	}
}
