#include <bot/order_book/orderBook.hpp>

void OrderBook::clear()
{
	bids.clear();
	asks.clear();
}

void OrderBook::update_bid(double price, double volume)
{
	if(volume > 0)
	{
		bids[price] = volume;
	}
	else
	{
		bids.erase(price);
	}
}

void OrderBook::update_ask(double price, double volume)
{
	if(volume > 0)
	{
		asks[price] = volume;
	}
	else
	{
		asks.erase(price);
	}
}

void OrderBook::update_order_book(nlohmann::json& order_book)
{
	if(order_book.contains("b"))
	{
		for(const auto& bid : order_book["b"])
		{
			double price = std::stod(bid[0].get<std::string>());
			double volume = std::stod(bid[1].get<std::string>());

			if(volume == 0.0)
				bids.erase(price);
			else
				bids[price] = volume;
		}
	}

	if(order_book.contains("a"))
	{
		for(const auto& ask : order_book["a"])
		{
			double price = std::stod(ask[0].get<std::string>());
			double volume = std::stod(ask[1].get<std::string>());

			if(volume == 0.0)
				asks.erase(price);
			else
				asks[price] = volume;
		}
	}
}

double OrderBook::get_best_bid() const
{
	return bids.empty() ? 0.0 : bids.rbegin()->first;
}

double OrderBook::get_best_ask() const
{
	return asks.empty() ? 0.0 : asks.begin()->first;
}