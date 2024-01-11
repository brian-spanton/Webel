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

    void TailLog::add_entry(std::shared_ptr<LogEntry> entry)
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

            if (!this->tail[next])
                continue;

            this->tail[next]->render_utf32(stream);
        }
    }
}