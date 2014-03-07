// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IRefCounted.h"
#include "Tls.Types.h"

namespace Tls
{
	__interface ICertificate : public Basic::IRefCounted
	{
		Certificates* Certificates();
		bool CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult);
	};
}