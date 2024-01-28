// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"
#include "Basic.ICompletionQueue.h"

namespace Basic
{
    class SingleByteEncodingIndex;

    class Globals : public ILog, public ICompletionQueue
    {
    public:
        Globals();

        void Initialize(std::shared_ptr<ILog> logger, std::shared_ptr<ICompletionQueue> completion_queue);
        void InitializeSocketApi();

        std::shared_ptr<std::string> component_name = std::make_shared<std::string>("Basic");

        static byte utf_16_big_endian_bom[2];
        static byte utf_16_little_endian_bom[2];
        static byte utf_8_bom[3];

        UnicodeStringRef ftp_scheme;
        UnicodeStringRef file_scheme;
        UnicodeStringRef gopher_scheme;
        UnicodeStringRef http_scheme;
        UnicodeStringRef https_scheme;
        UnicodeStringRef ws_scheme;
        UnicodeStringRef wss_scheme;

        UnicodeStringRef utf_32_big_endian_label;
        UnicodeStringRef utf_32_little_endian_label;
        UnicodeStringRef utf_16_big_endian_label;
        UnicodeStringRef utf_16_little_endian_label;
        UnicodeStringRef utf_8_label;
        UnicodeStringRef us_ascii_label;
        UnicodeStringRef us_windows_1252_label;

        UnicodeStringRef CRLF;
        UnicodeStringRef percent_forty;
        UnicodeStringRef percent_two_e;
        UnicodeStringRef dot_percent_two_e;
        UnicodeStringRef percent_two_e_dot;
        UnicodeStringRef dot_dot;
        UnicodeStringRef dot;

        UnicodeStringRef text_plain_media_type;
        UnicodeStringRef text_html_media_type;
        UnicodeStringRef application_json_media_type;
        UnicodeStringRef charset_parameter_name;

        EncoderMap encoder_map;
        DecoderMap decoder_map;

        std::shared_ptr<SingleByteEncodingIndex> ascii_index;

        bool simple_encode_anti_set[0x100] = { 0 };
        bool default_encode_anti_set[0x100] = { 0 };
        bool password_encode_anti_set[0x100] = { 0 };
        bool username_encode_anti_set[0x100] = { 0 };

        typedef StringMapCaseSensitive<UnicodeStringRef> PortMap;
        PortMap scheme_to_port_map;

        bool sanitizer_white_space[0x100] = { 0 };

        std::shared_ptr<ILog> log;
        std::shared_ptr<ICompletionQueue> completion_queue;

        LPFN_CONNECTEX ConnectEx = 0;

        void GetEncoder(std::shared_ptr<UnicodeString> encoding, std::shared_ptr<IEncoder>* encoder);
        void GetDecoder(std::shared_ptr<UnicodeString> encoding, std::shared_ptr<IDecoder>* decoder);

        virtual void ILog::add_entry(std::shared_ptr<LogEntry> entry);

        virtual void ICompletionQueue::BindToCompletionQueue(HANDLE handle);
        virtual void ICompletionQueue::QueueJob(std::shared_ptr<Job> job);
    };

    extern Globals* globals;
}
