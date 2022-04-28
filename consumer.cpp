#include <iostream>

#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <amqpcpp.h>

#include "include/amqp_asio_handler.h"

#include "cpx.hh"
#include <avro/Decoder.hh>

int main(void)
{

    boost::asio::io_context io_context(1);

    tcp::resolver resolver(io_context);
    tcp::resolver::query query("localhost", "5672");
    AmqpAsioHandler handler(io_context, resolver.resolve(query));
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);

    auto receiveMessageCallback = [](const AMQP::Message &message,
            uint64_t deliveryTag,
            bool redelivered)
    {
        std::istringstream iss(std::string(message.body(), message.bodySize()));
        auto in = avro::istreamInputStream(iss, 32);
        avro::DecoderPtr d = avro::binaryDecoder();
        d->init(*in);
        c::cpx c2;
        avro::decode(*d, c2);
        std::cout << " [x] " << '(' << c2.re << ", " << c2.im << ')' << std::endl;
    };

    AMQP::QueueCallback callback =
            [&](const std::string &name, int msgcount, int consumercount)
            {
                channel.bindQueue("logs", name,"");
                channel.consume(name, AMQP::noack).onReceived(receiveMessageCallback);
            };

    AMQP::SuccessCallback success = [&]()
            {
                channel.declareQueue(AMQP::exclusive).onSuccess(callback);
            };

    channel.declareExchange("logs", AMQP::fanout).onSuccess(success);

    std::cout << " [*] Waiting for messages. To exit press CTRL-C\n";
    return io_context.run();
}