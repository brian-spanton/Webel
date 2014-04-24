// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.TextWriter.h"
#include "Http.CookieParser.h"
#include "Http.Globals.h"
#include "Basic.ClientSocket.h"
#include "Basic.ServerSocket.h"

namespace Http
{
    using namespace Basic;

    // $ this is here because it must get constructed before globals
    CookieParser::StringMap CookieParser::attr_map;

    Globals* globals = 0;

    Globals::Globals()
    {
    }

    void Globals::Initialize()
    {
        CR = '\r';
        LF = '\n';
        SP = ' ';
        HT = '\t';
        DQ = '\"';
        QM = '?';
        FS = '/';
        SC = ';';
        EQ = '=';

        byte CRLF[] = { CR, LF };
        CopyMemory(this->CRLF, CRLF, sizeof(CRLF));

        header_content_length.Initialize("content-length");
        header_connection.Initialize("connection");
        keep_alive.Initialize("keep-alive");
        header_content_type.Initialize("content-type");
        HTTP_1_1.Initialize("HTTP/1.1");
        header_te.Initialize("te");
        trailers.Initialize("trailers");
        header_host.Initialize("host");
        reason_ok.Initialize("ok");
        reason_request_uri.Initialize("request-uri");
        reason_method.Initialize("method");
        reason_bad_request.Initialize("bad request");
        header_content_encoding.Initialize("content-encoding");
        identity.Initialize("identity");
        header_transfer_encoding.Initialize("transfer-encoding");
        header_transfer_length.Initialize("transfer-length");
        chunked.Initialize("chunked");
        header_location.Initialize("location");
        header_set_cookie.Initialize("set-cookie");
        expires_av_name.Initialize("expires");
        max_age_av_name.Initialize("max-age");
        domain_av_name.Initialize("domain");
        path_av_name.Initialize("path");
        secure_av_name.Initialize("secure");
        httponly_av_name.Initialize("httponly");
        header_cookie.Initialize("cookie");

        application_x_www_form_urlencoded_media_type.Initialize("application/x-www-form-urlencoded");

        head_method.Initialize("HEAD");
        get_method.Initialize("GET");
        post_method.Initialize("POST");

        ZeroMemory(CHAR, sizeof(CHAR));

        for (uint16 ch = 0; ch <= 127; ch++)
            CHAR[ch] = true;

        ZeroMemory(ALPHA, sizeof(ALPHA));
        ZeroMemory(UPALPHA, sizeof(UPALPHA));

        for (uint16 ch = 'A'; ch <= 'Z'; ch++)
        {
            UPALPHA[ch] = true;
            ALPHA[ch] = true;
        }

        ZeroMemory(LOALPHA, sizeof(LOALPHA));

        for (uint16 ch = 'a'; ch <= 'z'; ch++)
        {
            LOALPHA[ch] = true;
            ALPHA[ch] = true;
        }

        ZeroMemory(DIGIT, sizeof(DIGIT));
        ZeroMemory(HEX, sizeof(HEX));

        for (uint16 ch = '0'; ch <= '9'; ch++)
        {
            DIGIT[ch] = true;
            HEX[ch] = true;
        }

        for (uint16 ch = 'a'; ch <= 'f'; ch++)
            HEX[ch] = true;

        for (uint16 ch = 'A'; ch <= 'F'; ch++)
            HEX[ch] = true;

        ZeroMemory(CTL, sizeof(CTL));

        for (uint16 ch = 0; ch <= 31; ch++)
            CTL[ch] = true;

        CTL[127] = true;

        ZeroMemory(SEPARATOR, sizeof(SEPARATOR));

        SEPARATOR['('] = true;
        SEPARATOR[')'] = true;
        SEPARATOR['<'] = true;
        SEPARATOR['>'] = true;
        SEPARATOR['@'] = true;
        SEPARATOR[','] = true;
        SEPARATOR[';'] = true;
        SEPARATOR[':'] = true;
        SEPARATOR['\\'] = true;
        SEPARATOR[DQ] = true;
        SEPARATOR[FS] = true;
        SEPARATOR['['] = true;
        SEPARATOR[']'] = true;
        SEPARATOR[DQ] = true;
        SEPARATOR[EQ] = true;
        SEPARATOR['{'] = true;
        SEPARATOR['}'] = true;
        SEPARATOR[SP] = true;
        SEPARATOR[HT] = true;

        ZeroMemory(TOKEN, sizeof(TOKEN));

        for (uint16 ch = 0; ch < _countof(TOKEN); ch++)
        {
            if (CHAR[ch] && !CTL[ch] && !SEPARATOR[ch])
                TOKEN[ch] = true;
        }

        ZeroMemory(LWS, sizeof(LWS));

        LWS[SP] = true;
        LWS[HT] = true;
        LWS[CR] = true;
        LWS[LF] = true;

        ZeroMemory(WSP, sizeof(WSP));

        WSP[SP] = true;
        WSP[HT] = true;

        ZeroMemory(COOKIE_OCTET, sizeof(COOKIE_OCTET));

        COOKIE_OCTET[0x21] = true;

        for (uint16 ch = 0x23; ch <= 0x2B; ch++)
            COOKIE_OCTET[ch] = true;

        for (uint16 ch = 0x2D; ch <= 0x3A; ch++)
            COOKIE_OCTET[ch] = true;

        for (uint16 ch = 0x3C; ch <= 0x5B; ch++)
            COOKIE_OCTET[ch] = true;

        for (uint16 ch = 0x5D; ch <= 0x7E; ch++)
            COOKIE_OCTET[ch] = true;

        CookieParser::InitializeStatics();
    }
};