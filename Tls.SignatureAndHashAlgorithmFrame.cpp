// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
    using namespace Basic;

    SignatureAndHashAlgorithmFrame::SignatureAndHashAlgorithmFrame(SignatureAndHashAlgorithm* signature_and_hash_algorithm) :
        signature_and_hash_algorithm(signature_and_hash_algorithm),
        signature_algorithm_frame((byte*)&this->signature_and_hash_algorithm->signature, sizeof(this->signature_and_hash_algorithm->signature)), // initialization is in order of declaration in class def
        hash_algorithm_frame((byte*)&this->signature_and_hash_algorithm->hash, sizeof(this->signature_and_hash_algorithm->hash)) // initialization is in order of declaration in class def
    {
    }

    EventResult SignatureAndHashAlgorithmFrame::consider_event(IEvent* event)
    {
        EventResult result;

        switch (get_state())
        {
        case State::start_state:
            switch_to_state(State::signature_algorithm_state);
            break;

        case State::signature_algorithm_state:
            result = delegate_event_change_state_on_fail(&this->signature_algorithm_frame, event, State::signature_algorithm_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::name_state);
            break;

        case State::name_state:
            result = delegate_event_change_state_on_fail(&this->hash_algorithm_frame, event, State::hash_algorithm_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::SignatureAndHashAlgorithmFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}