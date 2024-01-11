// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    enum LogLevel
    {
        Debug, // very verbose, when debugging a mysterious symptom, or bad inputs
        Info, // informational messages about interesting states or events
        Error, // a non-critical error, *not* caused by untrusted input
        Alert, // a condition that a human should be alerted to
        Critical, // a severe (or even fatal) impact to functionality
    };

    class LogEntry
    {
    private:
        DWORD thread;
        SYSTEMTIME time = { 0 };
        std::shared_ptr<std::string> component;

    public:
        LogLevel level = LogLevel::Debug;
        std::string ascii_message;
        UnicodeString unicode_message;
        uint32 code = 0;

        LogEntry(LogLevel level, const char* component);
        LogEntry(LogLevel level, const char* component, const char* message, uint32 code);

        void render_ascii(std::string* output);
        void render_utf8(ByteString* output);
        void render_utf16(std::wstring* output);
        void render_utf32(IStream<Codepoint>* output);
    };

    void LogInfo(const char* component, const char* message);
    void LogDebug(const char* component, const char* message);
    void LogDebug(const char* component, const char* message, uint32 code);
    void LogError(const char* component, const char* message);
    void LogError(const char* component, const char* message, uint32 code);
    void LogAlert(const char* component, const char* message);
    void LogCritical(const char* component, const char* message);
    void LogCritical(const char* component, const char* message, uint32 code);
}