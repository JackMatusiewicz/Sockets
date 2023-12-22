#ifndef SOCKETS_TCPCLIENT_H
#define SOCKETS_TCPCLIENT_H

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>

#include "SocketInfo.h"
#include "result/Result.h"

// Used to send and receive data from a particular endpoint that is represented by a socketInfo.
class TcpClient {
private:
    SocketInfo _socketInfo;
    bool _has_socket;
    int _socket_fd;
public:
    explicit TcpClient(SocketInfo&& socketInfo)
        : _socketInfo{std::move(socketInfo)}
        , _has_socket(false)
        , _socket_fd(0)
        {}

    [[nodiscard]] Result<int, std::string> open_connection() noexcept {
        if (_has_socket) {
            return Result<int, std::string>::fromError("Connection already active");
        }
        struct addrinfo hints{};
        struct addrinfo* results;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        if (_socketInfo.wants_local_machine()) {
            hints.ai_flags = AI_PASSIVE;
        }

        auto result = getaddrinfo(
                _socketInfo.address().empty() ? nullptr : _socketInfo.address().c_str(),
                _socketInfo.port() == 0 ? nullptr : std::to_string(_socketInfo.port()).c_str(),
                &hints,
                &results);
        if (result != 0) {
            std::string error_message(gai_strerror(result));
            return Result<int, std::string>::fromError(error_message);
        }
        for (auto it = results; it != nullptr; it = it->ai_next) {
            int socket_fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
            if (socket_fd == -1) {
                // We failed on this one so continue
                continue;
            }
            if (connect(socket_fd, it->ai_addr, it->ai_addrlen) == -1) {
                close(socket_fd);
                continue;
            } else {
                _has_socket = true;
                _socket_fd = socket_fd;
                break;
            }
        }
        if (!_has_socket) {
            return Result<int, std::string>::fromError("Unable to connect");
        }
        return Result<int, std::string>::fromSuccess(0);
    }

    Result<int, std::string> close_connection() noexcept {
        if (!_has_socket) {
            return Result<int, std::string>::fromError("No connection open");
        }
        close(_socket_fd);
        _has_socket = false;
        _socket_fd = 0;

        return Result<int, std::string>::fromSuccess(0);
    }

    // This is a blocking method, will need to implement a way to make it not so.
    Result<int, std::string> send_data(const std::vector<char>& data) const noexcept {
        if (!_has_socket) {
            return Result<int, std::string>::fromError("Cannot send when connection is not open");
        }
        auto data_sent = 0;
        auto current_index = 0;
        auto size = data.size();
        while (true) {
            auto actual_amount_sent = send(_socket_fd, &data[current_index], size - current_index, 0);
            if (actual_amount_sent == -1) {
                return Result<int, std::string>::fromError("Unable to send data");
            }
            data_sent += actual_amount_sent;
            current_index = data_sent;
            if (data_sent == size) {
                return Result<int, std::string>::fromSuccess(0);
            }
        }
    }
};

#endif //SOCKETS_TCPCLIENT_H