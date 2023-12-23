#include "stdafx.h"
#include "Basic.Event.h"

namespace Basic
{
	template <>
	bool Event::Read(IEvent* event, uint32 count, const byte** out_address, uint32* out_count, bool* yield)
	{
		if (event->get_type() == EventType::ready_for_read_bytes_event)
		{
			ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
			return read_event->element_source->Read(count, out_address, out_count, yield);
		}

		(*out_address) = 0;
		(*out_count) = 0;
		(*yield) = (count > 0);
		return false;
	}

	template <>
	bool Event::ReadNext(IEvent* event, byte* element, bool* yield)
	{
		if (event->get_type() == EventType::ready_for_read_bytes_event)
		{
			ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
			return read_event->element_source->ReadNext(element, yield);
		}

		(*yield) = true;
		return false;
	}

	template <>
	void Event::AddObserver(IEvent* event, IStream<byte>* stream)
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

		throw new Exception("Basic::Event::AddObserver");
	}

	template <>
	void Event::RemoveObserver(IEvent* event, IStream<byte>* stream)
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

		throw new Exception("Basic::Event::RemoveObserver");
	}

	template <>
	bool Event::Read(IEvent* event, uint32 count, const Codepoint** out_address, uint32* out_count, bool* yield)
	{
		if (event->get_type() == EventType::ready_for_read_codepoints_event)
		{
			ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
			return read_event->element_source->Read(count, out_address, out_count, yield);
		}

		(*out_address) = 0;
		(*out_count) = 0;
		(*yield) = (count > 0);
		return false;
	}

	template <>
	bool Event::ReadNext(IEvent* event, Codepoint* element, bool* yield)
	{
		if (event->get_type() == EventType::ready_for_read_codepoints_event)
		{
			ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
			return read_event->element_source->ReadNext(element, yield);
		}

		(*yield) = true;
		return false;
	}

	template <>
	void Event::AddObserver(IEvent* event, IStream<Codepoint>* stream)
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

		throw new Exception("Basic::Event::AddObserver");
	}

	template <>
	void Event::RemoveObserver(IEvent* event, IStream<Codepoint>* stream)
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

		throw new Exception("Basic::Event::RemoveObserver");
	}

	void Event::UndoReadNext(IEvent* event)
	{
		if (event->get_type() == EventType::ready_for_read_bytes_event)
		{
			ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;
			return read_event->element_source->UndoReadNext();
		}
		else if (event->get_type() == EventType::ready_for_read_codepoints_event)
		{
			ReadyForReadCodepointsEvent* read_event = (ReadyForReadCodepointsEvent*)event;
			return read_event->element_source->UndoReadNext();
		}

		throw new Exception("Basic::Event::UndoReadNext");
	}
}