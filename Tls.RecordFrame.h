// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class RecordFrame : public Frame
    {
    private:
        enum State
        {
            type_frame_pending_state = Start_State,
            version_frame_pending_state,
            length_frame_pending_state,
            fragment_frame_pending_state,
            done_state = Succeeded_State,
            type_frame_failed,
            version_frame_failed,
            length_frame_failed,
            fragment_frame_failed,
        };

        Record* record;
        NumberFrame<ContentType> type_frame;
        NumberFrame<ProtocolVersion> version_frame;
        NumberFrame<uint16> length_frame;
        MemoryRange fragment_frame;

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        RecordFrame(Record* record);

        void reset();
    };

    template <>
    struct __declspec(novtable) serialize<Record>
    {
        void operator()(const Record* value, IStream<byte>* stream) const
        {
            serialize<ContentType>()(&value->type, stream);
            serialize<ProtocolVersion>()(&value->version, stream);
            serialize<uint16>()(&value->length, stream);
            value->fragment->write_to_stream(stream);
        }
    };
}