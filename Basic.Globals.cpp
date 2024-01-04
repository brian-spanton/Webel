// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Globals.h"
#include "Basic.Utf32LittleEndianDecoder.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    Globals* globals = 0;

    Globals::Globals()
    {
    }

    bool Globals::HandleError(const char* context, uint32 error)
    {
        if (this->error_handler.get() == 0)
            return false;

        return this->error_handler->HandleError(context, error);
    }

    Basic::IStream<Codepoint>* Globals::LogStream()
    {
        if (this->error_handler.get() == 0)
            throw FatalError("no error handler set");

        return this->error_handler->LogStream();
    }

    Basic::TextWriter* Globals::DebugWriter()
    {
        if (this->error_handler.get() == 0)
            throw FatalError("no error handler set");

        return this->error_handler->DebugWriter();
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

    void Globals::Initialize(std::shared_ptr<IErrorHandler> error_handler, std::shared_ptr<ICompletionQueue> completion_queue)
    {
        this->error_handler = error_handler;
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

        // $$$ should use structured MediaType?
        initialize_unicode(&text_plain_media_type, "text/plain; charset=utf-8");
        initialize_unicode(&text_html_media_type, "text/html; charset=utf-8");
        initialize_unicode(&application_json_media_type, "application/json; charset=utf-8");
        initialize_unicode(&charset_parameter_name, "charset");

        ZeroMemory(this->simple_encode_anti_set, sizeof(this->simple_encode_anti_set));

        for (Codepoint c = 0x0020; c <= 0x007E; c++)
            this->simple_encode_anti_set[c] = true;

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

    bool Globals::InitializeSocketApi()
    {
        WSADATA wsaData;
        int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (error != 0)
            return HandleError("WSAStartup", error);

        SOCKET tempSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (tempSocket == INVALID_SOCKET)
            return HandleError("socket", WSAGetLastError());

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
            return HandleError("socket", WSAGetLastError());

        closesocket(tempSocket);

        return true;
    }

    void Globals::QueueJob(std::shared_ptr<Job> job)
    {
        if (this->completion_queue.get() == 0)
            throw FatalError("no error handler set");

        return this->completion_queue->QueueJob(job);
    }

    void Globals::BindToCompletionQueue(HANDLE handle)
    {
        if (this->completion_queue.get() == 0)
            throw FatalError("no error handler set");

        return this->completion_queue->BindToCompletionQueue(handle);
    }
}
