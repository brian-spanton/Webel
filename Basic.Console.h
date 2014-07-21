// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Lock.h"
#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"

namespace Basic
{
    class Console : public ArrayStream<Codepoint>
    {
    private:
        Lock lock;
        HANDLE output;
        HANDLE input;
        DWORD originalMode;
        std::shared_ptr<IProcess> protocol;
        ElementSource<Codepoint> protocol_element_source;

        static DWORD WINAPI Thread(void* param);
        bool TryInitialize(std::shared_ptr<IProcess> protocol, HANDLE* createdThread);

    public:
        Console();
        virtual ~Console();

        void Initialize(std::shared_ptr<IProcess> protocol, HANDLE* createdThread);
        bool Thread();

        virtual void IStream<Codepoint>::write_elements(const Codepoint* elements, uint32 count);
        virtual void IStream<Codepoint>::write_eof();
    };
}