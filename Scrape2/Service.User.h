// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.MemoryLog.h"

namespace Service
{
    using namespace Basic;

    class User : public ILog
    {
    private:
        // $$$ switch to JsonLog
        std::shared_ptr<MemoryLog> memory_log;

    public:
        User();

        virtual void ILog::write_entry(UnicodeStringRef entry);
    };
}
