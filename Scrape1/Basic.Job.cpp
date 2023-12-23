// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Job.h"

namespace Basic
{
    Job::Job(std::shared_ptr<ICompleter> completer, std::shared_ptr<void> context)
    {
        this->hEvent = 0;
        this->Internal = 0;
        this->InternalHigh = 0;
        this->Offset = 0;
        this->OffsetHigh = 0;

        this->completer = completer;
        this->context = context;
    }

    std::shared_ptr<Job> Job::make(std::shared_ptr<ICompleter> completer, std::shared_ptr<void> context)
    {
        std::shared_ptr<Job> job = std::make_shared<Job>(completer, context);
        job->self = job->shared_from_this();
        return job;
    }

    void Job::complete(OVERLAPPED_ENTRY* entry)
    {
        Job* job = (Job*)entry->lpOverlapped;
        job->complete(entry->dwNumberOfBytesTransferred, entry->Internal);
    }

    void Job::complete(uint32 count, uint32 error)
    {
        std::shared_ptr<ICompleter> completer = this->completer.lock();

        if (completer.get() != 0)
        {
            completer->complete(this->context, count, error);
        }

        this->context.reset();
        this->self.reset();
    }
}