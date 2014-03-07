// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.Globals.h"
#include "Http.HeadersFrame.h"
#include "Http.Types.h"
#include "Basic.Event.h"

namespace Http
{
	using namespace Basic;

	byte HeadersFrame::colon = ':';

	void HeadersFrame::Initialize(NameValueCollection* nvc)
	{
		__super::Initialize();
		this->nvc = nvc;
	}

	void HeadersFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::expecting_name_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->CR)
				{
					switch_to_state(State::expecting_LF_after_headers_state);
				}
				else if (Http::globals->TOKEN[b])
				{
					this->name = New<UnicodeString>();
					this->name->push_back(b);
					switch_to_state(State::receiving_name_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_name_error);
				}
			}
			break;

		case State::receiving_name_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == colon)
				{
					this->value = New<UnicodeString>();
					switch_to_state(State::expecting_value_state);
				}
				else if (b == Http::globals->SP || b == Http::globals->HT)
				{
					this->value = New<UnicodeString>();
					switch_to_state(State::expecting_colon_state);
				}
				else if (Http::globals->TOKEN[b])
				{
					this->name->push_back(b);
				}
				else if (b == ';')
				{
					// a particular site seems to think this is ok, despite the RFC
					this->name->push_back(b);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::receiving_name_error);
				}
			}
			break;

		case State::expecting_colon_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == colon)
				{
					switch_to_state(State::expecting_value_state);
				}
				else if (b == Http::globals->SP || b == Http::globals->HT)
				{
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_colon_error);
				}
			}
			break;

		case State::expecting_value_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->CR)
				{
					switch_to_state(State::expecting_LF_after_value_state);
				}
				else if (b == Http::globals->SP || b == Http::globals->HT)
				{
				}
				else
				{
					this->value->push_back(b);
					switch_to_state(State::receiving_value_state);
				}
			}
			break;

		case State::receiving_value_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->CR)
				{
					switch_to_state(State::expecting_LF_after_value_state);
				}
				else
				{
					this->value->push_back(b);
				}
			}
			break;

		case State::expecting_LF_after_value_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->LF)
				{
					switch_to_state(State::expecting_next_header_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_LF_after_value_error);
				}
			}
			break;

		case State::expecting_next_header_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->CR)
				{
					NameValueCollection::value_type nv(this->name, this->value);
					this->nvc->insert(nv);
					this->name = (UnicodeString*)0;
					this->value = (UnicodeString*)0;

					switch_to_state(State::expecting_LF_after_headers_state);
				}
				else if (b == Http::globals->SP || b == Http::globals->HT)
				{
					// this is called "folding", it's a continuation of the previous header line's value

					this->value->push_back(Http::globals->SP);
					switch_to_state(State::expecting_value_state);
				}
				else if (Http::globals->TOKEN[b])
				{
					NameValueCollection::value_type nv(this->name, this->value);
					this->nvc->insert(nv);
					this->value = (UnicodeString*)0;

					this->name = New<UnicodeString>();
					this->name->push_back(b);
					switch_to_state(State::receiving_name_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_next_header_error);
				}
			}
			break;

		case State::expecting_LF_after_headers_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->LF)
				{
					switch_to_state(State::done_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_LF_after_headers_error);
				}
			}
			break;

		default:
			throw new Exception("HeadersFrame::Process unexpected state");
		}
	}

	void HeadersFrame::SerializeTo(IStream<byte>* stream)
	{
		for (NameValueCollection::iterator it = this->nvc->begin(); it != this->nvc->end(); it++)
		{
			it->first->ascii_encode(stream);
			stream->Write(&colon, 1);
			stream->Write(&Http::globals->SP, 1);
			it->second->ascii_encode(stream);
			stream->Write(Http::globals->CRLF, _countof(Http::globals->CRLF));
		}

		stream->Write(Http::globals->CRLF, _countof(Http::globals->CRLF));
	}
}