// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.AsyncBytes.h"
#include "Basic.DebugLog.h"
#include "Basic.IProcess.h"

namespace Basic
{
    class DebugStream : public IProcess
    {
    private:
        DebugLog::Ref debug_log; // REF
        UnicodeString::Ref buffer; // REF

    public:
        typedef Basic::Ref<DebugStream> Ref;

        void Initialize(DebugLog* debug_log);

        virtual void IProcess::Process(IEvent* event, bool* yield);
        virtual void IProcess::Process(IEvent* event);
        virtual bool IProcess::Pending();
        virtual bool IProcess::Succeeded();
        virtual bool IProcess::Failed();
    };
}