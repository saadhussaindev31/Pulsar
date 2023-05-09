//
// Created by Saad Hussain on 08-05-2023
//
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

std::mutex connection_mutex;
bool received_heartbeat = false;

void check_heartbeat(boost::asio::steady_timer &timer, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    std::unique_lock<std::mutex> lock(connection_mutex);
    if (!received_heartbeat) {
        socket->close();
        std::cerr << "Client disconnected due to missed heartbeat." << std::endl;
    } else {
        received_heartbeat = false;
    }
    lock.unlock();

    timer.expires_at(timer.expires_at() + boost::asio::chrono::seconds(5));
    timer.async_wait([&](const boost::system::error_code &error) {
        if (!error) {
            check_heartbeat(timer, socket);
        }
    });
}



void handle_client(boost::asio::io_service &io_service, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    std::cerr << "Client connected" << std::endl;

    boost::asio::steady_timer timer(io_service, boost::asio::chrono::seconds(5));
    timer.async_wait([&](const boost::system::error_code &error) {
        if (!error) {
            check_heartbeat(timer, socket);
        }
    });

    try {
        for (;;) {
            char data[1];
            boost::system::error_code ec;
            size_t len = socket->read_some(boost::asio::buffer(data), ec);

            if (ec) {
                if (ec == boost::asio::error::eof) {
                    std::cerr << "Client disconnected" << std::endl;
                    socket->close();
                    break; // Client closed the socket, do not update received_heartbeat
                }
                throw boost::system::system_error(ec); // Handle other errors
            }

            {
                std::lock_guard<std::mutex> lock(connection_mutex);
                if (data[0] == 'H') {
                    received_heartbeat = true;
                    std::cerr << "Heartbeat received from client" << std::endl;
                }
            }
        }
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}


int main() {
    const unsigned short port = 5555;

    try {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

        std::cout << "Server running on port " << port << std::endl;

        for (;;) {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_service);
            acceptor.accept(*socket);

            std::thread(handle_client, std::ref(io_service), socket).detach();
        }
    } catch (std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
