// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.NumberFrame.h"
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

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        ServerNameFrame(ServerName* serverName);
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::ServerName>
    {
        void operator()(const Tls::ServerName* value, IStream<byte>* stream) const
        {
            serialize<Tls::NameType>()(&value->name_type, stream);
            serialize<Tls::HostName>()(&value->name, stream);
        }
    };

    template <>
    struct __declspec(novtable) make_deserializer<Tls::ServerName> : public make_frame_deserializer<Tls::ServerName, Tls::ServerNameFrame> {};
}