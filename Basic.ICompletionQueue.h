// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Socket.h"
#include "Basic.FileLog.h"
#include "Basic.Job.h"

namespace Basic
{
    __interface ICompletionQueue
    {
        void BindToCompletionQueue(HANDLE handle);
        void QueueJob(std::shared_ptr<Job> job);
    };
}