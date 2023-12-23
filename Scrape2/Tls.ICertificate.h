// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"

namespace Tls
{
    __interface ICertificate
    {
        Certificates* Certificates();
        bool CertDecrypt(PBYTE pbInput, DWORD cbInput, PBYTE pbOutput, DWORD cbOutput, DWORD* pcbResult);
    };
}