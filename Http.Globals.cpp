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
        byte CRLF[] = { CR, LF };
        CopyMemory(this->CRLF, CRLF, sizeof(CRLF));

        initialize_unicode(&header_accept_type, "accept-type");
        initialize_unicode(&header_content_length, "content-length");
        initialize_unicode(&header_connection, "connection");
        initialize_unicode(&keep_alive, "keep-alive");
        initialize_unicode(&header_content_type, "content-type");
        initialize_unicode(&HTTP_1_1, "HTTP/1.1");
        initialize_unicode(&header_te, "te");
        initialize_unicode(&trailers, "trailers");
        initialize_unicode(&header_host, "host");
        initialize_unicode(&reason_ok, "ok");
        initialize_unicode(&reason_request_uri, "request-uri");
        initialize_unicode(&reason_method, "method");
        initialize_unicode(&reason_bad_request, "bad request");
        initialize_unicode(&header_content_encoding, "content-encoding");
        initialize_unicode(&identity, "identity");
        initialize_unicode(&gzip, "gzip");
        initialize_unicode(&deflate, "deflate");
        initialize_unicode(&header_transfer_encoding, "transfer-encoding");
        initialize_unicode(&header_transfer_length, "transfer-length");
        initialize_unicode(&chunked, "chunked");
        initialize_unicode(&header_location, "location");
        initialize_unicode(&header_set_cookie, "set-cookie");
        initialize_unicode(&expires_av_name, "expires");
        initialize_unicode(&max_age_av_name, "max-age");
        initialize_unicode(&domain_av_name, "domain");
        initialize_unicode(&path_av_name, "path");
        initialize_unicode(&secure_av_name, "secure");
        initialize_unicode(&httponly_av_name, "httponly");
        initialize_unicode(&header_cookie, "cookie");

        initialize_unicode(&application_x_www_form_urlencoded_media_type, "application/x-www-form-urlencoded");

        initialize_unicode(&head_method, "HEAD");
        initialize_unicode(&get_method, "GET");
        initialize_unicode(&post_method, "POST");

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