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

    ProcessResult SignatureAndHashAlgorithmFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::start_state:
            switch_to_state(State::signature_algorithm_state);
            break;

        case State::signature_algorithm_state:
            result = process_event_change_state_on_fail(&this->signature_algorithm_frame, event, State::signature_algorithm_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::name_state);
            break;

        case State::name_state:
            result = process_event_change_state_on_fail(&this->hash_algorithm_frame, event, State::hash_algorithm_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls", "SignatureAndHashAlgorithmFrame", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}