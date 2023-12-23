// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class ServerNameFrame : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            type_state,
            name_state,
            done_state = Succeeded_State,
            type_frame_failed,
            name_frame_failed,
        };

        ServerName* serverName;
        NumberFrame<NameType> type_frame;
        VectorFrame<HostName> name_frame;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        ServerNameFrame(ServerName* serverName);
    };

    template <>
    struct __declspec(novtable) serialize<ServerName>
    {
        void operator()(const ServerName* value, IStream<byte>* stream) const
        {
            serialize<NameType>()(&value->name_type, stream);
            serialize<HostName>()(&value->name, stream);
        }
    };

    template <>
    struct __declspec(novtable) make_deserializer<ServerName> : public make_frame_deserializer<ServerName, ServerNameFrame> {};
}