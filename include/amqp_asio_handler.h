#pragma once

#include <memory>
#include <amqpcpp.h>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;


class AmqpAsioHandler : public AMQP::ConnectionHandler
{
    boost::asio::io_context& _io_context;
    tcp::socket _socket;

    class AmqpSession;
    std::shared_ptr<AmqpSession> _session;

    bool _read_in_progress = false;
    bool _write_in_progress = false;
    bool _connected = false;
public:
    AmqpAsioHandler(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints);

    virtual void onData(AMQP::Connection *connection, const char *data, size_t size);

    virtual void onReady(AMQP::Connection *connection);

    virtual void onError(AMQP::Connection *connection, const char *message);

    virtual void onClosed(AMQP::Connection *connection) ;

private:
    void handle_connect(boost::system::error_code ec, tcp::endpoint);

    void do_operations();
};