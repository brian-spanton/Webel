// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogFrame.h"
#include "Basic.Globals.h"

namespace Basic
{
    LogFrame::LogFrame(std::shared_ptr<ILog> log) :
        log(log)
    {
        if (this->log.get() != 0)
        {
            Basic::globals->LogStream()->logs.push_back(log);
        }
    }

    LogFrame::~LogFrame()
    {
        if (this->log.get() != 0)
        {
            Basic::globals->LogStream()->logs.pop_back();
        }
    }
}