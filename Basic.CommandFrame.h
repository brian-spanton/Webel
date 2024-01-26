// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.Event.h"

namespace Basic
{
    template <typename element_type>
    class CommandFrame : public Frame
    {
    private:
        enum State
        {
            word_state = Start_State,
            done_state = Succeeded_State,
        };

        std::shared_ptr<String<element_type> > word;
        std::vector<std::shared_ptr<String<element_type> > >* command;

    public:
        CommandFrame(std::vector<std::shared_ptr<String<element_type> > >* command) :
            command(command)
        {
        }

        void reset()
        {
            __super::reset();
            this->word = std::make_shared<String<element_type> >();
        }

        virtual ProcessResult IProcess::process_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::word_state:
                {
                    if (event->get_type() == EventType::can_send_codepoints_event)
                        return ProcessResult::process_result_blocked;

                    element_type b;
                    ProcessResult result = Event::ReadNext(event, &b);
                    if (result == process_result_blocked)
                        return ProcessResult::process_result_blocked;

                    if (b == ' ')
                    {
                        this->command->push_back(this->word);
                        this->word = std::make_shared<String<element_type> >();
                    }
                    else if (b == '\r')
                    {
                        this->command->push_back(this->word);
                        switch_to_state(State::done_state);
                    }
                    else
                    {
                        this->word->push_back(b);
                    }
                }
                break;

            default:
                throw FatalError("Basic", "CommandFrame", "process_event", "unhandled state", this->get_state());
            }

            return ProcessResult::process_result_ready;
        }
    };
}