// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.ResponderIDListFrame.h"

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
        Inline<ResponderIDListFrame> list_frame;
        Inline<ExtensionsFrame> extensions_frame;

    public:
        typedef Basic::Ref<OCSPStatusRequestFrame, IProcess> Ref;

        void Initialize(OCSPStatusRequest* ocsp_status_request);
        virtual void IProcess::Process(IEvent* event, bool* yield);
    };
}