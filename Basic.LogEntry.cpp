// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogEntry.h"
#include "Basic.Utf16Encoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    LogEntry::LogEntry(LogLevel level, const char* component) :
        thread(GetCurrentThreadId()),
        level(level)
    {
        GetSystemTime(&this->time);
        this->component = std::make_shared<std::string>(component); // $$$ make all of these inputs globals and passed in as native type
    }

    LogEntry::LogEntry(LogLevel level, const char* component, const char* message, uint32 code) :
        thread(GetCurrentThreadId()),
        level(level),
        ascii_message(message),
        code(code)
    {
        GetSystemTime(&this->time);
        this->component = std::make_shared<std::string>(component);
    }

    void LogEntry::render_ascii(std::string* output)
    {
        char context[0x40];
        int context_length = sprintf_s(context, "%010d %04d/%02d/%02d %02d:%02d:%02d.%03d ", 
            this->thread, 
            this->time.wYear, 
            this->time.wMonth, 
            this->time.wDay, 
            this->time.wHour, 
            this->time.wMinute, 
            this->time.wSecond, 
            this->time.wMilliseconds);

        output->reserve(sizeof(context) + this->component->length() + this->ascii_message.length() + this->unicode_message.length());

        output->append(context, context_length);

        switch (this->level)
        {
        case LogLevel::Debug:
            output->append("DEBUG");
            break;

        case LogLevel::Info:
            output->append("INFO");
            break;

        case LogLevel::Error:
            output->append("ERROR");
            break;

        case LogLevel::Alert:
            output->append("ALERT");
            break;

        case LogLevel::Critical:
            output->append("CRITICAL");
            break;

        default:
            throw FatalError("Basic", "LogEntry::render_ascii unhandled level");
        }

        output->append(": [");
        output->append(this->component->begin(), this->component->end());
        output->append("]");

        if (this->code)
        {
            char code_string[0x40];
            // $$$ want the logic from TextWriter::WriteError
            int code_string_length = sprintf_s(code_string, " (code=%d)", this->code);
            output->append(code_string, code_string_length);
        }

        if (this->ascii_message.size())
        {
            output->push_back(' ');
            output->append(this->ascii_message);
        }
        else if (this->unicode_message.size())
        {
            output->push_back(' ');
            ascii_encode(&this->unicode_message, output);
        }
    }

    void LogEntry::render_utf8(ByteString* output)
    {
        // $$ can we avoid double buffering?
        UnicodeString message;
        StdStringStream<Codepoint> input_stream(static_cast<std::basic_string<Codepoint>*>(&message));
        render_utf32(&input_stream);

        StdStringStream<byte> output_stream(output);
        utf_8_encode(&message, &output_stream);
    }

    void LogEntry::render_utf16(std::wstring* output)
    {
        wchar_t context[0x40];
        int context_length = swprintf_s(context, L"%010d %04d/%02d/%02d %02d:%02d:%02d.%03d ", 
            this->thread, 
            this->time.wYear, 
            this->time.wMonth, 
            this->time.wDay, 
            this->time.wHour, 
            this->time.wMinute, 
            this->time.wSecond, 
            this->time.wMilliseconds);

        output->reserve(sizeof(context) + this->component->length() + this->ascii_message.length() + this->unicode_message.length());

        output->append(context, context_length);

        switch (this->level)
        {
        case LogLevel::Debug:
            output->append(L"DEBUG");
            break;

        case LogLevel::Info:
            output->append(L"INFO");
            break;

        case LogLevel::Error:
            output->append(L"ERROR");
            break;

        case LogLevel::Alert:
            output->append(L"ALERT");
            break;

        case LogLevel::Critical:
            output->append(L"CRITICAL");
            break;

        default:
            throw FatalError("Basic", "LogEntry::render_ascii unhandled level");
        }

        output->append(L": [");
        output->append(this->component->begin(), this->component->end());
        output->append(L"]");

        if (this->code)
        {
            wchar_t code_string[0x40];
            // $$$ want the logic from TextWriter::WriteError
            int code_string_length = swprintf_s(code_string, L" (code=%d)", this->code);
            output->append(code_string, code_string_length);
        }

        if (this->ascii_message.size())
        {
            output->push_back(' ');
            output->append(this->ascii_message.begin(), this->ascii_message.end());
        }
        else if (this->unicode_message.size())
        {
            output->push_back(' ');
            Utf16Encoder encoder(output);
            encoder.write_elements(this->unicode_message.c_str(), this->unicode_message.length());
        }
    }

    void LogEntry::render_utf32(IStream<Codepoint>* stream)
    {
        TextWriter writer(stream);

        writer.WriteFormat<0x40>("%010d %04d/%02d/%02d %02d:%02d:%02d.%03d ", 
            this->thread, 
            this->time.wYear, 
            this->time.wMonth, 
            this->time.wDay, 
            this->time.wHour, 
            this->time.wMinute, 
            this->time.wSecond, 
            this->time.wMilliseconds);

        switch (this->level)
        {
        case LogLevel::Debug:
            writer.write_literal("DEBUG");
            break;

        case LogLevel::Info:
            writer.write_literal("INFO");
            break;

        case LogLevel::Error:
            writer.write_literal("ERROR");
            break;

        case LogLevel::Alert:
            writer.write_literal("ALERT");
            break;

        case LogLevel::Critical:
            writer.write_literal("CRITICAL");
            break;

        default:
            throw FatalError("Basic", "LogEntry::render_ascii unhandled level");
        }

        writer.write_literal(": [");
        writer.write_elements(this->component->c_str(), this->component->length());
        writer.write_literal("]");

        if (this->code)
        {
            writer.WriteFormat<0x40>(" (code=%d)", this->code);
        }

        if (this->ascii_message.size())
        {
            writer.write_literal(" ");
            writer.write_elements(this->ascii_message.c_str(), this->ascii_message.length());
        }
        else if (this->unicode_message.size())
        {
            writer.write_literal(" ");
            stream->write_elements(this->unicode_message.c_str(), this->unicode_message.length());
        }
    }

    void LogDebug(const char* component, const char* message)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Debug, component, message, 0);
        Basic::globals->add_entry(entry);
    }

    void LogDebug(const char* component, const char* message, uint32 code)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Debug, component, message, code);
        Basic::globals->add_entry(entry);
    }

    void LogInfo(const char* component, const char* message)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Info, component, message, 0);
        Basic::globals->add_entry(entry);
    }

    void LogError(const char* component, const char* message)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Error, component, message, 0);
        Basic::globals->add_entry(entry);
    }

    void LogError(const char* component, const char* message, uint32 code)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Error, component, message, code);
        Basic::globals->add_entry(entry);
    }

    void LogAlert(const char* component, const char* message)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Alert, component, message, 0);
        Basic::globals->add_entry(entry);
    }

    void LogCritical(const char* component, const char* message)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Critical, component, message, 0);
        Basic::globals->add_entry(entry);
    }

    void LogCritical(const char* component, const char* message, uint32 code)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Critical, component, message, code);
        Basic::globals->add_entry(entry);
    }
}