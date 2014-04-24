// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatExtensionFrame.h"

namespace Tls
{
    using namespace Basic;

    void HeartbeatExtensionFrame::Initialize(HeartbeatExtension* heartbeat_extension)
    {
        __super::Initialize();
        this->heartbeat_extension = heartbeat_extension;
    }

    void HeartbeatExtensionFrame::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::start_state:
            (*yield) = false;
            this->mode_frame.Initialize(&this->heartbeat_extension->mode);
            switch_to_state(State::mode_frame_pending_state);
            break;

        case State::mode_frame_pending_state:
            if (this->mode_frame.Pending())
            {
                this->mode_frame.Process(event, yield);
            }

            if (this->mode_frame.Failed())
            {
                switch_to_state(State::mode_frame_failed);
            }
            else if (this->mode_frame.Succeeded())
            {
                switch_to_state(State::done_state);
            }
            break;

        default:
            throw new Exception("HeartbeatExtensionFrame::Process unexpected state");
        }
    }

    void HeartbeatExtensionFrame::SerializeTo(IStream<byte>* stream)
    {
        this->mode_frame.Initialize(&this->heartbeat_extension->mode);
        this->mode_frame.SerializeTo(stream);
    }
}