// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.GzipFrame.h"

namespace Basic
{
    GzipFrame::GzipFrame(std::shared_ptr<IStream<byte> > uncompressed) :
        uncompressed(uncompressed)
    {
    }

    EventResult GzipFrame::consider_event(IEvent* event)
    {
        // RFC1952 https://www.rfc-editor.org/rfc/rfc1952

        return event_result_process_inactive;
    }
}