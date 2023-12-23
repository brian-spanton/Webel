// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface ILog
    {
        void write_entry(UnicodeStringRef entry);
    };
}