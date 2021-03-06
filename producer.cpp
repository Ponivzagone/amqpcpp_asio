#include <boost/asio/io_service.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <boost/program_options.hpp>

#include <amqpcpp.h>
#include "include/amqp_asio_handler.h"

#include <iostream>

#include "cpx.hh"
#include <avro/Encoder.hh>

namespace po = boost::program_options;

int main(int argc, char** argv) 
{

    std::string msg = "info: Hello World!";
    po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "print usage message")
        ("message,m", po::value(&msg), "message for amqp log");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {  
        std::cout << desc << "\n";
        return 0;
    }

    boost::asio::io_context io_context(1);

    tcp::resolver resolver(io_context);
    tcp::resolver::query query("localhost", "5672");
    AmqpAsioHandler handler(io_context, resolver.resolve(query));
    AMQP::Connection connection(&handler, AMQP::Login("guest", "guest"), "/");
    AMQP::Channel channel(&connection);
    
    // create a temporary queue
    channel.declareExchange("logs", AMQP::fanout).onSuccess([&]()
    {
        std::ostringstream oss;
        auto out = avro::ostreamOutputStream(oss, 32);
        avro::EncoderPtr e = avro::binaryEncoder();
        e->init(*out);
        c::cpx c1;
        c1.re = 1.56;
        c1.im = 9.1378;
        avro::encode(*e, c1);
        out->flush();
        auto msg  = oss.str();
        bool status = channel.publish("logs", "", msg);
        std::cout << " [x] Sent " << msg << std::endl;
        connection.close();
    });
    
    std::cout << "io_context.run" << std::endl;
    return io_context.run();


}