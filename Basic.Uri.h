// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ISerializable.h"

namespace Basic
{
    class Uri : public IRefCounted
    {
    private:
        void parse_error(Codepoint c);

    public:
        static bool is_ascii_alpha(Codepoint c);
        static bool is_ascii_alphanumeric(Codepoint c);
        static bool is_ascii_digit(Codepoint c);
        static bool is_ascii_hex_digit(Codepoint c);
        static bool is_url_codepoint(Codepoint c);
        static bool is_relative_scheme(UnicodeString::Ref scheme);
        static bool is_secure_scheme(UnicodeString::Ref scheme);
        static bool is_http_scheme(UnicodeString::Ref scheme);
        static void percent_encode(byte b, IStream<Codepoint>* result);
        static void percent_decode(UnicodeString* string, IStream<byte>* bytes);
        static void utf_8_percent_encode(Codepoint codepoint, const bool (&anti_set)[0x100], IStream<Codepoint>* result);
        static bool host_parse(UnicodeString::Ref input, IStream<Codepoint>* result);

        enum State
        {
            scheme_start_state,
            scheme_state,
            scheme_data_state,
            no_scheme_state,
            relative_or_authority_state,
            relative_state,
            relative_slash_state,
            authority_first_slash_state,
            authority_second_slash_state,
            authority_ignore_slashes_state,
            authority_state,
            file_host_state,
            host_state,
            hostname_state,
            port_state,
            relative_path_start_state,
            relative_path_state,
            query_state,
            fragment_state,
        };

        typedef Basic::Ref<Uri> Ref;

        UnicodeString::Ref scheme; // REF
        UnicodeString::Ref scheme_data; // REF
        UnicodeString::Ref username; // REF
        UnicodeString::Ref password; // REF
        UnicodeString::Ref host; // REF
        UnicodeString::Ref port; // REF
        Path path;
        UnicodeString::Ref query; // REF
        UnicodeString::Ref fragment; // REF
        bool relative_flag;

        void Initialize();
        void Initialize(UnicodeString::Ref input);

        template <int Count>
        void Initialize(const char (&input)[Count])
        {
            UnicodeString::Ref unicode_input;
            unicode_input.Initialize(input);

            Initialize(unicode_input);
        }

        bool Parse(UnicodeString::Ref input, Uri::Ref base);
        bool Parse(UnicodeString::Ref input, Uri::Ref base, UnicodeString::Ref encoding_override, State state_override, bool state_override_given);
        void SerializeTo(IStream<Codepoint>* output, bool exclude_fragment, bool path_only);
        bool is_secure_scheme();
        bool is_http_scheme();
        uint16 get_port();
    };
}