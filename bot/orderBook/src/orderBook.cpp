#include <bot/orderBook/orderBook.hpp>

void OrderBook::clear()
{
	bids.clear();
	asks.clear();
}

void OrderBook::updateBid(double price, double volume)
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

void OrderBook::updateAsk(double price, double volume)
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

void OrderBook::updateOrderBook(nlohmann::json& orderBook)
{
	if(orderBook.contains("b"))
	{
		for(const auto& bid : orderBook["b"])
		{
			double price = std::stod(bid[0].get<std::string>());
			double volume = std::stod(bid[1].get<std::string>());

			if(volume == 0.0)
				bids.erase(price);
			else
				bids[price] = volume;
		}
	}

	if(orderBook.contains("a"))
	{
		for(const auto& ask : orderBook["a"])
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

double OrderBook::getBestBid() const
{
	return bids.empty() ? 0.0 : bids.rbegin()->first;
}

double OrderBook::getBestAsk() const
{
	return asks.empty() ? 0.0 : asks.begin()->first;
}