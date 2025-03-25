#pragma once
#include <boost/asio.hpp>
#include <bot/orderBook/orderBook.hpp>
#include <memory>
#include <spdlog/spdlog.h>

constexpr double fee = 0.01;

class ArbitrageEngine
{
public:
	ArbitrageEngine(std::shared_ptr<OrderBook> book1,
				 std::shared_ptr<OrderBook> book2);

	boost::asio::awaitable<void> run();

private:
	std::shared_ptr<OrderBook> book1_;
	std::shared_ptr<OrderBook> book2_;

	void checkArbitrage_();
};
