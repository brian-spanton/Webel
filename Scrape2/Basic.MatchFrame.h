// Copyright � 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    template <class result_type>
    class MatchFrame : public StateMachine, public UnitStream<Codepoint>
    {
    private:
        enum State
        {
            matching_state = Start_State,
            done_state = Succeeded_State,
        };

        typedef StringMapCaseSensitive<result_type> Dictionary;
        typedef std::vector<typename Dictionary::iterator> MatchList;

        std::shared_ptr<Dictionary> dictionary;
        typename Dictionary::iterator* value;
        uint32 matched_chars;
        MatchList remaining_possible_matches;
        typename Dictionary::iterator best_complete_match_so_far;

    public:
        MatchFrame(std::shared_ptr<Dictionary> dictionary, typename Dictionary::iterator* value) :
            dictionary(dictionary),
            value(value),
            matched_chars(0),
            best_complete_match_so_far(this->dictionary->end()) // initialization is in order of declaration in class def
        {
            (*this->value) = this->best_complete_match_so_far;
        }

        void reset()
        {
            __super::reset();

            this->matched_chars = 0;
            this->best_complete_match_so_far = this->dictionary->end();
            (*this->value) = this->best_complete_match_so_far;
        }

        virtual void IStream<Codepoint>::write_element(Codepoint c)
        {
            switch (get_state())
            {
            case State::matching_state:
                {
                    if (this->matched_chars == 0)
                    {
                        for (Dictionary::iterator it = this->dictionary->begin(); it != this->dictionary->end(); it++)
                        {
                            if (it->first->at(this->matched_chars) == c)
                            {
                                if (this->matched_chars + 1 == it->first->size())
                                {
                                    this->best_complete_match_so_far = it;
                                    (*this->value) = this->best_complete_match_so_far;
                                }
                                else
                                {
                                    this->remaining_possible_matches.push_back(it);
                                }
                            }
                        }
                    }
                    else
                    {
                        for (MatchList::iterator it = this->remaining_possible_matches.begin(); it != this->remaining_possible_matches.end(); )
                        {
                            if ((*it)->first->at(this->matched_chars) == c)
                            {
                                if (this->matched_chars + 1 == (*it)->first->size())
                                {
                                    this->best_complete_match_so_far = (*it);
                                    (*this->value) = this->best_complete_match_so_far;
                                    it = this->remaining_possible_matches.erase(it);
                                }
                                else
                                {
                                    it++;
                                }
                            }
                            else
                            {
                                it = this->remaining_possible_matches.erase(it);
                            }
                        }
                    }

                    this->matched_chars++;

                    if (this->remaining_possible_matches.size() == 0)
                    {
                        switch_to_state(State::done_state);
                    }
                }
                break;

            default:
                throw FatalError("Basic::MatchFrame::handle_event unexpected state");
            }
        }
    };
}
