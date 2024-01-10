// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    enum LogLevel
    {
        Debug,
        Alert,
        Critical,
    };

    class LogEntry
    {
    private:
        DWORD thread;
        SYSTEMTIME time = { 0 };

    public:
        LogLevel level = LogLevel::Debug;
        std::shared_ptr<std::string> component;
        std::string ascii_message;
        UnicodeString unicode_message;
        uint32 code = 0;

        LogEntry();
        LogEntry(LogLevel level, const char* component);
        LogEntry(LogLevel level, const char* component, const char* message, uint32 code);

        void render_ascii(std::string* output);
        void render_utf8(ByteString* output);
        void render_utf16(std::wstring* output);
        void render_utf32(IStream<Codepoint>* output);
    };
}