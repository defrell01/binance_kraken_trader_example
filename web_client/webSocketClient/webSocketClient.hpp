#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class WebSocketClient
{
public:
	WebSocketClient(asio::io_context& ioc, asio::ssl::context& ssl_ctx);
	asio::awaitable<void> connect(const std::string& host, const std::string& port, const std::string& target);
	asio::awaitable<void> read_loop();
	asio::awaitable<void> write(const std::string& message);
private:
	asio::io_context& io_context_;
	beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;
};