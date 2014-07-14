// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogStream.h"
#include "Basic.Event.h"
#include "Basic.Frame.h"

namespace Basic
{
    void LogStream::Initialize(std::shared_ptr<LogFile> log_file)
    {
        this->log_file = log_file;
        this->entry = std::make_shared<UnicodeString>();
        this->entry->reserve(0x400);
    }

    void LogStream::write_element(Codepoint codepoint)
    {
        // prefix each line with some context
        if (this->entry->size() == 0)
        {
            TextWriter text(this->entry.get());
            text.WriteThreadId();
            text.write_literal(" ");
            text.WriteTimestamp();
            text.write_literal(" ");
        }

        this->entry->push_back(codepoint);

        // when we get to the end of a line, write the entry out
        if (codepoint == '\n')
        {
            this->log_file->write_entry(this->entry.get());

            // now the entry is ready to reuse
            this->entry->clear();
        }
    }
}