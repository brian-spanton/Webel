// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.LogEntry.h"

namespace Basic
{
    __interface ILog
    {
        void add_entry(std::shared_ptr<LogEntry> entry);
    };
}