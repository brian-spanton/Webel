// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Event.h"
#include "Basic.Frame.h"
#include "Basic.Globals.h"

namespace Basic
{
    template <>
    ProcessResult Event::Read(IEvent* event, uint32 count, const byte** out_address, uint32* out_count)
    {
        if (event->get_type() == EventType::element_stream_ending_event)
            return ProcessResult::process_result_blocked;

        if (event->get_type() != EventType::received_bytes_event)
            Event::HandleUnexpectedEvent("Event::Read", event);

        ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;

        if (read_event->element_source->Exhausted())
            return ProcessResult::process_result_blocked;

        read_event->element_source->Read(count, out_address, out_count);

        return ProcessResult::process_result_ready;
    }

    template <>
    ProcessResult Event::ReadNext(IEvent* event, byte* element)
    {
        if (event->get_type() == EventType::element_stream_ending_event)
            return ProcessResult::process_result_blocked;

        if (event->get_type() != EventType::received_bytes_event)
            Event::HandleUnexpectedEvent("Event::ReadNext", event);

        ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;
        if (read_event->element_source->Exhausted())
            return ProcessResult::process_result_blocked; // event consumed

        read_event->element_source->ReadNext(element);

        return ProcessResult::process_result_ready;
    }

    template <>
    void Event::AddObserver(IEvent* event, std::shared_ptr<IStream<byte> > stream)
    {
        if (event->get_type() == EventType::received_bytes_event)
        {
            ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;
            return read_event->element_source->AddObserver(stream);
        }
        else if (event->get_type() == EventType::can_send_bytes_event)
        {
            CanSendBytesEvent* write_event = (CanSendBytesEvent*)event;
            return write_event->element_source->AddObserver(stream);
        }

        throw FatalError("Basic", "Event::AddObserver");
    }

    template <>
    void Event::RemoveObserver(IEvent* event, std::shared_ptr<IStream<byte> > stream)
    {
        if (event->get_type() == EventType::received_bytes_event)
        {
            ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;
            return read_event->element_source->RemoveObserver(stream);
        }
        else if (event->get_type() == EventType::can_send_bytes_event)
        {
            CanSendBytesEvent* write_event = (CanSendBytesEvent*)event;
            return write_event->element_source->RemoveObserver(stream);
        }

        throw FatalError("Basic", "Event::RemoveObserver");
    }

    template <>
    ProcessResult Event::Read(IEvent* event, uint32 count, const Codepoint** out_address, uint32* out_count)
    {
        if (event->get_type() == EventType::element_stream_ending_event)
            return ProcessResult::process_result_blocked;

        if (event->get_type() != EventType::received_codepoints_event)
            Event::HandleUnexpectedEvent("Event::Read", event);

        ReceivedCodepointsEvent* read_event = (ReceivedCodepointsEvent*)event;

        if (read_event->element_source->Exhausted())
            return ProcessResult::process_result_blocked;

        read_event->element_source->Read(count, out_address, out_count);

        return ProcessResult::process_result_ready;
    }

    template <>
    ProcessResult Event::ReadNext(IEvent* event, Codepoint* element)
    {
        if (event->get_type() == EventType::element_stream_ending_event)
            return ProcessResult::process_result_blocked;

        if (event->get_type() != EventType::received_codepoints_event)
            Event::HandleUnexpectedEvent("Event::ReadNext", event);

        ReceivedCodepointsEvent* read_event = (ReceivedCodepointsEvent*)event;
        if (read_event->element_source->Exhausted())
            return ProcessResult::process_result_blocked; // event consumed

        read_event->element_source->ReadNext(element);

        return ProcessResult::process_result_ready;
    }

    template <>
    void Event::AddObserver(IEvent* event, std::shared_ptr<IStream<Codepoint> > stream)
    {
        if (event->get_type() == EventType::received_codepoints_event)
        {
            ReceivedCodepointsEvent* read_event = (ReceivedCodepointsEvent*)event;
            return read_event->element_source->AddObserver(stream);
        }
        else if (event->get_type() == EventType::can_send_codepoints_event)
        {
            CanSendCodepointsEvent* write_event = (CanSendCodepointsEvent*)event;
            return write_event->element_source->AddObserver(stream);
        }

        Event::HandleUnexpectedEvent("Event::AddObserver", event);
    }

    template <>
    void Event::RemoveObserver(IEvent* event, std::shared_ptr<IStream<Codepoint> > stream)
    {
        if (event->get_type() == EventType::received_codepoints_event)
        {
            ReceivedCodepointsEvent* read_event = (ReceivedCodepointsEvent*)event;
            return read_event->element_source->RemoveObserver(stream);
        }
        else if (event->get_type() == EventType::can_send_codepoints_event)
        {
            CanSendCodepointsEvent* write_event = (CanSendCodepointsEvent*)event;
            return write_event->element_source->RemoveObserver(stream);
        }

        Event::HandleUnexpectedEvent("Event::RemoveObserver", event);
    }

    void Event::HandleUnexpectedEvent(const char* function, IEvent* event)
    {
        char message[0x100];
        sprintf_s(message, "%s unexpected event type %d", function, event->get_type());
        throw FatalError("Basic", message);
    }
}