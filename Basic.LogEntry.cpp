// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogEntry.h"
#include "Basic.Utf16Encoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    LogCallContextFrame::LogCallContextFrame(const char* frame)
    {
        LogEntry::current_call_context.push_back(std::make_shared<std::string>(frame));
    }

    LogCallContextFrame::~LogCallContextFrame()
    {
        LogEntry::current_call_context.pop_back();
    }

    thread_local LogContext LogEntry::current_call_context;

    void LogEntry::make(LogLevel level, const char* ns, const char* cl, const char* func, const char* message, uint32 code)
    {
        std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(level, ns, cl, func);
        entry->ascii_message = message;
        entry->code = code;
        Basic::globals->add_entry(entry);
    }

    LogEntry::LogEntry(LogLevel level, const char* ns, const char* cl, const char* func) :
        thread_id(GetCurrentThreadId()),
        level(level)
    {
        this->code_context.push_back(std::make_shared<std::string>(ns));
        this->code_context.push_back(std::make_shared<std::string>(cl));
        this->code_context.push_back(std::make_shared<std::string>(func));

        this->call_context = LogEntry::current_call_context;

        GetSystemTime(&this->time);
    }

    void LogEntry::render_ascii(std::string* output)
    {
        char execution_context[0x40];
        int runtime_context_length = sprintf_s(execution_context, "%010d %04d/%02d/%02d %02d:%02d:%02d.%03d ", 
            this->thread_id, 
            this->time.wYear, 
            this->time.wMonth, 
            this->time.wDay, 
            this->time.wHour, 
            this->time.wMinute, 
            this->time.wSecond, 
            this->time.wMilliseconds);

        output->reserve(sizeof(execution_context) + 0x100 + this->ascii_message.length() + this->unicode_message.length());

        output->append("[");
        output->append(execution_context, runtime_context_length);

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
            LogCritical("Basic", "LogEntry", "render_ascii", "unhandled level", this->level);
            return;
        }

        output->append("] [");

        for (auto context_frame = this->code_context.begin(); context_frame != this->code_context.end(); context_frame++)
        {
            if (context_frame != this->code_context.begin())
                output->append("/");

            output->append((*context_frame)->begin(), (*context_frame)->end());
        }

        output->append("] [");

        for (auto context_frame = this->call_context.begin(); context_frame != this->call_context.end(); context_frame++)
        {
            if (context_frame != this->call_context.begin())
                output->append("/");

            output->append((*context_frame)->begin(), (*context_frame)->end());
        }

        output->append("]");

        if (this->code)
        {
            char code_string[0x40];
            // $$ want the logic from TextWriter::WriteError
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
        wchar_t execution_context[0x40];
        int runtime_context_length = swprintf_s(execution_context, L"%010d %04d/%02d/%02d %02d:%02d:%02d.%03d ", 
            this->thread_id, 
            this->time.wYear, 
            this->time.wMonth, 
            this->time.wDay, 
            this->time.wHour, 
            this->time.wMinute, 
            this->time.wSecond, 
            this->time.wMilliseconds);

        output->reserve(sizeof(code_context) + 0x100 + this->ascii_message.length() + this->unicode_message.length());

        output->append(L"[");
        output->append(execution_context, runtime_context_length);

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
            LogCritical("Basic", "LogEntry", "render_ascii", "unhandled level", this->level);
            return;
        }

        output->append(L"] [");

        for (auto context_frame = this->code_context.begin(); context_frame != this->code_context.end(); context_frame++)
        {
            if (context_frame != this->code_context.begin())
                output->append(L"/");

            output->append((*context_frame)->begin(), (*context_frame)->end());
        }

        output->append(L"] [");

        for (auto context_frame = this->call_context.begin(); context_frame != this->call_context.end(); context_frame++)
        {
            if (context_frame != this->call_context.begin())
                output->append(L"/");

            output->append((*context_frame)->begin(), (*context_frame)->end());
        }

        output->append(L"]");

        if (this->code)
        {
            wchar_t code_string[0x40];
            // $$ want the logic from TextWriter::WriteError
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

        writer.write_literal("[");

        writer.WriteFormat<0x40>("%010d %04d/%02d/%02d %02d:%02d:%02d.%03d ", 
            this->thread_id, 
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
            LogCritical("Basic", "LogEntry", "render_ascii", "unhandled level", this->level);
            return;
        }

        writer.write_literal("] [");

        for (auto context_frame = this->code_context.begin(); context_frame != this->code_context.end(); context_frame++)
        {
            if (context_frame != this->code_context.begin())
                writer.write_literal("/");

            writer.write_elements((*context_frame)->c_str(), (*context_frame)->length());
        }

        writer.write_literal("] [");

        for (auto context_frame = this->call_context.begin(); context_frame != this->call_context.end(); context_frame++)
        {
            if (context_frame != this->call_context.begin())
                writer.write_literal("/");

            writer.write_elements((*context_frame)->c_str(), (*context_frame)->length());
        }

        writer.write_literal("]");

        if (this->code)
        {
            writer.write_literal(" (code=");
            writer.WriteError(this->code);
            writer.write_literal(")");
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

    void LogDebug(const char* ns, const char* cl, const char* func, const char* message, uint32 code)
    {
        LogEntry::make(LogLevel::Debug, ns, cl, func, message, code);
    }

    void LogInfo(const char* ns, const char* cl, const char* func, const char* message, uint32 code)
    {
        LogEntry::make(LogLevel::Info, ns, cl, func, message, code);
    }

    void LogError(const char* ns, const char* cl, const char* func, const char* message, uint32 code)
    {
        LogEntry::make(LogLevel::Error, ns, cl, func, message, code);
    }

    void LogAlert(const char* ns, const char* cl, const char* func, const char* message, uint32 code)
    {
        LogEntry::make(LogLevel::Alert, ns, cl, func, message, code);
    }

    void LogCritical(const char* ns, const char* cl, const char* func, const char* message, uint32 code)
    {
        LogEntry::make(LogLevel::Critical, ns, cl, func, message, code);
    }
}