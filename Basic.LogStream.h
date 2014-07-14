// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.LogFile.h"

namespace Basic
{
    class LogStream : public UnitStream<Codepoint>
    {
    private:
        std::shared_ptr<LogFile> log_file;
        UnicodeStringRef entry;

    public:
        void Initialize(std::shared_ptr<LogFile> log_file);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
    };
}