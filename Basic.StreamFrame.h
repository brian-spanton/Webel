// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Basic
{
    // the inverse of a ProcessStream, the point of StreamFrame is to present
    // any IStream as a Frame (and therefore IProcess).
    // $ it isn't actually used at the moment, should we remove it?
    template <typename element_type>
    class StreamFrame : public Frame
    {
    private:
        enum State
        {
            receiving_state = Start_State,
            done_state = Succeeded_State,
        };

        int expected = 0;
        int received = 0;
        std::shared_ptr<IStream<element_type> > destination;

    public:
        void reset(std::shared_ptr<IStream<element_type> > destination, int expected)
        {
            __super::reset();
            this->destination = destination;
            this->expected = expected;
            this->received = 0;
        }

        virtual ProcessResult IProcess::process_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::receiving_state:
                {
                    const element_type* elements;
                    uint32 useable;

                    ProcessResult result = Event::Read(event, this->expected - this->received, &elements, &useable);
                    if (result == process_result_blocked)
                        return ProcessResult::process_result_blocked;

                    destination->write_elements(elements, useable);

                    this->received += useable;

                    if (this->received == this->expected)
                    {
                        switch_to_state(State::done_state);
                        return ProcessResult::process_result_ready;
                    }
                }
                break;

            default:
                throw FatalError("Basic", "StreamFrame::process_event unhandled state");
            }

            return ProcessResult::process_result_ready;
        }
    };
}