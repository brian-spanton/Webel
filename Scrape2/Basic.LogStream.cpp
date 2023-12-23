// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogStream.h"
#include "Basic.TextWriter.h"

namespace Basic
{
    void LogStream::write_element(Codepoint codepoint)
    {
        // prefix each line with some context
        if (this->entry.get() == 0)
        {
            this->entry = std::make_shared<UnicodeString>();
            this->entry->reserve(0x100);

            TextWriter text(this->entry.get());
            text.write_thread_id();
            text.write_literal(" ");
            text.write_timestamp();
            text.write_literal(" ");
        }

        this->entry->push_back(codepoint);

        // when we get to the end of a line, write the entry out
        if (codepoint == '\n')
        {
            for (LogList::iterator it = this->logs.begin(); it != this->logs.end(); it++)
            {
                std::shared_ptr<ILog> log = it->lock();
                if (log.get() == 0)
                    continue;

                log->write_entry(this->entry);
            }

            this->entry.reset();
        }
    }
}