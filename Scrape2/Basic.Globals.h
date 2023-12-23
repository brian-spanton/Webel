// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ICompletionQueue.h"
#include "Basic.LogStream.h"

namespace Basic
{
    class SingleByteEncodingIndex;

    class Globals : public ICompletionQueue
    {
    public:
        Globals();

        void Initialize(std::shared_ptr<ICompletionQueue> completion_queue);
        bool InitializeSocketApi();

        byte utf_16_big_endian_bom[2];
        byte utf_16_little_endian_bom[2];
        byte utf_8_bom[3];

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

        bool simple_encode_anti_set[0x100];
        bool default_encode_anti_set[0x100];
        bool password_encode_anti_set[0x100];
        bool username_encode_anti_set[0x100];

        typedef StringMapCaseSensitive<UnicodeStringRef> PortMap;
        PortMap scheme_to_port_map;

        bool sanitizer_white_space[0x100];

        std::shared_ptr<ICompletionQueue> completion_queue;

        LPFN_CONNECTEX ConnectEx;

        void GetEncoder(std::shared_ptr<UnicodeString> encoding, std::shared_ptr<IEncoder>* encoder);
        void GetDecoder(std::shared_ptr<UnicodeString> encoding, std::shared_ptr<IDecoder>* decoder);

        virtual bool HandleError(const char* context, uint32 error);
        virtual LogStream* LogStream();

        virtual void ICompletionQueue::BindToCompletionQueue(Socket* socket);
        virtual void ICompletionQueue::BindToCompletionQueue(FileLog* log_file);
        virtual void ICompletionQueue::QueueJob(std::shared_ptr<Job> job);
    };

    extern Globals* globals;
}
