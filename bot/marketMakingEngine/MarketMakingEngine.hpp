#pragma once
#include <boost/asio.hpp>
#include <bot/orderBook/orderBook.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <spdlog/spdlog.h>

constexpr double spread = 5.0;
constexpr double orderSize = 0.01;

class MarketMakingEngine
{
public:
	MarketMakingEngine(std::shared_ptr<OrderBook> book);

	boost::asio::awaitable<void> run();

private:
	std::shared_ptr<OrderBook> book_;

	void simulateMarketMaking_();
};
