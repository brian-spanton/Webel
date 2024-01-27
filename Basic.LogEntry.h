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

    typedef std::vector<std::shared_ptr<std::string> > LogContext;

    class LogCallContextFrame
    {
    public:
        LogCallContextFrame(const char* frame);
        ~LogCallContextFrame();
    };

    class LogEntry
    {
    private:
        DWORD thread_id;
        SYSTEMTIME time = { 0 };
        LogContext code_context;
        LogContext call_context;

    public:
        static thread_local LogContext current_call_context;

        static void make(LogLevel level, const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);

        LogLevel level = LogLevel::Debug;
        std::string ascii_message;
        UnicodeString unicode_message;
        uint32 code = 0;

        LogEntry(LogLevel level, const char* ns, const char* cl, const char* func);

        void render_ascii(std::string* output);
        void render_utf8(ByteString* output);
        void render_utf16(std::wstring* output);
        void render_utf32(IStream<Codepoint>* output);
    };

    void LogDebug(const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);
    void LogInfo(const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);
    void LogError(const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);
    void LogAlert(const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);
    void LogCritical(const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);
}