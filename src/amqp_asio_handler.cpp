#include <amqp_asio_handler.h>
#include <iostream>
#include <boost/asio/streambuf.hpp>

#include <boost/bind/bind.hpp>


class AmqpAsioHandler::AmqpSession
{
public:
    AmqpSession(tcp::socket& socket)
        : _socket(socket)
    {
    }

    bool want_read() const
    {
        return _state == reading;
    }

    void do_read(boost::system::error_code& ec)
    {   
        std::cout << "do_operations reading availible " << _socket.available() << std::endl;
        if (_socket.available() > 0)
        {
            _read_buff.commit(_socket.read_some(_read_buff.prepare(_socket.available()), ec));
        }

        if (_connection && _read_buff.size() > 0)
        {
            _read_buff.consume(_connection->parse(boost::asio::buffer_cast<const char*>(_read_buff.data()), _read_buff.size()));
        }
    }

    bool want_write() const
    {
        return _state == writing;
    }

    void do_write(boost::system::error_code& ec)
    {
        auto size = _socket.write_some(_write_buff.data(), ec);
        std::cout << "do_operations writing send size " << size << std::endl;
        _write_buff.consume(size);
        _state = _write_buff.size() > 0 ? writing : reading;
    }

private:
    tcp::socket& _socket;
    AMQP::Connection * _connection;
    boost::asio::streambuf _read_buff, _write_buff;
    enum { reading, writing } _state = reading;
    friend class AmqpAsioHandler;
};


AmqpAsioHandler::AmqpAsioHandler(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints) :
    _io_context(io_context),
    _socket(io_context),
    _session(new AmqpSession(_socket))
{
    boost::asio::async_connect(_socket, endpoints, 
        boost::bind(&AmqpAsioHandler::handle_connect, this, 
            boost::asio::placeholders::error, boost::asio::placeholders::endpoint
        )
    );
}

void AmqpAsioHandler::handle_connect(boost::system::error_code ec, tcp::endpoint) 
{
    if (!ec) 
    {
        _socket.non_blocking(true);
        std::cout << "connect succesfull" << std::endl;   
        boost::asio::socket_base::keep_alive option(true);
        _socket.set_option(option);
        _connected = true;
        do_operations(); 
    }
}

void AmqpAsioHandler::do_operations()
{
    
    if (!_connected)
    {
        return;
    }
    
    if (_session->want_read() && !_read_in_progress)
    {
        std::cout << "do_operations read - " << (_session->want_read() && !_read_in_progress) << std::endl;
        _read_in_progress = true;
        _socket.async_wait(tcp::socket::wait_read, 
        [this](boost::system::error_code ec)
        {
            _read_in_progress = false;
            if (!ec)
            {
                _session->do_read(ec);
            }

            if (!ec || ec == boost::asio::error::would_block)
            {
                do_operations();
            }
            else
            {
                _socket.close();
            }
        });
    }

    if (_session->want_write() && !_write_in_progress)
    {
        std::cout << "do_operations write - " << (_session->want_write() && !_write_in_progress) << std::endl;
        _write_in_progress = true;
        _socket.async_wait(tcp::socket::wait_write,
        [this](boost::system::error_code ec)
        {
            _write_in_progress = false;
            if (!ec)
            {
                _session->do_write(ec);
            }

            if (!ec || ec == boost::asio::error::would_block)
            {
                do_operations();
            }
            else
            {
                _socket.close();
            }
        });
    }
}

void AmqpAsioHandler::onReady(AMQP::Connection *connection)
{
    std::cout << "AmqpAsioHandler::onReady" << std::endl;
    _session->_connection = connection;
}

void AmqpAsioHandler::onData(
        AMQP::Connection *connection, const char *data, size_t size)
{
    _session->_connection = connection;
    std::cout << "AmqpAsioHandler::onData " << std::string(data, size) << " " << size << std::endl;
    std::ostream(&_session->_write_buff).write(data, size);    
    _session->_state = AmqpSession::writing;
    do_operations();
}

void AmqpAsioHandler::onError(
        AMQP::Connection *connection, const char *message)
{
    std::cerr << "AMQP error " << message << std::endl;
}

void AmqpAsioHandler::onClosed(AMQP::Connection *connection)
{
    std::cout << "AMQP closed connection" << std::endl;
    _socket.close();
}
