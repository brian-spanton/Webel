// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Types.h"
#include "Basic.Globals.h"
#include "Basic.IProcess.h"

namespace Basic
{
    FatalError::FatalError(const char* context)
    {
        Basic::globals->HandleError(context, 0);
    }

    FatalError::FatalError(const char* context, uint32 error)
    {
        Basic::globals->HandleError(context, error);
    }

    bool HandleError(const char* context)
    {
        return Basic::globals->HandleError(context, 0);
    }

    IoCompletionEvent::IoCompletionEvent(std::shared_ptr<void> context, uint32 count, uint32 error) :
        context(context),
        count(count),
        error(error)
    {
    }

    uint32 IoCompletionEvent::get_type()
    {
        return EventType::io_completion_event;
    }

    uint32 ReceivedBytesEvent::get_type()
    {
        return EventType::received_bytes_event;
    }

    void ReceivedBytesEvent::Initialize(IElementSource<byte>* element_source)
    {
        this->element_source = element_source;
    }

    uint32 CanSendBytesEvent::get_type()
    {
        return EventType::can_send_bytes_event;
    }

    void CanSendBytesEvent::Initialize(IElementSource<byte>* element_source)
    {
        this->element_source = element_source;
    }

    uint32 ReceivedCodepointsEvent::get_type()
    {
        return EventType::received_codepoints_event;
    }

    void ReceivedCodepointsEvent::Initialize(IElementSource<Codepoint>* element_source)
    {
        this->element_source = element_source;
    }

    uint32 CanSendCodepointsEvent::get_type()
    {
        return EventType::can_send_codepoints_event;
    }

    void CanSendCodepointsEvent::Initialize(IElementSource<Codepoint>* element_source)
    {
        this->element_source = element_source;
    }

    uint32 ElementStreamEndingEvent::get_type()
    {
        return EventType::element_stream_ending_event;
    }

    uint32 EncodingsCompleteEvent::get_type()
    {
        return EventType::encodings_complete_event;
    }
}
