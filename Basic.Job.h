// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ICompleter.h"

namespace Basic
{
    class ServerSocket;

    class Job : public OVERLAPPED, public std::enable_shared_from_this<Job>
    {
    private:
        void complete(uint32 count, uint32 error);

        std::shared_ptr<Job> self_ref;
        std::shared_ptr<ICompleter> completer;
        std::shared_ptr<void> context;

    public:
        static void complete(OVERLAPPED_ENTRY* entry);
        static std::shared_ptr<Job> make(std::shared_ptr<ICompleter> completer, std::shared_ptr<void> context);

        Job(std::shared_ptr<ICompleter> completer, std::shared_ptr<void> context);
    };
}