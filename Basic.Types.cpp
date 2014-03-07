// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Types.h"
#include "Basic.Globals.h"

namespace Basic
{
	Exception::Exception(const char* context)
	{
		Basic::globals->HandleError(context, 0);
	}

	Exception::Exception(const char* context, uint32 error)
	{
		Basic::globals->HandleError(context, error);
	}

	bool HandleError(const char* context)
	{
		return Basic::globals->HandleError(context, 0);
	}

	uint32 ProcessEvent::get_type()
	{
		return EventType::process_event;
	}

	uint32 ReadyForReadBytesEvent::get_type()
	{
		return EventType::ready_for_read_bytes_event;
	}

	void ReadyForReadBytesEvent::Initialize(IElementSource<byte>* element_source)
	{
		this->element_source = element_source;
	}

	uint32 ReadyForWriteBytesEvent::get_type()
	{
		return EventType::ready_for_write_bytes_event;
	}

	void ReadyForWriteBytesEvent::Initialize(IElementSource<byte>* element_source)
	{
		this->element_source = element_source;
	}

	uint32 ReadyForReadCodepointsEvent::get_type()
	{
		return EventType::ready_for_read_codepoints_event;
	}

	void ReadyForReadCodepointsEvent::Initialize(IElementSource<Codepoint>* element_source)
	{
		this->element_source = element_source;
	}

	uint32 ReadyForWriteCodepointsEvent::get_type()
	{
		return EventType::ready_for_write_codepoints_event;
	}

	void ReadyForWriteCodepointsEvent::Initialize(IElementSource<Codepoint>* element_source)
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
