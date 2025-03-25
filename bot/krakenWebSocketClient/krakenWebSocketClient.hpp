#pragma once
#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <bot/orderBook/orderBook.hpp>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>

class KrakenWebSocketClient
{
public:
	KrakenWebSocketClient(boost::asio::io_context& ioc,
					  boost::asio::ssl::context& ssl_ctx,
					  std::shared_ptr<OrderBook> orderBook);

	boost::asio::awaitable<void>
	connect(const std::string& host, const std::string& port, const std::string& target);
	boost::asio::awaitable<void> read_loop();
	boost::asio::awaitable<void> write(const std::string& message);

private:
	boost::asio::io_context& io_context_;
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;
	std::shared_ptr<OrderBook> orderBook_;
};
