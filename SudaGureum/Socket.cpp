#include "Common.h"

#include "Socket.h"

namespace SudaGureum
{
    TcpSocket::TcpSocket(boost::asio::io_service &ios)
        : socket_(ios)
    {
    }

    void TcpSocket::asyncHandshakeAsServer(std::function<void (const boost::system::error_code &)> handler)
    {
        throw(std::logic_error("asyncHandshakeAsServer is not implemented in TcpSocket"));
    }

    void TcpSocket::asyncReadSome(const boost::asio::mutable_buffers_1 &buffer,
        std::function<void (const boost::system::error_code &, size_t)> handler)
    {
        socket_.async_read_some(buffer, std::move(handler));
    }

    void TcpSocket::asyncWrite(const boost::asio::const_buffers_1 &buffer,
        std::function<void (const boost::system::error_code &, size_t)> handler)
    {
        boost::asio::async_write(socket_, buffer, std::move(handler));
    }

    void TcpSocket::asyncWrite(const boost::asio::mutable_buffers_1 &buffer,
        std::function<void (const boost::system::error_code &, size_t)> handler)
    {
        boost::asio::async_write(socket_, buffer, std::move(handler));
    }

    void TcpSocket::asyncConnect(const boost::asio::ip::tcp::resolver::iterator &endPointIt,
        std::function<void (const boost::system::error_code &, boost::asio::ip::tcp::resolver::iterator)> handler)
    {
        boost::asio::async_connect(socket_, endPointIt, std::move(handler));
    }

    boost::system::error_code TcpSocket::close()
    {
        boost::system::error_code ec;
        socket_.close(ec);
        return ec;
    }

    boost::asio::ip::tcp::socket &TcpSocket::socket()
    {
        return socket_;
    }

    TcpSslSocket::TcpSslSocket(boost::asio::io_service &ios)
        : TcpSslSocket(ios, std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23)) // Generic SSL/TLS
    {
    }

    TcpSslSocket::TcpSslSocket(boost::asio::io_service &ios, std::shared_ptr<boost::asio::ssl::context> context)
        : ctx_(std::move(context))
        , stream_(ios, *ctx_)
    {
    }

    void TcpSslSocket::asyncHandshakeAsServer(std::function<void (const boost::system::error_code &)> handler)
    {
        stream_.async_handshake(boost::asio::ssl::stream_base::server, handler);
    }

    void TcpSslSocket::asyncReadSome(const boost::asio::mutable_buffers_1 &buffer,
        std::function<void (const boost::system::error_code &, size_t)> handler)
    {
        stream_.async_read_some(buffer, std::move(handler));
    }

    void TcpSslSocket::asyncWrite(const boost::asio::const_buffers_1 &buffer,
        std::function<void (const boost::system::error_code &, size_t)> handler)
    {
        boost::asio::async_write(stream_, buffer, std::move(handler));
    }

    void TcpSslSocket::asyncWrite(const boost::asio::mutable_buffers_1 &buffer,
        std::function<void (const boost::system::error_code &, size_t)> handler)
    {
        boost::asio::async_write(stream_, buffer, std::move(handler));
    }

    void TcpSslSocket::asyncConnect(const boost::asio::ip::tcp::resolver::iterator &endPointIt,
        std::function<void (const boost::system::error_code &, boost::asio::ip::tcp::resolver::iterator)> handler)
    {
        boost::asio::async_connect(stream_.lowest_layer(), endPointIt,
            [this, handler](const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator resolverIt)
            {
                if(ec)
                    handler(ec, resolverIt);
                else
                {
                    stream_.async_handshake(boost::asio::ssl::stream_base::client,
                        std::bind(handler, std::placeholders::_1, resolverIt));
                }
            }
        );
    }

    boost::asio::ssl::stream<boost::asio::ip::tcp::socket>::lowest_layer_type &TcpSslSocket::socket()
    {
        return stream_.lowest_layer();
    }

    boost::system::error_code TcpSslSocket::close()
    {
        boost::system::error_code ec;
        stream_.shutdown(ec);
        return ec;
    }
}