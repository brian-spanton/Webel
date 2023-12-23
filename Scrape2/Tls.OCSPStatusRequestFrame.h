// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class OCSPStatusRequestFrame : public StateMachine, public IElementConsumer<byte>
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

    public:
        OCSPStatusRequestFrame(OCSPStatusRequest* ocsp_status_request);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};
}