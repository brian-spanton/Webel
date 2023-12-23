// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class ServerNameFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
			type_state = Start_State,
            name_state,
            done_state = Succeeded_State,
            type_frame_failed,
            name_frame_failed,
        };

        ServerName* serverName;
        NumberFrame<NameType> type_frame;
        VectorFrame<HostName> name_frame;

    public:
        ServerNameFrame(ServerName* serverName);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
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