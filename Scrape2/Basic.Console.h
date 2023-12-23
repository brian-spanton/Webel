// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Lock.h"
#include "Basic.IStream.h"
#include "Basic.IThread.h"
#include "Basic.ITransportEventHandler.h"
#include "Basic.ElementSource.h"

namespace Basic
{
    class Console : public UnitStream<Codepoint>, public IThread
    {
    private:
        HANDLE output;
        HANDLE input;
        DWORD originalMode;
        std::shared_ptr<ITransportEventHandler<Codepoint> > event_handler;

        static DWORD WINAPI Thread(void* param);

    public:
        Console();
        ~Console();

        void Initialize(std::shared_ptr<ITransportEventHandler<Codepoint> > event_handler);

        virtual bool IThread::thread();
        virtual void IStream<Codepoint>::write_element(Codepoint element);
    };
}