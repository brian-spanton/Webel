// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Socket.h"
#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Basic.Lock.h"

namespace Basic
{
    class ConnectedSocket : public Socket, public IStream<byte>
    {
    protected:
        std::weak_ptr<IProcess> protocol;
        ElementSource<byte> protocol_element_source;
        sockaddr_in remoteAddress;
        uint32 receive_buffer_size;

        void StartReceive(std::shared_ptr<ByteString> bytes);
        void Received(ByteString* bytes);
        void InitializePeer(sockaddr_in* remoteAddress);
        void Send(ByteStringRef buffer);
        void Disconnect(std::shared_ptr<IProcess>* protocol);
        void DisconnectAndNotifyProtocol();
        virtual void CompleteReceive(std::shared_ptr<ByteString> bytes, uint32 error);
        virtual void CompleteDisconnect();

    public:
        ConnectedSocket(std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size);

        virtual void IStream<byte>::write_elements(const byte* elements, uint32 count);
        virtual void IStream<byte>::write_element(byte element);
        virtual void IStream<byte>::write_eof();
    };
}