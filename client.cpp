//
// Created by Saad Hussain on 08-05-2023
//
#include <boost/asio.hpp>
#include <iostream>

void send_heartbeat(boost::asio::steady_timer &timer, boost::asio::ip::tcp::socket &socket) {
    char heartbeat[] = {'H'};
    boost::asio::write(socket, boost::asio::buffer(heartbeat, 1));

    std::cout << "Sent heartbeat" << std::endl;

    timer.expires_at(timer.expires_at() + boost::asio::chrono::seconds(1));
    timer.async_wait([&](const boost::system::error_code &error) {
        if (!error) {
            send_heartbeat(timer, socket);
        }
    });
}

int main() {
    const std::string server_address = "127.0.0.1";
    const unsigned short port = 5555;

    try {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::socket socket(io_service);
        boost::asio::connect(socket, resolver.resolve({server_address, std::to_string(port)}));
        std::cout << "Connected to server at " << server_address << ":" << port << std::endl;

        boost::asio::steady_timer timer(io_service, boost::asio::chrono::seconds(1));
        timer.async_wait([&](const boost::system::error_code &error) {
            if (!error) {
                send_heartbeat(timer, socket);
            }
        });

        io_service.run();
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}