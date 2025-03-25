#pragma once

#include <absl/container/btree_map.h>
#include <nlohmann/json.hpp>

class OrderBook
{
public:
	absl::btree_map<double, double> bids;
	absl::btree_map<double, double> asks;

	void updateBid(double price, double volume);

	void updateAsk(double price, double volume);

	void updateOrderBook(nlohmann::json& orderBook);

	void clear();

	double getBestBid() const;

	double getBestAsk() const;
};
