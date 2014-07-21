// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.DebugLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    void DebugLog::write_entry(UnicodeStringRef entry)
    {
        ByteString bytes;
        ascii_encode(entry.get(), &bytes);
        OutputDebugStringA((char*)bytes.c_str());
    }
}