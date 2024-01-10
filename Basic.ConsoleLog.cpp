// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ConsoleLog.h"
#include "Basic.Globals.h"
#include "Basic.Utf16Encoder.h"

namespace Basic
{
    void ConsoleLog::add_entry(std::shared_ptr<LogEntry> entry)
    {
        std::wstring output;
        entry->render_utf16(&output);
        output.push_back('\n');
        wprintf_s(output.c_str());
    }
}