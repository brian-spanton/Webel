// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.Types.h"
#include "Tls.Globals.h"

namespace Tls
{
	uint32 ChangeCipherSpecEvent::get_type()
	{
		return Tls::EventType::change_cipher_spec_event;
	}
}