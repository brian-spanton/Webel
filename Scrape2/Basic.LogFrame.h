// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class LogFrame
    {
    private:
        std::shared_ptr<ILog> log;

    public:
        LogFrame(std::shared_ptr<ILog> log);
        ~LogFrame();
    };
}