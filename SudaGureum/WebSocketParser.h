#pragma once

#include "Utility.h"

namespace SudaGureum
{
    enum WebSocketFrameOpcode
    {
        // Non-control frame
        Continuation = 0x0,
        Text = 0x1,
        Binary = 0x2,

        // Control frame
        Close = 0x8,
        Ping = 0x9,
        Pong = 0xA
    };

    struct WebSocketRequest // General WebSocket message (client -> server)
    {
        enum Command
        {
            Close,
            Ping,
        };

        Command command_;
        CaseInsensitiveUnorderedMap params_;
        std::vector<uint8_t> rawData_;

        WebSocketRequest();
        explicit WebSocketRequest(Command command);
        WebSocketRequest(Command command, CaseInsensitiveUnorderedMap params);
        WebSocketRequest(Command command, std::vector<uint8_t> rawData);
    };

    struct WebSocketResponse // General WebSocket message (server -> client)
    {
        enum Command
        {
            Close,
            Pong,
            Text,
        };

        Command command_;
        std::vector<uint8_t> rawData_;

        WebSocketResponse();
        explicit WebSocketResponse(Command command);
        WebSocketResponse(Command command, std::vector<uint8_t> rawData);

        WebSocketFrameOpcode opcode() const;
    };

    struct SudaGureumRequest // "SudaGureum" specific message (client -> server)
    {
        uint32_t id_;
        std::string method_;
        CaseInsensitiveUnorderedMap params_;
    };

    struct SudaGureumResponse // "SudaGureum" specific message (server -> client)
    {
        uint32_t id_;
        bool success_;
        rapidjson::Document responseDoc_;

        SudaGureumResponse();
        explicit SudaGureumResponse(const SudaGureumRequest &request);
        SudaGureumResponse(const SudaGureumRequest &request, bool success);
    };

    class WebSocketParser
    {
    private:
        enum State
        {
            IN_WEB_SOCKET_FRAME_HEADER,
            IN_PAYLOAD,
            PARSE_ERROR
        };

    private:
        static bool IsControlFrameOpcode(WebSocketFrameOpcode opcode);

    public:
        WebSocketParser();

    public:
        bool parse(const std::vector<uint8_t> &data,
            std::function<void (const WebSocketRequest &)> wscb,
            std::function<void (const SudaGureumRequest &)> sgcb);

    public:
        explicit operator bool() const;

    private:
        bool parseEmptyFrame(std::function<void (const WebSocketRequest &)> wscb,
            std::function<void (const SudaGureumRequest &)> sgcb);
        bool parseFrame(std::vector<uint8_t> data,
            std::function<void (const WebSocketRequest &)> wscb,
            std::function<void (const SudaGureumRequest &)> sgcb);
        bool parsePayload(std::function<void (const SudaGureumRequest &)> cb);

    private:
        State state_;
        std::vector<uint8_t> buffer_;

        bool finalFragment_;
        WebSocketFrameOpcode opcode_;
        bool masked_;
        uint8_t payloadLen1_;

        uint64_t payloadLen_;
        std::array<uint8_t, 4> maskingKey_;

        std::vector<uint8_t> totalPayload_;
    };
}
