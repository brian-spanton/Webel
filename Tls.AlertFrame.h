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

        virtual ProcessResult IProcess::consider_event(IEvent* event);

    public:
        AlertFrame(Alert* alert);
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::Alert>
    {
        void operator()(const Tls::Alert* value, IStream<byte>* stream) const
        {
            serialize<Tls::AlertLevel>()(&value->level, stream);
            serialize<Tls::AlertDescription>()(&value->description, stream);
        }
    };
}
