// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface IJobEventHandler
    {
        void job_completed(std::shared_ptr<void> context, uint32 count, uint32 error);
    };
}