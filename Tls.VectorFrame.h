// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Basic.CountStream.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
	using namespace Basic;

	template<class item_class, uint32 encoded_length_min, uint32 encoded_length_max = 0, uint32 length_of_encoded_length = 0>
	class VectorFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			length_frame_pending_state = Start_State,
			length_known_state,
			next_item_state,
			item_pending_state,
			items_received_state,
			done_state = Succeeded_State,
			item_frame_failed,
			length_mismatch_error,
			vector_complete_failed,
			length_frame_failed,
			unexpected_length_error,
		};

		uint32 encoded_length;
		CountStream<byte>::Ref counter; // REF
		Inline<NumberFrame<uint32, length_of_encoded_length> > length_frame;
		Basic::Ref<IProcess> item_frame; // REF

		virtual void GetItemFrame(item_class* item, Basic::Ref<IProcess>* value)
		{
			MemoryRange::Ref frame = New<MemoryRange>();
			frame->Initialize((byte*)item, sizeof(item_class));
			(*value) = frame;
		}

		virtual void GetItemSerializer(item_class* item, Basic::Ref<ISerializable>* value)
		{
			MemoryRange::Ref frame = New<MemoryRange>();
			frame->Initialize((byte*)item, sizeof(item_class));
			(*value) = frame;
		}

		virtual bool ProcessVectorComplete()
		{
			return true;
		}

	protected:
		void CalculateLength()
		{
			Inline<CountStream<byte> > countStream;
			WriteItemsTo(&countStream);

			this->encoded_length = countStream.count;

			if (this->encoded_length < encoded_length_min)
				throw new Exception("Tls::VectorFrame::Write too short");

			if (this->encoded_length > encoded_length_max)
				throw new Exception("Tls::VectorFrame::Write too long");
		}

		void WriteLengthTo(IStream<byte>* stream)
		{
			Inline<NumberFrame<uint32, length_of_encoded_length> > number;
			number.Initialize(&this->encoded_length);

			number.SerializeTo(stream);
		}

		void WriteItemsTo(IStream<byte>* stream)
		{
			for (Vector::iterator it = this->items->begin(); it != this->items->end(); it++)
			{
				Basic::Ref<ISerializable> frame;
				GetItemSerializer(&(*it), &frame);

				frame->SerializeTo(stream);
			}
		}

		void switch_to_state(IEvent* event, State state)
		{
			__super::switch_to_state(state);

			if (!Pending())
				Event::RemoveObserver<byte>(event, this->counter);
		}

	public:
		typedef Basic::Ref<VectorFrame<item_class, encoded_length_min, encoded_length_max, length_of_encoded_length>, IProcess> Ref;
		typedef std::vector<item_class> Vector;
		typedef item_class Item;

		Vector* items;

		void Initialize(Vector* items)
		{
			__super::Initialize();
			this->items = items;
			this->length_frame.Initialize(&this->encoded_length);
		}

		void Initialize(Vector* items, uint32 encoded_length)
		{
			__super::Initialize();
			this->items = items;
			this->encoded_length = encoded_length;
			__super::switch_to_state(State::length_known_state);
		}

		virtual void IProcess::Process(IEvent* event, bool* yield)
		{
			switch (frame_state())
			{
			case State::length_frame_pending_state:
				if (this->length_frame.Pending())
				{
					this->length_frame.Process(event, yield);
				}
				else if (this->length_frame.Failed())
				{
					switch_to_state(event, State::length_frame_failed);
				}
				else
				{
					switch_to_state(event, State::length_known_state);
				}
				break;

			case State::length_known_state:
				if (!(this->encoded_length >= encoded_length_min && this->encoded_length <= encoded_length_max))
				{
					switch_to_state(event, State::unexpected_length_error);
					return;
				}

				this->counter = New<CountStream<byte> >();
				Event::AddObserver<byte>(event, this->counter);

				this->items->clear();

				// since each item must be at list 1 byte big, may as well
				// reserve at least that much.  in the case where sizeof(item_class) == 1
				// this should be all the space ever needed
				this->items->reserve(this->encoded_length);

				switch_to_state(event, State::next_item_state);
				break;

			case State::next_item_state:
				{
					uint32 received = this->counter->count;

					if (received > this->encoded_length)
					{
						switch_to_state(event, State::length_mismatch_error);
					}
					else if (received == this->encoded_length)
					{
						switch_to_state(event, State::items_received_state);
					}
					else
					{
						this->items->push_back(item_class());

						GetItemFrame(&this->items->back(), &this->item_frame);

						switch_to_state(event, State::item_pending_state);
					}
				}
				break;

			case State::item_pending_state:
				if (this->item_frame->Pending())
				{
					this->item_frame->Process(event, yield);
				}
				else if (this->item_frame->Failed())
				{
					switch_to_state(event, State::item_frame_failed);
				}
				else
				{
					switch_to_state(event, State::next_item_state);
				}
				break;

			case State::items_received_state:
				{
					bool success = ProcessVectorComplete();
					if (!success)
					{
						switch_to_state(event, State::vector_complete_failed);
						return;
					}

					switch_to_state(event, State::done_state);
				}
				break;

			default:
				throw new Exception("Tls::VectorFrame unexpected state");
			}
		}

		virtual void ISerializable::SerializeTo(IStream<byte>* stream)
		{
			CalculateLength();
			WriteLengthTo(stream);
			WriteItemsTo(stream);
		}
	};
}
