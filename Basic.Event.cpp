// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Event.h"
#include "Basic.Frame.h"

namespace Basic
{
    template <>
    void Event::Read(IEvent* event, uint32 count, const byte** out_address, uint32* out_count)
    {
        if (event->get_type() != EventType::ready_for_read_bytes_event)
        {
            HandleError("unexpected event");
            throw Yield("unexpected event");
        }

        ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
        read_event->element_source->Read(count, out_address, out_count);
    }

    template <>
    void Event::ReadNext(IEvent* event, byte* element)
    {
        if (event->get_type() != EventType::ready_for_read_bytes_event)
        {
            HandleError("unexpected event");
            throw Yield("unexpected event");
        }

        ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
        read_event->element_source->ReadNext(element);
    }

    template <>
    void Event::AddObserver(IEvent* event, std::shared_ptr<IStream<byte> > stream)
    {
        if (event->get_type() == EventType::ready_for_read_bytes_event)
        {
            ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
            return read_event->element_source->AddObserver(stream);
        }
        else if (event->get_type() == EventType::ready_for_write_bytes_event)
        {
            ReadyForWriteBytesEvent* write_event = (ReadyForWriteBytesEvent*)event;
            return write_event->element_source->AddObserver(stream);
        }

        throw FatalError("Basic::Event::AddObserver");
    }

    template <>
    void Event::RemoveObserver(IEvent* event, std::shared_ptr<IStream<byte> > stream)
    {
        if (event->get_type() == EventType::ready_for_read_bytes_event)
        {
            ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
            return read_event->element_source->RemoveObserver(stream);
        }
        else if (event->get_type() == EventType::ready_for_write_bytes_event)
        {
            ReadyForWriteBytesEvent* write_event = (ReadyForWriteBytesEvent*)event;
            return write_event->element_source->RemoveObserver(stream);
        }

        throw FatalError("Basic::Event::RemoveObserver");
    }

    template <>
    void Event::Read(IEvent* event, uint32 count, const Codepoint** out_address, uint32* out_count)
    {
        if (event->get_type() != EventType::ready_for_read_codepoints_event)
        {
            HandleError("unexpected event");
            throw Yield("unexpected event");
        }

        ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
        read_event->element_source->Read(count, out_address, out_count);
    }

    template <>
    void Event::ReadNext(IEvent* event, Codepoint* element)
    {
        if (event->get_type() != EventType::ready_for_read_codepoints_event)
        {
            HandleError("unexpected event");
            throw Yield("unexpected event");
        }

        ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
        read_event->element_source->ReadNext(element);
    }

    template <>
    void Event::AddObserver(IEvent* event, std::shared_ptr<IStream<Codepoint> > stream)
    {
        if (event->get_type() == EventType::ready_for_read_codepoints_event)
        {
            ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
            return read_event->element_source->AddObserver(stream);
        }
        else if (event->get_type() == EventType::ready_for_write_codepoints_event)
        {
            ReadyForWriteCodepointsEvent* write_event = (ReadyForWriteCodepointsEvent*)event;
            return write_event->element_source->AddObserver(stream);
        }

        throw FatalError("Basic::Event::AddObserver");
    }

    template <>
    void Event::RemoveObserver(IEvent* event, std::shared_ptr<IStream<Codepoint> > stream)
    {
        if (event->get_type() == EventType::ready_for_read_codepoints_event)
        {
            ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
            return read_event->element_source->RemoveObserver(stream);
        }
        else if (event->get_type() == EventType::ready_for_write_codepoints_event)
        {
            ReadyForWriteCodepointsEvent* write_event = (ReadyForWriteCodepointsEvent*)event;
            return write_event->element_source->RemoveObserver(stream);
        }

        throw FatalError("Basic::Event::RemoveObserver");
    }
}