// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class ExtensionHeaderFrame : public StateMachine, public IElementConsumer<byte>
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

    public:
        ExtensionHeaderFrame(ExtensionHeader* extension);

        void reset();

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
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