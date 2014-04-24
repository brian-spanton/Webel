// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IRefCounted.h"

namespace Basic
{
    __interface ICompletion : public IRefCounted
    {
        void CompleteAsync(OVERLAPPED_ENTRY& entry);
    };
}