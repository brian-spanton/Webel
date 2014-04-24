// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Lock.h"

namespace Basic
{
    class _declspec(novtable) Hold
    {
    private:
        Lock& lock;

    public:
        Hold(Lock& lock) :
            lock(lock)
        {
            lock.Acquire();
        }

        ~Hold()
        {
            lock.Release();
        }
    };
}