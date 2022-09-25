/*
 * Copyright (c) TrueBrain
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "ClientImpl.h"

#include <chrono>
#include <string>
#include <map>
#include <netdb.h>
#include <thread>
#include <vector>

// Some definitions to make future cross-platform work easier.
#define SOCKET int
#define INVALID_SOCKET -1
#define closesocket close

class Packet;

class TrueMQTT::Client::Impl::Connection
{
public:
    Connection(TrueMQTT::Client::Impl &impl);
    ~Connection();

    bool send(Packet &packet) const;
    void socketError();

private:
    // Implemented in Connection.cpp
    void run();
    void resolve();
    bool tryNextAddress();
    void connect(addrinfo *address);
    bool connectToAny();
    std::string addrinfoToString(const addrinfo *address) const;

    // Implemented in Packet.cpp
    ssize_t recv(char *buffer, size_t length) const;
    bool recvLoop();
    bool sendConnect();

    enum class State
    {
        RESOLVING,
        CONNECTING,
        AUTHENTICATING,
        CONNECTED,
        BACKOFF,
        SOCKET_ERROR,
        STOP,
    };

    TrueMQTT::Client::Impl &m_impl;

    State m_state = State::RESOLVING; ///< Current state of the connection.
    std::thread m_thread;             ///< Current thread used to run this connection.

    std::chrono::milliseconds m_backoff; ///< Current backoff time.

    addrinfo *m_host_resolved = nullptr;      ///< Address info of the hostname, once looked up.
    std::vector<addrinfo *> m_addresses = {}; ///< List of addresses to try to connect to.

    size_t m_address_current = 0;                              ///< Index of the address we are currently trying to connect to.
    std::chrono::steady_clock::time_point m_last_attempt = {}; ///< Time of the last attempt to connect to the current address.

    std::vector<SOCKET> m_sockets = {};                    ///< List of sockets we are currently trying to connect to.
    std::map<SOCKET, addrinfo *> m_socket_to_address = {}; ///< Map of sockets to the address they are trying to connect to.

    SOCKET m_socket = INVALID_SOCKET; ///< The socket we are currently connected with, or INVALID_SOCKET if not connected.
};
