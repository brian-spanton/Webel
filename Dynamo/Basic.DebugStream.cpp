#include "stdafx.h"
#include "Basic.DebugStream.h"
#include "Basic.Event.h"
#include "Basic.DebugLog.h"

namespace Basic
{
	void DebugStream::Initialize(DebugLog* debug_log)
	{
		this->debug_log = debug_log;
		this->buffer = New<UnicodeString>();
		this->buffer->reserve(0x4000);
	}

	void DebugStream::Process(IEvent* event, bool* yield)
	{
		Codepoint codepoint;
		if (!Event::ReadNext(event, &codepoint, yield))
			return;

		if (this->buffer->size() == 0)
		{
			TextWriter text(this->buffer);
			text.WriteThreadId();
			text.Write(" ");
			text.WriteTimestamp();
			text.Write(" ");
		}

		if (codepoint == '\n')
		{
			this->buffer->push_back(codepoint);

			AsyncBytes::Ref bytes = New<AsyncBytes>("4");
			bytes->Initialize(this->buffer->size() * 5 / 4);

			this->buffer->utf_8_encode(bytes);
			this->buffer->clear();
	
			this->debug_log->Write(bytes);
		}
		else
		{
			this->buffer->push_back(codepoint);
		}
	}

	void DebugStream::Process(IEvent* event)
	{
		Frame::Process(this, event);
	}

	bool DebugStream::Pending()
	{
		return true;
	}

	bool DebugStream::Succeeded()
	{
		return false;
	}

	bool DebugStream::Failed()
	{
		return false;
	}
}