#include <iostream>
#include <chrono>
#include <ctime>

#include "lib/SocketInfo.h"
#include "lib/TcpClient.h"

int main() {
    auto si = SocketInfo(SocketType::Stream, 12345);
    auto client = TcpClient(std::move(si));
    auto connection_result = client.open_connection();
    if (connection_result.is_error()) {
        std::cout << "connection opening error: " << connection_result.get_error() << std::endl;
        return -1;
    }
    auto current_time = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(current_time);
    std::string message = std::string("The time is now: ") + std::ctime(&time);
    std::vector<char> data(message.begin(), message.end());
    client.send_data(data);
    auto received_result = client.receive_data();
    if (!received_result.is_error()) {
        auto received_data = received_result.get_success();
        auto str = std::string(received_data.begin(), received_data.end());
        std::cout << "Received the following string: " << str << std::endl;
    }
    connection_result = client.close_connection();
    if (connection_result.is_error()) {
        std::cout << "connection closing error: " << connection_result.get_error() << std::endl;
        return -1;
    }
    return 0;
}
