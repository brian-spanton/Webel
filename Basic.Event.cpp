// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Event.h"
#include "Basic.Frame.h"

namespace Basic
{
    template <>
    EventResult Event::Read(IEvent* event, uint32 count, const byte** out_address, uint32* out_count)
    {
        if (event->get_type() != EventType::received_bytes_event)
        {
            HandleError("unexpected event");
            throw FatalError("unexpected event");
            //return EventResult::event_result_yield; // unexpected event
        }

        ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;

        if (read_event->element_source->Exhausted())
            return EventResult::event_result_yield;

        read_event->element_source->Read(count, out_address, out_count);

        return EventResult::event_result_continue;
    }

    template <>
    EventResult Event::ReadNext(IEvent* event, byte* element)
    {
        if (event->get_type() != EventType::received_bytes_event)
        {
            HandleError("unexpected event");
            return EventResult::event_result_yield; // unexpected event
        }

        ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;
        if (read_event->element_source->Exhausted())
            return EventResult::event_result_yield; // event consumed

        read_event->element_source->ReadNext(element);

        return EventResult::event_result_continue;
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

        throw FatalError("Basic::Event::AddObserver");
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

        throw FatalError("Basic::Event::RemoveObserver");
    }

    template <>
    EventResult Event::Read(IEvent* event, uint32 count, const Codepoint** out_address, uint32* out_count)
    {
        if (event->get_type() != EventType::received_codepoints_event)
        {
            HandleError("unexpected event");
            throw FatalError("unexpected event");
            //return EventResult::event_result_yield; // unexpected event
        }

        ReceivedCodepointsEvent* read_event = (ReceivedCodepointsEvent*)event;

        if (read_event->element_source->Exhausted())
            return EventResult::event_result_yield;

        read_event->element_source->Read(count, out_address, out_count);

        return EventResult::event_result_continue;
    }

    template <>
    EventResult Event::ReadNext(IEvent* event, Codepoint* element)
    {
        if (event->get_type() != EventType::received_codepoints_event)
        {
            HandleError("unexpected event");
            throw FatalError("unexpected event");
            //return EventResult::event_result_yield; // unexpected event
        }

        ReceivedCodepointsEvent* read_event = (ReceivedCodepointsEvent*)event;
        if (read_event->element_source->Exhausted())
            return EventResult::event_result_yield; // event consumed

        read_event->element_source->ReadNext(element);

        return EventResult::event_result_continue;
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

        throw FatalError("Basic::Event::AddObserver");
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

        throw FatalError("Basic::Event::RemoveObserver");
    }
}