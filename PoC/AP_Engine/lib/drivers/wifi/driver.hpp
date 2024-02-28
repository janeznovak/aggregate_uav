#ifndef FCPP_POC_WIFI_DRIVER_H_
#define FCPP_POC_WIFI_DRIVER_H_

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <chrono>
#include <exception>
#include <random>
#include <thread>
#include <vector>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lib/poc_utils.hpp"
#include "lib/common/mutex.hpp"
#include "lib/settings.hpp"
#include "lib/component/base.hpp"
#include "lib/deployment/os.hpp"


#define DBG_PRINT_SUCCESSFUL_CALLS
#define DBG_TRANSCEIVER_ACTIVITY_LED

#define AP_NET_BRADDR_PORT             "AP_NET_BRADDR_PORT"
#define DEFAULT_NET_BRADDR_PORT        10000
#define AP_NET_BRADDR_IP               "AP_NET_BRADDR_IP"
#define DEFAULT_NET_BRADDR_IP          "192.168.1.255"
#define AP_RECV_TIMEOUT_MS             "AP_RECV_TIMEOUT_MS"
#define DEFAULT_AP_RECV_TIMEOUT_MS     20

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing OS-dependent functionalities.
namespace os {



//! @brief Access the local unique identifier.
device_t uid() {
    return static_cast<device_t>(AP_NODE_UID);
}

//! @brief Access the port using in communication
int port() {
    std::string port_str = get_env_var(AP_NET_BRADDR_PORT, std::to_string(DEFAULT_NET_BRADDR_PORT));
    int port = stoi(port_str);
    return port;
}

//! @brief Access the broadcast ip address used in communication
std::string broadcast_ip_address() {
    std::string ip_address_str = get_env_var(AP_NET_BRADDR_IP, std::to_string(DEFAULT_NET_BRADDR_IP));
    return ip_address_str;
}

//! @brief Access the port using in communication
int recv_timeout() {
    std::string recv_timeout_ms = get_env_var(AP_RECV_TIMEOUT_MS, std::to_string(DEFAULT_AP_RECV_TIMEOUT_MS));
    int port = stoi(recv_timeout_ms);
    return port;
}

/**
 * @brief Low-level interface for hardware network capabilities.
 * It uses UDP messages and broadcast communication.
 * The driver creates two socket (for send and receive) binded to broadcast IP.
 *
 * It should have the following minimal public interface:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * struct data_type;                            // default-constructible type for settings
 * data_type data;                              // network settings
 * transceiver(data_type);                      // constructor with settings
 * bool send(device_t, std::vector<char>, int); // broadcasts a message after given attemps
 * message_type receive(int);                   // listens for messages after given failed sends
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct transceiver {
    //! @brief Default-constructible type for settings.
    struct data_type {
        //! @brief Number of attempts after which a send is aborted.
        uint8_t send_attempts;
        //! @brief Broadcast address used for communication.
        string broadcast_addr_str;
        //! @brief Max packet size received.
        size_t max_packet_size;
        //! @brief Timeout used between reception attempts
        int recv_millis_timeout;

        //! @brief Member constructor with defaults.
        data_type(uint8_t sndatt = 5, string braddr = broadcast_ip_address(), size_t max_psize = 65507, int recvmillis_timeout = recv_timeout()) : 
            send_attempts(sndatt), 
            broadcast_addr_str(braddr), 
            max_packet_size(max_psize),
            recv_millis_timeout(recvmillis_timeout) {}
    };

    //! @brief Network settings.
    data_type data;

    ~transceiver() {
        close(send_sockfd);
        close(recv_sockfd);
    }

    //! @brief Constructor with settings.
    transceiver(data_type d) : data(d), m_fcpp_timer(common::make_tagged_tuple<>()), recv_counter(0) {
        send_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (send_sockfd < 0) {
            std::cerr << "Error opening sending socket" << std::endl;
            exit(1);
        }

        int send_enable = 1;
        if (setsockopt(send_sockfd, SOL_SOCKET, SO_BROADCAST, &send_enable, sizeof(send_enable)) < 0) {
            std::cerr << "Error configuring sending socket for broadcast" << std::endl;
            exit(1);
        }

        recv_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (recv_sockfd < 0) {
            std::cerr << "Error opening receiving socket" << std::endl;
            exit(1);
        }
        
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = AF_INET;
        in_addr_t ip_addr = inet_addr(data.broadcast_addr_str.c_str()); 
        if (ip_addr == INADDR_NONE) {
            std::cerr << "Error: IP address not valid" << std::endl;
            exit(1);
        }
        broadcast_addr.sin_addr.s_addr = ip_addr;
        broadcast_addr.sin_port = htons(port());

        if (bind(recv_sockfd, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
            std::cerr << "Error binding receiving socket" << std::endl;
            exit(1);
        }
    }

    //! @brief Broadcasts a given message.
    bool send(device_t id, const std::vector<char>& m, int attempt) {
        recv_counter++;
        if (AP_ENGINE_DEBUG) {
             std::cout << "Send data with counter=" << recv_counter << " : attempt " <<  attempt << std::endl;
        }

        size_t size = m.size() + sizeof(device_t) + sizeof(long long);
        char packet[size];

        char *ptr = packet;
        memcpy(ptr, m.data(), m.size());
        ptr += m.size();
        memcpy(ptr, &id, sizeof(device_t));
        ptr += sizeof(device_t);
        memcpy(ptr, &recv_counter, sizeof(long long));

        // Send data using socket UDP
        if (attempt < data.send_attempts) {
            ssize_t b_sent = sendto(send_sockfd, packet, size, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
            if (b_sent < 0) {
                std::cerr << "Error sending message" << std::endl;
                return false;
            }
            return true;
        } else {
             std::cerr << "Attempts exceeds:" << attempt << std::endl;
        }
        return false;
    }

    //! @brief Receives the next incoming message (empty if no incoming message).
    message_type receive(int attempt) {
        // std::cout << "Receive data" << std::endl;

        // Initialize descriptor for listen new data
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(recv_sockfd, &read_fds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = data.recv_millis_timeout * 1000;  // Timeout of X milliseconds

        // Receives data using socket UDP
        char buffer[data.max_packet_size];
        socklen_t addr_len = sizeof(broadcast_addr);
        message_type m;

        // Try to listen if there are available data 
        int ready = select(recv_sockfd + 1, &read_fds, NULL, NULL, &timeout);
        if (ready > 0) {
            if (FD_ISSET(recv_sockfd, &read_fds)) {
                ssize_t recv_len = recv(recv_sockfd, buffer, data.max_packet_size, 0);
                if (recv_len == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        std::cout << "No data available" << std::endl;
                    } else {
                        perror("recvfrom");
                    }
                } else if(recv_len < 0) {
                    // std::cout << "No data found" << std::endl;
                } else {
                    size_t content_size = recv_len - sizeof(device_t) - sizeof(long long);
                    long long m_counter = *reinterpret_cast<long long*>(buffer + content_size + sizeof(device_t));
                    m.device = *reinterpret_cast<device_t*>(buffer + content_size);
                    m.content.resize(content_size);
                    memcpy(m.content.data(), buffer, content_size);
                    m.time = m_fcpp_timer.real_time();
                    m.power = 1.0; // TODO

                    // times_t time;
                    // //! @brief UID of the sender device.
                    // device_t device;
                    // //! @brief An estimate of the signal power (RSSI).
                    // real_t power;
                    // //! @brief The message content (empty content represent no message).
                    // std::vector<char> content;
                    
                    if (AP_ENGINE_DEBUG) {
                        std::cout << "Bytes received from node-" << m.device << " with counter=" << m_counter << ": " << recv_len << std::endl;
                    }
                }
            }
        }

        return m;
    }

  private:
    //! @brief An empty net object for accessing real time.
    component::combine<>::component<>::net m_fcpp_timer;
    //! @brief Send socket descriptor
    int send_sockfd;
    //! @brief Receive socket descriptor
    int recv_sockfd;
    //! @brief Receive counter message
    long long recv_counter;
    //! @brief Structure describing the socket address used.
    struct sockaddr_in broadcast_addr;
};


}


}

#endif // FCPP_POC_WIFI_DRIVER_H_
