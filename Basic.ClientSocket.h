// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ConnectedSocket.h"

namespace Basic
{
    class ClientSocket : public ConnectedSocket
    {
    private:
        enum State
        {
            new_state,
            bound_state,
            connecting_state,
            receiving_state,
        };

        State state;
        ByteStringRef sendBuffer;

        virtual void CompleteReadyForSend();
        virtual void CompleteSend(std::shared_ptr<ByteString> bytes, uint32 count, uint32 error);

    public:
        ClientSocket(std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size);

        bool Resolve(UnicodeStringRef host, uint16 port, sockaddr_in* remoteAddress);
        void Initialize();
        void StartConnect(IN_ADDR server, uint16 port);
        void StartConnect(sockaddr_in remoteAddress);

        virtual void IStream<byte>::write_elements(const byte* elements, uint32 count);
    };
}