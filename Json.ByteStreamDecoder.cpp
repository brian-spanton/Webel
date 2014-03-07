// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ByteStreamDecoder.h"
#include "Json.Globals.h"
#include "Basic.MemoryRange.h"
#include "Basic.StreamFrame.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"

namespace Json
{
	using namespace Basic;

	void ByteStreamDecoder::Initialize(UnicodeString::Ref charset, IStream<Codepoint>* output)
	{
		__super::Initialize();
		this->charset = charset;
		this->output = output;
		this->bom_frame.Initialize(this->bom, sizeof(this->bom));
	}

	void ByteStreamDecoder::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::unconsume_not_initialized_state:
			{
				this->unconsume = New<ByteString>();
				this->unconsume->reserve(1024);
				Event::AddObserver<byte>(event, this->unconsume);

				switch_to_state(State::bom_frame_pending_state);
			}
			break;

		case State::bom_frame_pending_state:
			if (this->bom_frame.Pending())
			{
				this->bom_frame.Process(event, yield);
			}
			else if (this->bom_frame.Failed())
			{
				Event::RemoveObserver<byte>(event, this->unconsume);
				switch_to_state(State::bom_frame_failed);
			}
			else
			{
				Event::RemoveObserver<byte>(event, this->unconsume);

				if (bom[0] == 0 && bom[1] == 0 && bom[2] == 0 && bom[3] != 0)
				{
					this->encoding = Basic::globals->utf_32_big_endian_label;
				}
				else if (bom[0] == 0 && bom[1] != 0 && bom[2] == 0 && bom[3] != 0)
				{
					this->encoding = Basic::globals->utf_16_big_endian_label;
				}
				else if (bom[0] != 0 && bom[1] == 0 && bom[2] == 0 && bom[3] == 0)
				{
					this->encoding = Basic::globals->utf_32_little_endian_label;
				}
				else if (bom[0] != 0 && bom[1] == 0 && bom[2] != 0 && bom[3] == 0)
				{
					this->encoding = Basic::globals->utf_16_little_endian_label;
				}
				else if (bom[0] != 0 && bom[1] != 0 && bom[2] != 0 && bom[3] != 0)
				{
					this->encoding = Basic::globals->utf_8_label;
				}
				else if (this->charset.item() != 0)
				{
					this->encoding = this->charset;
				}
				else
				{
					switch_to_state(State::could_not_guess_encoding_error);
					return;
				}

				Basic::globals->GetDecoder(this->encoding, &this->decoder);
				if (this->decoder.item() == 0)
				{
					switch_to_state(State::could_not_find_decoder_error);
				}
				else
				{
					this->decoder->set_destination(this->output);

					this->decoder->Write(this->unconsume->c_str(), this->unconsume->size());
					switch_to_state(State::decoding_byte_stream);
				}
			}
			break;

		case State::decoding_byte_stream:
			{
				const byte* elements;
				uint32 count;

				if (!Event::Read(event, 0xffffffff, &elements, &count, yield))
					return;

				this->decoder->Write(elements, count);
				(*yield) = true;
			}
			break;

		default:
			throw new Exception("Json::ByteStreamDecoder::Process unexpected state");
		}
	}
}