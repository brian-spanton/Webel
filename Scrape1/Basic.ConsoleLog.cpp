// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ConsoleLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    void ConsoleLog::write_entry(UnicodeStringRef entry)
    {
        ByteString bytes;
        ascii_encode(entry.get(), &bytes);
        printf((char*)bytes.c_str());
    }
}