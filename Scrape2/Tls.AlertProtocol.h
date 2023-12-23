// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Tls.AlertFrame.h"
#include "Basic.ITransportEventHandler.h"

namespace Tls
{
    using namespace Basic;

    class RecordLayer;

    class AlertProtocol : public ITransportEventHandler<byte>, public IElementConsumer<byte>
    {
    protected:
        RecordLayer* session;
        Alert alert;
        std::shared_ptr<AlertFrame> alert_frame;

    public:
        AlertProtocol(RecordLayer* session);

		void ITransportEventHandler<byte>::transport_connected();
		void ITransportEventHandler<byte>::transport_disconnected();
		void ITransportEventHandler<byte>::transport_received(const byte* elements, uint32 count);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};
}