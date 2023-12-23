// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

	class AlertFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
            level_frame_pending_state = Start_State,
            description_frame_pending_state,
            done_state = Succeeded_State,
            level_frame_failed,
            description_frame_failed,
        };

        Alert* alert;
        NumberFrame<AlertLevel> level_frame;
        NumberFrame<AlertDescription> description_frame;

    public:
        AlertFrame(Alert* alert);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};

    template <>
    struct __declspec(novtable) serialize<Alert>
    {
        void operator()(const Alert* value, IStream<byte>* stream) const
        {
            serialize<AlertLevel>()(&value->level, stream);
            serialize<AlertDescription>()(&value->description, stream);
        }
    };
}
