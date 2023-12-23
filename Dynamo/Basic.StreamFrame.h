#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Basic
{
	template <class T>
	class StreamFrame : public Frame
	{
	private:
		enum State
		{
			receiving_state = Start_State,
			done_state = Succeeded_State,
		};

		int expected;
		int received;
		Ref<IStream<T> > destination; // $$$

	public:
		typedef Basic::Ref<StreamFrame> Ref;

		void Initialize(IStream<T>* destination, int expected)
		{
			__super::Initialize();
			this->destination = destination;
			this->expected = expected;
			this->received = 0;
		}

		virtual void IProcess::Process(IEvent* event, bool* yield)
		{
			switch (frame_state())
			{
			case State::receiving_state:
				{
					const T* elements;
					uint32 useable;

					if (!Event::Read(event, this->expected - this->received, &elements, &useable, yield))
						return;

					destination->Write(elements, useable);

					this->received += useable;

					if (this->received == this->expected)
					{
						switch_to_state(State::done_state);
					}
					else
					{
						(*yield) = true;
					}
				}
				break;

			default:
				throw new Exception("Basic::StreamFrame::Process unexpected state");
			}
		}
	};
}