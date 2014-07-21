// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.ILog.h"

namespace Basic
{
    class LogStream : public UnitStream<Codepoint>
    {
    private:
        UnicodeStringRef entry;

    public:
        typedef std::list<std::weak_ptr<ILog> > LogList;

        LogList logs;

        virtual void IStream<Codepoint>::write_element(Codepoint element);
    };
}