// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Globals.h"
#include "Basic.Utf32LittleEndianDecoder.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    Inline<Globals>* globals = 0;

    Globals::Globals()
    {
    }

    bool Globals::HandleError(const char* context, uint32 error)
    {
        if (this->error_handler.item() == 0)
            return false;

        return this->error_handler->HandleError(context, error);
    }

    Basic::IStream<Codepoint>* Globals::DebugStream()
    {
        if (this->error_handler.item() == 0)
            throw new Exception("no error handler set");

        return this->error_handler->DebugStream();
    }

    Basic::TextWriter* Globals::DebugWriter()
    {
        if (this->error_handler.item() == 0)
            throw new Exception("no error handler set");

        return this->error_handler->DebugWriter();
    }

    void Globals::GetEncoder(UnicodeString* encoding, Basic::Ref<IEncoder>* encoder)
    {
        EncoderMap::iterator it = this->encoder_map.find(encoding);
        if (it == this->encoder_map.end())
        {
            (*encoder) = 0;
            return;
        }

        it->second->CreateEncoder(encoder);
    }

    void Globals::GetDecoder(UnicodeString* encoding, Basic::Ref<IDecoder>* decoder)
    {
        DecoderMap::iterator it = this->decoder_map.find(encoding);
        if (it == this->decoder_map.end())
        {
            (*decoder) = 0;
            return;
        }

        it->second->CreateDecoder(decoder);
    }

    void Globals::Initialize(Basic::Ref<IErrorHandler> error_handler, Basic::Ref<ICompletionQueue> completion_queue)
    {
        this->error_handler = error_handler;
        this->completion_queue = completion_queue;

        this->ascii_index = New<SingleByteEncodingIndex>();
        this->ascii_index->Initialize();

        utf_16_big_endian_bom[0] = 0xFE;
        utf_16_big_endian_bom[1] = 0xFF;

        utf_16_little_endian_bom[0] = 0xFF;
        utf_16_little_endian_bom[1] = 0xFE;

        utf_8_bom[0] = 0xEF;
        utf_8_bom[1] = 0xBB;
        utf_8_bom[2] = 0xBF;

        ftp_scheme.Initialize("ftp");
        file_scheme.Initialize("file");
        gopher_scheme.Initialize("gopher");
        http_scheme.Initialize("http");
        https_scheme.Initialize("https");
        ws_scheme.Initialize("ws");
        wss_scheme.Initialize("wss");

        utf_32_big_endian_label.Initialize("utf-32be");
        utf_32_little_endian_label.Initialize("utf-32le");
        utf_16_big_endian_label.Initialize("utf-16be");
        utf_16_little_endian_label.Initialize("utf-16le");
        utf_8_label.Initialize("utf-8");
        us_ascii_label.Initialize("us-ascii");

        CRLF.Initialize("\r\n");
        percent_forty.Initialize("%40");
        percent_two_e.Initialize("%2e");
        dot_percent_two_e.Initialize(".%2e");
        percent_two_e_dot.Initialize("%2e.");
        dot_dot.Initialize("..");
        dot.Initialize(".");

        text_plain_media_type.Initialize("text/plain");
        text_html_media_type.Initialize("text/html");
        application_json_media_type.Initialize("application/json");
        charset_parameter_name.Initialize("charset");

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

        UnicodeString::Ref port;

        port.Initialize("21");
        this->scheme_to_port_map.insert(PortMap::value_type(this->ftp_scheme, port));

        port.Initialize("");
        this->scheme_to_port_map.insert(PortMap::value_type(this->file_scheme, port));

        port.Initialize("70");
        this->scheme_to_port_map.insert(PortMap::value_type(this->gopher_scheme, port));

        port.Initialize("80");
        this->scheme_to_port_map.insert(PortMap::value_type(this->http_scheme, port));
        this->scheme_to_port_map.insert(PortMap::value_type(this->ws_scheme, port));

        port.Initialize("443");
        this->scheme_to_port_map.insert(PortMap::value_type(this->https_scheme, port));
        this->scheme_to_port_map.insert(PortMap::value_type(this->wss_scheme, port));

        ZeroMemory(sanitizer_white_space, sizeof(sanitizer_white_space));

        sanitizer_white_space[' '] = true;
        sanitizer_white_space['\t'] = true;
        sanitizer_white_space['\r'] = true;
        sanitizer_white_space['\n'] = true;
        sanitizer_white_space[0xA0] = true;

        Utf32LittleEndianDecoderFactory::Ref utf_32_little_endian_decoder_factory = New<Utf32LittleEndianDecoderFactory>();
        this->decoder_map.insert(DecoderMap::value_type(utf_32_little_endian_label, utf_32_little_endian_decoder_factory.item()));

        Utf8DecoderFactory::Ref utf_8_decoder_factory = New<Utf8DecoderFactory>();
        this->decoder_map.insert(DecoderMap::value_type(utf_8_label, utf_8_decoder_factory.item()));

        Utf8EncoderFactory::Ref utf_8_encoder_factory = New<Utf8EncoderFactory>();
        this->encoder_map.insert(EncoderMap::value_type(utf_8_label, utf_8_encoder_factory.item()));
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

    void Globals::PostCompletion(Basic::ICompletion* completion, LPOVERLAPPED overlapped)
    {
        if (this->completion_queue.item() == 0)
            throw new Exception("no error handler set");

        return this->completion_queue->PostCompletion(completion, overlapped);
    }

    void Globals::QueueProcess(Basic::Ref<IProcess> process, ByteString::Ref cookie)
    {
        if (this->completion_queue.item() == 0)
            throw new Exception("no error handler set");

        return this->completion_queue->QueueProcess(process, cookie);
    }

    void Globals::BindToCompletionQueue(LogFile::Ref logfile)
    {
        if (this->completion_queue.item() == 0)
            throw new Exception("no error handler set");

        return this->completion_queue->BindToCompletionQueue(logfile);
    }

    void Globals::BindToCompletionQueue(Socket::Ref socket)
    {
        if (this->completion_queue.item() == 0)
            throw new Exception("no error handler set");

        return this->completion_queue->BindToCompletionQueue(socket);
    }
}
