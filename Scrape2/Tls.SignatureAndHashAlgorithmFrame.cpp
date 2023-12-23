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

	ConsumeElementsResult SignatureAndHashAlgorithmFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::signature_algorithm_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->signature_algorithm_frame, element_source, this, State::signature_algorithm_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::hash_algorithm_state);
			return ConsumeElementsResult::in_progress;
		}

		case State::hash_algorithm_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->hash_algorithm_frame, element_source, this, State::hash_algorithm_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("Tls::SignatureAndHashAlgorithmFrame::handle_event unexpected state");
        }
    }
}