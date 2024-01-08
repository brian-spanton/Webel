// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    // $$ split into a purely rendered representation and a stream based [de]serializer?
    class Uri
    {
    private:
        void parse_error(Codepoint codepoint);

    public:
        static bool is_ascii_alpha(Codepoint codepoint);
        static bool is_ascii_alphanumeric(Codepoint codepoint);
        static bool is_ascii_digit(Codepoint codepoint);
        static bool is_ascii_hex_digit(Codepoint codepoint);
        static bool is_url_codepoint(Codepoint codepoint);
        static bool is_relative_scheme(UnicodeStringRef scheme);
        static bool is_secure_scheme(UnicodeStringRef scheme);
        static bool is_http_scheme(UnicodeStringRef scheme);
        static void percent_encode(byte b, IStream<Codepoint>* result);
        static void percent_decode(UnicodeString* string, IStream<byte>* bytes);
        static void utf_8_percent_encode(Codepoint codepoint, const bool (&anti_set)[0x100], std::shared_ptr<IStream<Codepoint> > result);
        static bool host_parse(UnicodeStringRef input, std::shared_ptr<IStream<Codepoint> > result);

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

        UnicodeStringRef scheme;
        UnicodeStringRef scheme_data;
        UnicodeStringRef username;
        UnicodeStringRef password;
        UnicodeStringRef host;
        UnicodeStringRef port;
        Path path;
        UnicodeStringRef query;
        UnicodeStringRef fragment;
        bool relative_flag = false;

        void Initialize();
        void Initialize(UnicodeString* input);

        template <int Count>
        void Initialize(const char (&input)[Count])
        {
            UnicodeStringRef unicode_input;
            initialize_unicode(&unicode_input, input);

            Initialize(unicode_input.get());
        }

        bool Parse(UnicodeString* input, Uri* base);
        bool Parse(UnicodeString* input, Uri* base, UnicodeStringRef encoding_override, State state_override, bool state_override_given);
        void write_to_stream(IStream<Codepoint>* output, bool exclude_fragment, bool path_only);
        bool is_secure_scheme();
        bool is_http_scheme();
        uint16 get_port();
    };
}