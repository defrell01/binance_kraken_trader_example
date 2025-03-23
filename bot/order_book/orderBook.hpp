#pragma once

#include <absl/container/btree_map.h>
#include <nlohmann/json.hpp>

class OrderBook
{
public:
	absl::btree_map<double, double> bids;
	absl::btree_map<double, double> asks;

	void update_bid(double price, double volume);

	void update_ask(double price, double volume);

	void update_order_book(nlohmann::json& order_book);

	void clear();

	double get_best_bid() const;

	double get_best_ask() const;
};
