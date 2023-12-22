// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class ExtensionHeaderFrame : public Frame
    {
    private:
        enum State
        {
            type_frame_pending_state = Start_State,
            length_frame_pending_state,
            done_state = Succeeded_State,
            type_frame_failed,
            length_frame_failed,
        };

        ExtensionHeader* extension;
        NumberFrame<ExtensionType> type_frame;
        NumberFrame<uint16> length_frame;

        virtual event_result IProcess::consider_event(IEvent* event);

    public:
        ExtensionHeaderFrame(ExtensionHeader* extension);

        void reset();
    };

    template <>
    struct __declspec(novtable) serialize<ExtensionHeader>
    {
        void operator()(const ExtensionHeader* value, IStream<byte>* stream) const
        {
            serialize<ExtensionType>()(&value->type, stream);
            serialize<uint16>()(&value->length, stream);
        }
    };
}