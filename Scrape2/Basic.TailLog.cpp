// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.TailLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    TailLog::TailLog() :
        bookmark(0)
    {
    }

    void TailLog::write_entry(UnicodeStringRef entry)
    {
        this->tail[this->bookmark] = entry;

        // advance the circular array bookmark
        this->bookmark = (this->bookmark + 1) % _countof(this->tail);
    }

    void TailLog::write_to_stream(IStream<Codepoint>* stream) const
    {
        for (uint32 i = 0; i < _countof(this->tail); i++)
        {
            uint32 next = (this->bookmark + i) % _countof(this->tail);

            if (this->tail[next].get() == 0)
                continue;

            this->tail[next]->write_to_stream(stream);
        }
    }

    void TailLog::WriteTo(ILog* log)
    {
        for (uint32 i = 0; i < _countof(this->tail); i++)
        {
            uint32 next = (this->bookmark + i) % _countof(this->tail);

            if (this->tail[next].get() == 0)
                continue;

            log->write_entry(this->tail[next]);
        }
    }
}