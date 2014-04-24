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

        byte CR; // carriage return
        byte LF; // line feed
        byte SP; // space
        byte HT; // horizontal tab
        byte DQ; // double quote
        byte QM; // question mark
        byte FS; // forward slash
        byte SC; // semi colon
        byte EQ; // equals

        byte CRLF[2];

        bool CHAR[0x100];
        bool UPALPHA[0x100]; // upper case letters
        bool LOALPHA[0x100]; // lower case letters
        bool ALPHA[0x100]; // letters
        bool DIGIT[0x100]; // digits
        bool CTL[0x100]; // control characters
        bool TEXT[0x100];
        bool HEX[0x100]; // hexadecimal digits
        bool TOKEN[0x100];
        bool SEPARATOR[0x100];
        bool LWS[0x100]; // linear white space
        bool WSP[0x100]; // white space
        bool COOKIE_OCTET[0x100];

        UnicodeString::Ref header_content_length; // REF
        UnicodeString::Ref header_connection; // REF
        UnicodeString::Ref keep_alive; // REF
        UnicodeString::Ref header_content_type; // REF
        UnicodeString::Ref HTTP_1_1; // REF
        UnicodeString::Ref header_te; // REF
        UnicodeString::Ref trailers; // REF
        UnicodeString::Ref header_host; // REF
        UnicodeString::Ref reason_ok; // REF
        UnicodeString::Ref reason_request_uri; // REF
        UnicodeString::Ref reason_method; // REF
        UnicodeString::Ref reason_bad_request; // REF
        UnicodeString::Ref header_content_encoding; // REF
        UnicodeString::Ref identity; // REF
        UnicodeString::Ref header_transfer_encoding; // REF
        UnicodeString::Ref header_transfer_length; // REF
        UnicodeString::Ref chunked; // REF
        UnicodeString::Ref header_location; // REF
        UnicodeString::Ref header_set_cookie; // REF
        UnicodeString::Ref expires_av_name; // REF
        UnicodeString::Ref max_age_av_name; // REF
        UnicodeString::Ref domain_av_name; // REF
        UnicodeString::Ref path_av_name; // REF
        UnicodeString::Ref secure_av_name; // REF
        UnicodeString::Ref httponly_av_name; // REF
        UnicodeString::Ref header_cookie; // REF

        UnicodeString::Ref application_x_www_form_urlencoded_media_type; // REF

        UnicodeString::Ref head_method; // REF
        UnicodeString::Ref get_method; // REF
        UnicodeString::Ref post_method; // REF
    };

    extern Globals* globals;
}
