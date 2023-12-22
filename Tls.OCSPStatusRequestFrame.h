// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class OCSPStatusRequestFrame : public Frame
    {
    private:
        enum State
        {
            list_frame_pending_state = Start_State,
            extensions_frame_pending_state,
            done_state = Succeeded_State,
            list_frame_failed,
            extensions_frame_failed,
        };

        OCSPStatusRequest* ocsp_status_request;
        VectorFrame<ResponderIDList> list_frame;
        VectorFrame<Extensions> extensions_frame;

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        OCSPStatusRequestFrame(OCSPStatusRequest* ocsp_status_request);
    };
}