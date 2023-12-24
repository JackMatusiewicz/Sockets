#include <iostream>
#include <chrono>
#include <ctime>

#include "lib/SocketInfo.h"
#include "lib/ConnectedSocketClient.h"
#include "lib/AutoConnectingSocketClient.h"

int main() {
    auto si = SocketInfo(SocketType::Stream, 12345);
    auto client_creation_result = AutoConnectingSocketClient::create(std::move(si));
    const auto& client = client_creation_result.get_success();

    auto current_time = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(current_time);
    std::string message = std::string("The time is now: ") + std::ctime(&time);
    std::vector<char> data(message.begin(), message.end());
    auto _ = client.send_data(data);
    auto received_result = client.receive_data();
    if (!received_result.is_error()) {
        auto received_data = received_result.get_success();
        auto str = std::string(received_data.begin(), received_data.end());
        std::cout << "Received the following string: " << str << std::endl;
    }
    return 0;
}
