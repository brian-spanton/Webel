// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.User.h"

namespace Service
{
    using namespace Basic;

    User::User() :
        memory_log(std::make_shared<MemoryLog>())
    {
    }

    void User::write_entry(UnicodeStringRef entry)
    {
        this->memory_log->write_entry(entry);
    }
}
