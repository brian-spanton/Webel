// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ExtensionHeaderFrame.h"

namespace Tls
{
    using namespace Basic;

    void ExtensionHeaderFrame::Initialize(ExtensionHeader* extension)
    {
        __super::Initialize();
        this->extension = extension;
    }

    void ExtensionHeaderFrame::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::start_state:
            (*yield) = false;
            this->type_frame.Initialize(&this->extension->type);
            switch_to_state(State::type_frame_pending_state);
            break;

        case State::type_frame_pending_state:
            if (this->type_frame.Pending())
            {
                this->type_frame.Process(event, yield);
            }

            if (this->type_frame.Failed())
            {
                switch_to_state(State::type_frame_failed);
            }
            else if (this->type_frame.Succeeded())
            {
                this->length_frame.Initialize(&this->extension->length);
                switch_to_state(State::length_frame_pending_state);
            }
            break;

        case State::length_frame_pending_state:
            if (this->length_frame.Pending())
            {
                this->length_frame.Process(event, yield);
            }

            if (this->length_frame.Failed())
            {
                switch_to_state(State::length_frame_failed);
            }
            else if (this->length_frame.Succeeded())
            {
                switch_to_state(State::done_state);
            }
            break;

        default:
            throw new Exception("ExtensionHeaderFrame::Process unexpected state");
        }
    }

    void ExtensionHeaderFrame::SerializeTo(IStream<byte>* stream)
    {
        this->type_frame.Initialize(&this->extension->type);
        this->type_frame.SerializeTo(stream);

        this->length_frame.Initialize(&this->extension->length);
        this->length_frame.SerializeTo(stream);
    }
}