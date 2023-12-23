#include "stdafx.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.Globals.h"
#include "Json.Parser.h"
#include "Http.Globals.h"
#include "Http.Types.h"
#include "Dynamo.Globals.h"

namespace Basic
{
	void SingleByteEncodingIndex::Initialize(Http::Uri* index_url)
	{
		__super::Initialize();

		this->client = New<Http::Client>();
		this->client->Initialize();

		this->client->Get(index_url, this, (ByteString*)0);
	}

	void SingleByteEncodingIndex::Process(IEvent* event, bool* yield)
	{
		(*yield = true);

		switch (event->get_type())
		{
		case Http::EventType::response_headers_event:
			{
				Http::Response::Ref response = this->client->history.back().response;
				if (response->code != 200)
				{
					Uri::Ref url;
					this->client->get_url(&url);

					url->SerializeTo(Dynamo::globals->DebugStream(), 0, 0);
					Dynamo::globals->DebugWriter()->WriteLine(" did not return 200");

					switch_to_state(State::done_state);
					break;
				}

				FrameStream<byte>::Ref frame_stream = New<FrameStream<byte> >();
				frame_stream->Initialize(this);

				this->client->set_body_stream(frame_stream);

				if (frame_state() != State::line_start_state)
					switch_to_state(State::line_start_state);
			}
			break;

		case Basic::EventType::ready_for_read_bytes_event:
			{
				(*yield = false);

				switch (frame_state())
				{
				case State::line_start_state:
					{
						byte b;
						if (!Event::ReadNext(event, &b, yield))
							return;

						if (b == '#')
						{
							switch_to_state(State::ignore_line_state);
						}
						else if (b == Http::globals->LF)
						{
						}
						else if (Http::globals->WSP[b])
						{
							switch_to_state(State::before_index_state);
						}
						else if (Http::globals->DIGIT[b])
						{
							Event::UndoReadNext(event);
							this->index_stream.Initialize(&this->index);
							switch_to_state(State::index_pending_state);
						}
						else
						{
							switch_to_state(State::line_start_error);
						}
					}
					break;

				case State::ignore_line_state:
					{
						byte b;
						if (!Event::ReadNext(event, &b, yield))
							return;

						if (b == Http::globals->LF)
						{
							switch_to_state(State::line_start_state);
						}
					}
					break;

				case State::before_index_state:
					{
						byte b;
						if (!Event::ReadNext(event, &b, yield))
							return;

						if (Http::globals->WSP[b])
						{
						}
						else if (Http::globals->DIGIT[b])
						{
							Event::UndoReadNext(event);
							this->index_stream.Initialize(&this->index);
							switch_to_state(State::index_pending_state);
						}
						else
						{
							switch_to_state(State::before_index_error);
						}
					}
					break;

				case State::index_pending_state:
					{
						byte b;
						if (!Event::ReadNext(event, &b, yield))
							return;

						bool success = this->index_stream.WriteDigit(b);
						if (!success)
						{
							if (this->index_stream.get_digit_count() == 0)
							{
								switch_to_state(State::index_pending_error);
							}
							else
							{
								Event::UndoReadNext(event);
								switch_to_state(State::before_codepoint_state);
							}
						}
					}
					break;

				case State::before_codepoint_state:
					{
						byte b;
						if (!Event::ReadNext(event, &b, yield))
							return;

						if (Http::globals->WSP[b])
						{
						}
						else if (b == '0')
						{
						}
						else if (b == 'x')
						{
							this->codepoint_stream.Initialize(&this->codepoint);
							switch_to_state(State::codepoint_pending_state);
						}
						else
						{
							switch_to_state(State::before_codepoint_error);
						}
					}
					break;

				case State::codepoint_pending_state:
					{
						byte b;
						if (!Event::ReadNext(event, &b, yield))
							return;

						bool success = this->codepoint_stream.WriteDigit(b);
						if (!success)
						{
							if (this->codepoint_stream.get_digit_count() == 0)
							{
								switch_to_state(State::codepoint_pending_error);
							}
							else
							{
								this->index_map[this->index] = this->codepoint;
								this->codepoint_map.insert(CodepointMap::value_type(this->codepoint, this->index));
								switch_to_state(State::ignore_line_state);
							}
						}
					}
					break;

				default:
					throw new Exception("Basic::SingleByteEncodingIndex::Complete unexpected state");
				}
			}
			break;

		case Basic::EventType::element_stream_ending_event:
			break;

		case Http::EventType::response_complete_event:
			switch_to_state(State::done_state);
			break;

		default:
			throw new Exception("unexpected event");
		}
	}

	Codepoint SingleByteEncodingIndex::byte_to_codepoint(byte b)
	{
		return this->index_map[b];
	}

	byte SingleByteEncodingIndex::codepoint_to_byte(Codepoint c)
	{
		CodepointMap::iterator it = this->codepoint_map.find(c);
		if (it == this->codepoint_map.end())
			return 0;

		return it->second;
	}
}