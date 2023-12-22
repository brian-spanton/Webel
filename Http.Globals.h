// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.Types.h"
#include "Basic.ClientSocket.h"
#include "Basic.ServerSocket.h"

namespace Http
{
    using namespace Basic;

    class Globals
    {
    public:
        Globals();

        void Initialize();

        static const byte CR = '\r'; // carriage return
        static const byte LF = '\n'; // line feed
        static const byte SP = ' '; // space
        static const byte HT = '\t'; // horizontal tab
        static const byte DQ = '\"'; // double quote
        static const byte QM = '?'; // question mark
        static const byte FS = '/'; // forward slash
        static const byte SC = ';'; // semi colon
        static const byte EQ = '='; // equals
        static const byte colon = ':';

        byte CRLF[2];

        bool CHAR[0x100] = { 0 };
        bool UPALPHA[0x100] = { 0 }; // upper case letters
        bool LOALPHA[0x100] = { 0 }; // lower case letters
        bool ALPHA[0x100] = { 0 }; // letters
        bool DIGIT[0x100] = { 0 }; // digits
        bool CTL[0x100] = { 0 }; // control characters
        bool TEXT[0x100] = { 0 };
        bool HEX[0x100] = { 0 }; // hexadecimal digits
        bool TOKEN[0x100] = { 0 };
        bool SEPARATOR[0x100] = { 0 };
        bool LWS[0x100] = { 0 }; // linear white space
        bool WSP[0x100] = { 0 }; // white space
        bool COOKIE_OCTET[0x100] = { 0 };

        UnicodeStringRef header_content_length;
        UnicodeStringRef header_connection;
        UnicodeStringRef keep_alive;
        UnicodeStringRef header_content_type;
        UnicodeStringRef HTTP_1_1;
        UnicodeStringRef header_te;
        UnicodeStringRef trailers;
        UnicodeStringRef header_host;
        UnicodeStringRef reason_ok;
        UnicodeStringRef reason_request_uri;
        UnicodeStringRef reason_method;
        UnicodeStringRef reason_bad_request;
        UnicodeStringRef header_content_encoding;
        UnicodeStringRef identity;
        UnicodeStringRef header_transfer_encoding;
        UnicodeStringRef header_transfer_length;
        UnicodeStringRef chunked;
        UnicodeStringRef header_location;
        UnicodeStringRef header_set_cookie;
        UnicodeStringRef expires_av_name;
        UnicodeStringRef max_age_av_name;
        UnicodeStringRef domain_av_name;
        UnicodeStringRef path_av_name;
        UnicodeStringRef secure_av_name;
        UnicodeStringRef httponly_av_name;
        UnicodeStringRef header_cookie;

        UnicodeStringRef application_x_www_form_urlencoded_media_type;

        UnicodeStringRef head_method;
        UnicodeStringRef get_method;
        UnicodeStringRef post_method;
    };

    extern Globals* globals;
}
