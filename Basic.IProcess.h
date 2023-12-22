// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface IEvent;

    enum EventResult
    {
        event_result_yield = 0,
        event_result_continue,
        event_result_process_inactive,
    };

    enum EventType
    {
        process_event,
        received_bytes_event,
        can_send_bytes_event,
        received_codepoints_event,
        can_send_codepoints_event,
        element_stream_ending_event,
        request_headers_event,
        request_complete_event,
        encodings_complete_event,
    };

    // an IProcess receives events, known types of which are enumerated
    // in enum EventType. This is suitable for state machines that do more than,
    // or can respond to events outside of, stream processing. if pure stream
    // semantics are adequate, consider using IStream.
    __interface IProcess
    {
        EventResult consider_event(IEvent* event);
        bool in_progress();
        bool succeeded();
        bool failed();
    };
}