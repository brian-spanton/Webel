// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Job.h"

namespace Basic
{
    Job::Job(std::shared_ptr<IJobEventHandler> event_handler, std::shared_ptr<void> context)
    {
        this->hEvent = 0;
        this->Internal = 0;
        this->InternalHigh = 0;
        this->Offset = 0;
        this->OffsetHigh = 0;

        this->event_handler = event_handler;
        this->context = context;
    }

    std::shared_ptr<Job> Job::make(std::shared_ptr<IJobEventHandler> event_handler, std::shared_ptr<void> context)
    {
        std::shared_ptr<Job> job = std::make_shared<Job>(event_handler, context);
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
        std::shared_ptr<IJobEventHandler> event_handler = this->event_handler.lock();

        if (event_handler.get() != 0)
        {
            event_handler->job_completed(this->context, count, error);
        }

        this->context.reset();
        this->self.reset();
    }
}