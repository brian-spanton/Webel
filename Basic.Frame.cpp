// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Frame.h"

namespace Basic
{
    void Frame::switch_to_state(uint32 state)
    {
        if (this->state == state)
            throw new Exception("unexpected switch to current state");

        if (!Pending())
            throw new Exception("unexpected switch from non-pending state");

        this->state = state;

        if (Failed())
        {
            char state_string[0x40];
            sprintf_s(state_string, "state=%X", this->state);
            HandleError(state_string);
        }
    }

    void Frame::Initialize()
    {
        this->state = Start_State;
    }

    uint32 Frame::frame_state()
    {
        return this->state;
    }

    void Frame::Process(IProcess* frame, IEvent* event)
    {
        for (uint64 i = 0; true; i++)
        {
            if (i == 0x100000)
                throw new Exception("runaway frame?");

            if (!frame->Pending())
                break;

            bool yield = false;

            frame->Process(event, &yield);

            if (yield)
                break;
        }
    }

    void Frame::Process(IEvent* event)
    {
        Process(this, event);
    }

    bool Frame::Pending()
    {
        return (this->state < Succeeded_State);
    }

    bool Frame::Succeeded()
    {
        return (this->state == Succeeded_State);
    }

    bool Frame::Failed()
    {
        return (this->state > Succeeded_State);
    }
}