// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.Event.h"

namespace Basic
{
    template <typename element_type>
    class IgnoreFrame : public Frame, public IStream<element_type>
    {
    private:
        enum State
        {
            receiving_state = Start_State,
            done_state = Succeeded_State,
        };

        uint64 expected;
        uint64 received;

        virtual ProcessResult IProcess::process_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::receiving_state:
                {
                    const element_type* elements;
                    uint32 useable;

                    uint64 still_needed = this->expected - this->received;
                    uint32 count = still_needed > 0xffffffff ? 0xffffffff : (uint32)still_needed;

                    ProcessResult result = Event::Read(event, count, &elements, &useable);
                    if (result == process_result_blocked)
                        return ProcessResult::process_result_blocked;

                    this->received += useable;

                    if (this->received == this->expected)
                    {
                        switch_to_state(State::done_state);
                        return ProcessResult::process_result_ready;
                    }
                }
                break;

            default:
                throw FatalError("Basic", "IgnoreFrame", "process_event", "unhandled state", this->get_state());
            }

            return ProcessResult::process_result_ready;
        }

    public:
        IgnoreFrame() :
            received(0),
            expected(0xffffffffffffffff)
        {
        }

        void reset(uint64 expected)
        {
            __super::reset();
            this->received = 0;
            this->expected = expected;
        }
 
        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count)
        {
            this->received += count;
        }

        virtual void IStream<element_type>::write_element(element_type element)
        {
            this->received += 1;
        }

        virtual void IStream<element_type>::write_eof()
        {
        }
    };
}