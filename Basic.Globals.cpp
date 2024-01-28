// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Globals.h"
#include "Basic.Utf32LittleEndianDecoder.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    byte Globals::utf_16_big_endian_bom[2] = { 0xFE, 0xFF };
    byte Globals::utf_16_little_endian_bom[2] = { 0xFF, 0xFE };
    byte Globals::utf_8_bom[3] = { 0xEF, 0xBB, 0xBF };

    Globals* globals = 0;

    Globals::Globals()
    {
    }

    void Globals::add_entry(std::shared_ptr<LogEntry> entry)
    {
        if (!this->log)
            return;

        this->log->add_entry(entry);
    }

    void Globals::GetEncoder(std::shared_ptr<UnicodeString> encoding, std::shared_ptr<IEncoder>* encoder)
    {
        EncoderMap::iterator it = this->encoder_map.find(encoding);
        if (it == this->encoder_map.end())
        {
            (*encoder) = 0;
            return;
        }

        it->second->CreateEncoder(encoder);
    }

    void Globals::GetDecoder(std::shared_ptr<UnicodeString> encoding, std::shared_ptr<IDecoder>* decoder)
    {
        DecoderMap::iterator it = this->decoder_map.find(encoding);
        if (it == this->decoder_map.end())
        {
            (*decoder) = 0;
            return;
        }

        it->second->CreateDecoder(decoder);
    }

    void Globals::Initialize(std::shared_ptr<ILog> log, std::shared_ptr<ICompletionQueue> completion_queue)
    {
        this->log = log;
        this->completion_queue = completion_queue;

        this->ascii_index = std::make_shared<SingleByteEncodingIndex>();
        this->ascii_index->Initialize();

        initialize_unicode(&ftp_scheme, "ftp");
        initialize_unicode(&file_scheme, "file");
        initialize_unicode(&gopher_scheme, "gopher");
        initialize_unicode(&http_scheme, "http");
        initialize_unicode(&https_scheme, "https");
        initialize_unicode(&ws_scheme, "ws");
        initialize_unicode(&wss_scheme, "wss");

        initialize_unicode(&utf_32_big_endian_label, "utf-32be");
        initialize_unicode(&utf_32_little_endian_label, "utf-32le");
        initialize_unicode(&utf_16_big_endian_label, "utf-16be");
        initialize_unicode(&utf_16_little_endian_label, "utf-16le");
        initialize_unicode(&utf_8_label, "utf-8");
        initialize_unicode(&us_ascii_label, "us-ascii");
        initialize_unicode(&us_windows_1252_label, "windows-1252");

        initialize_unicode(&CRLF, "\r\n");
        initialize_unicode(&percent_forty, "%40");
        initialize_unicode(&percent_two_e, "%2e");
        initialize_unicode(&dot_percent_two_e, ".%2e");
        initialize_unicode(&percent_two_e_dot, "%2e.");
        initialize_unicode(&dot_dot, "..");
        initialize_unicode(&dot, ".");

        // $$ should use structured MediaType?
        initialize_unicode(&text_plain_media_type, "text/plain; charset=utf-8");
        initialize_unicode(&text_html_media_type, "text/html; charset=utf-8");
        initialize_unicode(&application_json_media_type, "application/json; charset=utf-8");
        initialize_unicode(&charset_parameter_name, "charset");

        ZeroMemory(this->simple_encode_anti_set, sizeof(this->simple_encode_anti_set));

        for (Codepoint codepoint = 0x0020; codepoint <= 0x007E; codepoint++)
            this->simple_encode_anti_set[codepoint] = true;

        CopyMemory(this->default_encode_anti_set, this->simple_encode_anti_set, sizeof(this->default_encode_anti_set));

        this->default_encode_anti_set[0x0020] = false;
        this->default_encode_anti_set['\"'] = false;
        this->default_encode_anti_set['#'] = false;
        this->default_encode_anti_set['<'] = false;
        this->default_encode_anti_set['>'] = false;
        this->default_encode_anti_set['?'] = false;
        this->default_encode_anti_set['`'] = false;

        CopyMemory(this->password_encode_anti_set, this->default_encode_anti_set, sizeof(this->password_encode_anti_set));

        this->password_encode_anti_set['/'] = false;
        this->password_encode_anti_set['@'] = false;
        this->password_encode_anti_set['\\'] = false;

        CopyMemory(this->username_encode_anti_set, this->password_encode_anti_set, sizeof(this->username_encode_anti_set));

        this->username_encode_anti_set[':'] = false;

        UnicodeStringRef port;

        initialize_unicode(&port, "21");
        this->scheme_to_port_map.insert(PortMap::value_type(this->ftp_scheme, port));

        initialize_unicode(&port, "");
        this->scheme_to_port_map.insert(PortMap::value_type(this->file_scheme, port));

        initialize_unicode(&port, "70");
        this->scheme_to_port_map.insert(PortMap::value_type(this->gopher_scheme, port));

        initialize_unicode(&port, "80");
        this->scheme_to_port_map.insert(PortMap::value_type(this->http_scheme, port));
        this->scheme_to_port_map.insert(PortMap::value_type(this->ws_scheme, port));

        initialize_unicode(&port, "443");
        this->scheme_to_port_map.insert(PortMap::value_type(this->https_scheme, port));
        this->scheme_to_port_map.insert(PortMap::value_type(this->wss_scheme, port));

        ZeroMemory(sanitizer_white_space, sizeof(sanitizer_white_space));

        sanitizer_white_space[' '] = true;
        sanitizer_white_space['\t'] = true;
        sanitizer_white_space['\r'] = true;
        sanitizer_white_space['\n'] = true;
        sanitizer_white_space[0xA0] = true;

        std::shared_ptr<Utf32LittleEndianDecoderFactory> utf_32_little_endian_decoder_factory = std::make_shared<Utf32LittleEndianDecoderFactory>();
        this->decoder_map.insert(DecoderMap::value_type(utf_32_little_endian_label, utf_32_little_endian_decoder_factory));

        std::shared_ptr<Utf8DecoderFactory> utf_8_decoder_factory = std::make_shared<Utf8DecoderFactory>();
        this->decoder_map.insert(DecoderMap::value_type(utf_8_label, utf_8_decoder_factory));

        std::shared_ptr<Utf8EncoderFactory> utf_8_encoder_factory = std::make_shared<Utf8EncoderFactory>();
        this->encoder_map.insert(EncoderMap::value_type(utf_8_label, utf_8_encoder_factory));
    }

    void Globals::InitializeSocketApi()
    {
        Basic::LogInfo("Basic", "Globals", "InitializeSocketApi", "initializing socket library");

        WSADATA wsaData;
        int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (error != 0)
            throw FatalError("Basic", "Globals", "InitializeSocketApi", "WSAStartup", error);

        SOCKET tempSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (tempSocket == INVALID_SOCKET)
            throw FatalError("Basic", "Globals", "InitializeSocketApi", "::socket", WSAGetLastError());

        uint32 count;

        GUID funcId = WSAID_CONNECTEX;
        error = WSAIoctl(
            tempSocket,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &funcId,
            sizeof(funcId),
            &ConnectEx,
            sizeof(ConnectEx),
            &count,
            0,
            0);
        if (error == SOCKET_ERROR)
            throw FatalError("Basic", "Globals", "InitializeSocketApi", "WSAIoctl", WSAGetLastError());

        closesocket(tempSocket);
    }

    void Globals::QueueJob(std::shared_ptr<Job> job)
    {
        if (!this->completion_queue)
            throw FatalError("Basic", "Globals", "QueueJob", "!this->completion_queue");

        return this->completion_queue->QueueJob(job);
    }

    void Globals::BindToCompletionQueue(HANDLE handle)
    {
        if (!this->completion_queue)
            throw FatalError("Basic", "Globals", "BindToCompletionQueue", "!this->completion_queue");

        return this->completion_queue->BindToCompletionQueue(handle);
    }
}
