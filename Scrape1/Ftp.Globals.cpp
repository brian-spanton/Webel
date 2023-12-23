// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Ftp.Globals.h"

namespace Ftp
{
    Globals* globals = 0;

    Globals::Globals()
    {
    }

    void Globals::Initialize()
    {
        initialize_ascii(&greeting, "220 awaiting input\r\n");
    }
}