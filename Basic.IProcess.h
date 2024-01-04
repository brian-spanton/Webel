// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface IEvent;

    enum ProcessResult
    {
        process_result_blocked = 0,
        process_result_ready,
        process_result_exited,
    };

    enum EventType
    {
        io_completion_event = 0,
        received_bytes_event,
        can_send_bytes_event,
        received_codepoints_event,
        can_send_codepoints_event,
        element_stream_ending_event,
        encodings_complete_event,
    };

    // an IProcess receives events, known types of which are enumerated
    // in enum EventType. This is suitable for state machines that do more than,
    // or can respond to events outside of, stream processing. if pure stream
    // semantics are adequate, consider using IStream.
    __interface IProcess
    {
        ProcessResult process_event(IEvent* event);
        bool in_progress();
        bool succeeded();
        bool failed();
    };
}