// Copyright � 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Basic
{
    template <typename element_type>
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
        std::shared_ptr<IStream<element_type> > destination;

    public:
        void Initialize(std::shared_ptr<IStream<element_type> > destination, int expected)
        {
            __super::Initialize();
            this->destination = destination;
            this->expected = expected;
            this->received = 0;
        }

        virtual void IProcess::consider_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::receiving_state:
                {
                    const element_type* elements;
                    uint32 useable;

                    Event::Read(event, this->expected - this->received, &elements, &useable);

                    destination->write_elements(elements, useable);

                    this->received += useable;

                    if (this->received == this->expected)
                    {
                        switch_to_state(State::done_state);
                        return;
                    }
                }
                break;

            default:
                throw FatalError("Basic::StreamFrame::handle_event unexpected state");
            }
        }
    };
}