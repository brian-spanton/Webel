// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Socket.h"
#include "Basic.FileLog.h"
#include "Basic.Job.h"

namespace Basic
{
    __interface ICompletionQueue
    {
        void BindToCompletionQueue(Socket* socket);
        void BindToCompletionQueue(FileLog* log_file);
        void QueueJob(std::shared_ptr<Job> job);
    };
}