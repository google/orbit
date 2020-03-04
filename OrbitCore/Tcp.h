//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#ifdef _WIN32
//#define _WIN32_WINNT 0x0501
#include <sdkddkver.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <asio.hpp>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_set>

#include "Message.h"
#include "OrbitAsio.h"
#include "TcpEntity.h"

using asio::ip::tcp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(const TcpConnection&) = delete;
  TcpConnection& operator=(const TcpConnection&) = delete;

  static std::shared_ptr<TcpConnection> create(asio::io_service& io_service) {
    return std::shared_ptr<TcpConnection>(new TcpConnection(io_service));
  }

  TcpSocket& GetSocket() { return wrapped_socket_; }

  void start() { ReadMessage(); }

  void ReadMessage();
  void ReadPayload();
  void ReadFooter();
  void DecodeMessage(Message& message);

  bool IsWebsocket() { return !web_socket_key_.empty(); }
  void ReadWebsocketHandshake();
  uint64_t GetNumBytesReceived() { return num_bytes_received_; }

  void ResetStats();
  std::vector<std::string> GetStats();

 private:
  TcpConnection(asio::io_service& io_service)
      : socket_(io_service),
        wrapped_socket_(&socket_),
        num_bytes_received_(0) {}
  // handle_write() is responsible for any further actions
  // for this client connection.
  void handle_write(const asio::error_code& /*error*/,
                    size_t /*bytes_transferred*/) {}

  void handle_request_line(asio::error_code ec, std::size_t bytes_transferred);
  void SendWebsocketResponse();

  tcp::socket socket_;
  TcpSocket wrapped_socket_;
  Message message_;
  std::vector<char> payload_;
  asio::streambuf stream_buf_;
  std::string web_socket_key_;
  uint64_t num_bytes_received_;
};

//-----------------------------------------------------------------------------
class tcp_server : public std::enable_shared_from_this<tcp_server> {
 public:
  tcp_server(asio::io_service& io_service, unsigned short port);
  ~tcp_server();

  void Disconnect();
  bool HasConnection() { return connection_ != nullptr; }
  TcpSocket* GetSocket() {
    return connection_ != nullptr ? &connection_->GetSocket() : nullptr;
  }
  void RegisterConnection(std::shared_ptr<TcpConnection> connection);
  ULONG64 GetNumBytesReceived() {
    return connection_ != nullptr ? connection_->GetNumBytesReceived() : 0;
  }
  void ResetStats() {
    if (connection_ != nullptr) {
      connection_->ResetStats();
    }
  }

 private:
  void start_accept();
  void handle_accept(std::shared_ptr<TcpConnection> new_connection,
                     const asio::error_code& error);

  tcp::acceptor acceptor_;
  std::shared_ptr<TcpConnection> connection_;
  // Is this here to keep them alive until server is destroyed?
  // This is not really used for anything else.
  std::unordered_set<std::shared_ptr<TcpConnection>> connections_set_;
};

//-----------------------------------------------------------------------------
class SharedConstBuffer {
 public:
  SharedConstBuffer() {}
  explicit SharedConstBuffer(const Message& message, const void* payload)
      : data_(new std::vector<char>()) {
    data_->resize(sizeof(Message) + message.m_Size + 4);
    memcpy(data_->data(), &message, sizeof(Message));

    if (payload != nullptr) {
      memcpy(data_->data() + sizeof(Message), payload, message.m_Size);
    }

    // Footer
    const unsigned int footer = MAGIC_FOOT_MSG;
    memcpy(data_->data() + sizeof(Message) + message.m_Size, &footer, 4);

    buffer_ = asio::buffer(*data_);
  }

  explicit SharedConstBuffer(TcpPacket& packet) {
    data_ = packet.Data();
    buffer_ = asio::buffer(*data_);
  }

  // Implement the ConstBufferSequence requirements.
  typedef asio::const_buffer value_type;
  typedef const asio::const_buffer* const_iterator;
  const asio::const_buffer* begin() const { return &buffer_; }
  const asio::const_buffer* end() const { return &buffer_ + 1; }

  std::shared_ptr<std::vector<char>> Data() { return data_; };

 private:
  std::shared_ptr<std::vector<char>> data_;
  asio::const_buffer buffer_;
};
