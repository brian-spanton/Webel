// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class AlertFrame : public Frame
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

        virtual void IProcess::consider_event(IEvent* event);

    public:
        AlertFrame(Alert* alert);
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
