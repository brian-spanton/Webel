// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.DebugLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    void DebugLog::add_entry(std::shared_ptr<LogEntry> entry)
    {
        std::wstring message;
        entry->render_utf16(&message);
        message.push_back(L'\n');
        OutputDebugStringW(message.c_str());
    }
}