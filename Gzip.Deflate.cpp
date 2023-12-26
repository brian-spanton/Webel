// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Gzip.MemberFrame.h"
#include "Gzip.Deflate.h"

namespace Gzip
{
    Deflate::Deflate(std::shared_ptr<IStream<byte> > uncompressed) :
        uncompressed(uncompressed)
    {
    }

    EventResult Deflate::consider_event(IEvent* event)
    {
        // RFC1951 https://www.rfc-editor.org/rfc/rfc1951

        EventResult result;

        switch (get_state())
        {
        case start_state:
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("ServerHelloFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}