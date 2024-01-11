// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ConsoleLog.h"
#include "Basic.Globals.h"
#include "Basic.Utf16Encoder.h"

namespace Basic
{
    void ConsoleLog::add_entry(std::shared_ptr<LogEntry> entry)
    {
        if (entry->level < LogLevel::Info)
            return;

        std::wstring output;
        entry->render_utf16(&output);
        output.push_back('\n');

        int result = wprintf_s(output.c_str());
        if (result < 0)
        {
            // this can happen if trying to render un-renderable characters in the current console code page,
            // which tends to abort the attempt, leaving a dangling line... this can help keep it looking tidy.
            wprintf_s(L"\n");
        }
    }
}