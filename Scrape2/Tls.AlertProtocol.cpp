// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertProtocol.h"
#include "Tls.RecordLayer.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"
#include "Basic.IElementConsumer.h"

namespace Tls
{
    using namespace Basic;

    AlertProtocol::AlertProtocol(RecordLayer* session) :
        session(session)
    {
		this->alert_frame = std::make_shared<AlertFrame>(&this->alert);
	}

	void AlertProtocol::transport_connected()
	{
	}

	void AlertProtocol::transport_disconnected()
	{
	}

	void AlertProtocol::transport_received(const byte* elements, uint32 count)
	{
		ElementSource<byte> element_source(elements, count);

		bool success = element_source.deliver_elements(this);
		if (!success)
		{
			this->session->DisconnectApplication();
			this->session->CloseTransport();
		}
	}

	ConsumeElementsResult AlertProtocol::consume_elements(IElementSource<byte>* element_source)
    {
		ConsumeElementsResult result = this->alert_frame->consume_elements(element_source);
		if (result != ConsumeElementsResult::succeeded)
			return result;

        switch (this->alert.description)
        {
        case AlertDescription::close_notify:
			this->session->DisconnectApplication();
			this->session->CloseTransport();
			return ConsumeElementsResult::succeeded;

		// $$ handle other alerts
		}

		this->alert_frame = std::make_shared<AlertFrame>(&this->alert);
		return ConsumeElementsResult::in_progress;
	}
}
