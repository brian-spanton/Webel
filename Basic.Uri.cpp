// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Uri.h"
#include "Basic.Globals.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.Utf8Decoder.h"

namespace Basic
{
    void Uri::Initialize()
    {
        this->scheme = std::make_shared<UnicodeString>();
        this->scheme->reserve(0x10);

        this->scheme_data = std::make_shared<UnicodeString>();
        this->username = std::make_shared<UnicodeString>();
        this->port = std::make_shared<UnicodeString>();
        this->relative_flag = false;
     }

    void Uri::Initialize(UnicodeString* input)
    {
        bool success = Parse(input, 0, UnicodeStringRef(), State::scheme_start_state, false);
        if (!success)
            throw FatalError("Basic", "Uri", "Initialize", "!Parse");
    }

    void Uri::parse_error(Codepoint codepoint)
    {
        char message[0x100];
        int result = sprintf_s(message, "codepoint=%04X", codepoint);
        if (result == -1)
            throw FatalError("Basic", "Uri", "parse_error", "sprintf_s", result);

        Basic::LogDebug("Basic", "Uri", "parse_error", message);
    }

    bool Uri::Parse(UnicodeString* input, Uri* base)
    {
        return Parse(input, base, UnicodeStringRef(), State::scheme_start_state, false);
    }

    bool Uri::Parse(UnicodeString* input, Uri* base, UnicodeStringRef encoding_override, State state_override, bool state_override_is_given)
    {
        // based on http://url.spec.whatwg.org/#parsing

        State state;
        if (state_override_is_given)
            state = state_override;
        else
            state = State::scheme_start_state;

        if (!encoding_override)
            encoding_override = Basic::globals->utf_8_label;

        UnicodeStringRef buffer = std::make_shared<UnicodeString>();
        buffer->reserve(0x40);

        bool at_flag = false;
        bool brackets_flag = false;

        for (uint32 pointer = 0; pointer <= (int)input->size(); pointer++)
        {
            Codepoint codepoint = EOF;
            uint32 remaining_size = 0;
            Codepoint remaining_0 = 0xfffffffe;
            Codepoint remaining_1 = 0xfffffffe;
            
            if (pointer < input->size())
            {
                codepoint = input->at(pointer);

                uint32 remaining_size = input->size() - pointer - 1;

                if (pointer + 1 < input->size())
                    remaining_0 = input->at(pointer + 1);

                if (pointer + 2 < input->size())
                    remaining_1 = input->at(pointer + 2);
            }

            switch (state)
            {
            case State::scheme_start_state:
                {
                    if (is_ascii_alpha(codepoint))
                    {
                        buffer->push_back(lower_case(codepoint));
                        state = State::scheme_state;
                    }
                    else if (!state_override_is_given)
                    {
                        state = State::no_scheme_state;
                        pointer -= 1;
                    }
                    else
                    {
                        parse_error(codepoint);
                        return true;
                    }
                }
                break;

            case State::scheme_state:
                {
                    if (is_ascii_alphanumeric(codepoint) || codepoint == '+' || codepoint == '-' || codepoint == '.')
                    {
                        buffer->push_back(lower_case(codepoint));
                    }
                    else if (codepoint == ':')
                    {
                        this->scheme = buffer;
                        buffer = std::make_shared<UnicodeString>();
                        buffer->reserve(0x40);

                        if (state_override_is_given)
                            return true;

                        if (is_relative_scheme(this->scheme))
                            this->relative_flag = true;

                        if (equals<UnicodeString, true>(this->scheme.get(), Basic::globals->file_scheme.get()))
                        {
                            state = State::relative_state;
                        }
                        else if (this->relative_flag
                            && base != 0
                            && equals<UnicodeString, true>(base->scheme.get(), this->scheme.get()))
                        {
                            state = State::relative_or_authority_state;
                        }
                        else if (this->relative_flag)
                        {
                            state = State::authority_first_slash_state;
                        }
                        else
                        {
                            state = State::scheme_data_state;
                        }
                    }
                    else if (!state_override_is_given)
                    {
                        buffer = std::make_shared<UnicodeString>();
                        buffer->reserve(0x40);

                        state = State::no_scheme_state;
                        pointer = -1;
                    }
                    else if (codepoint == EOF)
                    {
                        return true;
                    }
                    else
                    {
                        parse_error(codepoint);
                        return true;
                    }
                }
                break;

            case State::scheme_data_state:
                {
                    if (codepoint == '?')
                    {
                        this->query = std::make_shared<UnicodeString>();
                        this->query->reserve(0x100);

                        state = State::query_state;
                    }
                    else if (codepoint == '#')
                    {
                        this->fragment = std::make_shared<UnicodeString>();
                        this->fragment->reserve(0x40);
                        state = State::fragment_state;
                    }
                    else
                    {
                        if (codepoint != EOF && is_url_codepoint(codepoint) == false && codepoint != '%')
                        {
                            parse_error(codepoint);
                        }
                        else if (codepoint == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
                        {
                            parse_error(codepoint);
                        }
                        else if (!(codepoint == EOF || codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D))
                        {
                            utf_8_percent_encode(codepoint, Basic::globals->simple_encode_anti_set, this->scheme_data);
                        }
                    }
                }
                break;

            case State::no_scheme_state:
                {
                    if (base == 0 || !is_relative_scheme(base->scheme))
                    {
                        parse_error(codepoint);
                        return false;
                    }
                    else
                    {
                        state = State::relative_state;
                        pointer -= 1;
                    }
                }
                break;

            case State::relative_or_authority_state:
                {
                    if (codepoint == '/' && remaining_0 == '/')
                    {
                        state = authority_ignore_slashes_state;
                        pointer += 1;
                    }
                    else
                    {
                        parse_error(codepoint);
                        state = State::relative_state;
                        pointer -= 1;
                    }
                }
                break;

            case State::relative_state:
                {
                    this->relative_flag = true;

                    if (!equals<UnicodeString, true>(this->scheme.get(), Basic::globals->file_scheme.get()))
                        this->scheme = base->scheme;

                    switch(codepoint)
                    {
                    case EOF:
                        this->host = base->host;
                        this->port = base->port;
                        this->path = base->path;
                        this->query = base->query;
                        break;

                    case '/':
                        state = State::relative_slash_state;
                        break;

                    case '\\':
                        parse_error(codepoint);
                        state = State::relative_slash_state;
                        break;

                    case '?':
                        this->host = base->host;
                        this->port = base->port;
                        this->path = base->path;
                        this->query = std::make_shared<UnicodeString>();
                        state = State::query_state;
                        break;

                    case '#':
                        this->host = base->host;
                        this->port = base->port;
                        this->path = base->path;
                        this->query = base->query;
                        this->fragment = std::make_shared<UnicodeString>();
                        state = State::fragment_state;
                        break;

                    default:
                        {
                            if (equals<UnicodeString, true>(this->scheme.get(), Basic::globals->file_scheme.get()) == false || !is_ascii_alpha(codepoint)
                                || !(remaining_0 == ':' || remaining_0 == '|') || remaining_size != 1
                                || !(remaining_1 == '/' || remaining_1 == '\\' || remaining_1 == '?' || remaining_1 == '#'))
                            {
                                // 1. If url's scheme is not "file", or codepoint is not an ASCII alpha, or remaining does not start with
                                //    either ":" or "|", or remaining does not consist of one code point, or remaining's second code
                                //    point is not one of "/", "\", "?", and "#", then set url's host to base's host, url's port to
                                //    base's port, url's path to base's path, and then pop url's path.  
                                //
                                //        Note: This is a (platform-independent) Windows drive letter quirk. When found at the start
                                //        of a file URL it is treated as an absolute path rather than one relative to base's path.
                                this->host = base->host;
                                this->port = base->port;
                                this->path = base->path;
                                this->path.pop_back();
                            }

                            state = State::relative_path_state;
                            pointer -= 1;
                        }
                        break;
                    }
                }
                break;

            case State::relative_slash_state:
                {
                    if (codepoint == '/' || codepoint == '\\')
                    {
                        if (codepoint == '\\')
                            parse_error(codepoint);

                        if (equals<UnicodeString, true>(this->scheme.get(), Basic::globals->file_scheme.get()))
                            state = State::file_host_state;
                        else
                            state = State::authority_ignore_slashes_state;
                    }
                    else
                    {
                        if (!equals<UnicodeString, true>(this->scheme.get(), Basic::globals->file_scheme.get()))
                        {
                            this->host = base->host;
                            this->port = base->port;
                        }

                        state = State::relative_path_state;
                        pointer -= 1;
                    }
                }
                break;

            case State::authority_first_slash_state:
                {
                    if (codepoint == '/')
                    {
                        state = State::authority_second_slash_state;
                    }
                    else
                    {
                        parse_error(codepoint);
                        state = State::authority_ignore_slashes_state;
                        pointer -= 1;
                    }
                }
                break;

            case State::authority_second_slash_state:
                {
                    if (codepoint == '/')
                    {
                        state = State::authority_ignore_slashes_state;
                    }
                    else
                    {
                        parse_error(codepoint);
                        state = State::authority_ignore_slashes_state;
                        pointer -= 1;
                    }
                }
                break;

            case State::authority_ignore_slashes_state:
                {
                    if (codepoint != '/' && codepoint != '\\')
                    {
                        state = State::authority_state;
                        pointer -= 1;
                    }
                    else
                    {
                        parse_error(codepoint);
                    }
                }
                break;

            case State::authority_state:
                {
                    if (codepoint == '@')
                    {
                        if (at_flag)
                        {
                            parse_error(codepoint);
                            buffer->insert(buffer->begin(), Basic::globals->percent_forty->begin(), Basic::globals->percent_forty->end());
                        }

                        at_flag = true;

                        for (uint32 buffer_index = 0; buffer_index < buffer->size(); buffer_index++)
                        {
                            Codepoint codepoint = buffer->at(buffer_index);

                            if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                            {
                                parse_error(codepoint);
                                continue;
                            }

                            if (!is_url_codepoint(codepoint) && codepoint != '%')
                            {
                                parse_error(codepoint);
                            }

                            if (codepoint == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
                            {
                                parse_error(codepoint);
                            }

                            if (codepoint == ':' && !this->password)
                            {
                                this->password = std::make_shared<UnicodeString>();
                                continue;
                            }

                            if (this->password)
                            {
                                utf_8_percent_encode(codepoint, Basic::globals->default_encode_anti_set, this->password);
                            }
                            else
                            {
                                utf_8_percent_encode(codepoint, Basic::globals->default_encode_anti_set, this->username);
                            }
                        }

                        buffer = std::make_shared<UnicodeString>();
                    }
                    else if (codepoint == EOF || codepoint == '/' || codepoint == '\\' || codepoint == '?' || codepoint == '#')
                    {
                        pointer -= (buffer->size() + 1);
                        buffer = std::make_shared<UnicodeString>();
                        state = State::host_state;
                    }
                    else
                    {
                        buffer->push_back(codepoint);
                    }
                }
                break;

            case State::file_host_state:
                {
                    if (codepoint == EOF || codepoint == '/' || codepoint == '\\' || codepoint == '?' || codepoint == '#')
                    {
                        pointer -= 1;

                        if (buffer->size() == 2 && is_ascii_alpha(buffer->at(0))
                            && (buffer->at(1) == ':' || buffer->at(1) == '|'))
                        {
                            state = State::relative_path_state;
                        }
                        else if (buffer->size() == 0)
                        {
                            state = State::relative_path_start_state;
                        }
                        else
                        {
                            UnicodeStringRef host = std::make_shared<UnicodeString>();
                            bool success = host_parse(buffer, host);
                            if (!success)
                                return false;

                            this->host = host;
                            buffer = std::make_shared<UnicodeString>();
                            state = State::relative_path_start_state;
                        }
                    }
                    else if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                    {
                        parse_error(codepoint);
                    }
                    else
                    {
                        buffer->push_back(codepoint);
                    }
                }
                break;

            case State::host_state:
            case State::hostname_state:
                {
                    if (codepoint == ':' && brackets_flag == false)
                    {
                        UnicodeStringRef host = std::make_shared<UnicodeString>();
                        bool success = host_parse(buffer, host);
                        if (!success)
                            return false;

                        this->host = host;
                        buffer = std::make_shared<UnicodeString>();
                        state = State::port_state;

                        if (state_override_is_given && state_override == State::hostname_state)
                            return true;
                    }
                    else if (codepoint == EOF || codepoint == '\\' || codepoint == '/' || codepoint == '?' || codepoint == '#')
                    {
                        pointer -= 1;

                        UnicodeStringRef host = std::make_shared<UnicodeString>();
                        bool success = host_parse(buffer, host);
                        if (!success)
                            return false;

                        this->host = host;
                        buffer = std::make_shared<UnicodeString>();
                        state = State::relative_path_start_state;

                        if (state_override_is_given)
                            return true;
                    }
                    else if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                    {
                        parse_error(codepoint);
                    }
                    else
                    {
                        if (codepoint == '[')
                            brackets_flag = true;
                        else if (codepoint == ']')
                            brackets_flag = false;

                        buffer->push_back(codepoint);
                    }
                }
                break;

            case State::port_state:
                {
                    if (is_ascii_digit(codepoint))
                    {
                        buffer->push_back(codepoint);
                    }
                    else if (codepoint == EOF || codepoint == '/' || codepoint == '\\' || codepoint == '?' || codepoint == '#' || state_override_is_given)
                    {
                        while (buffer->size() > 1 && buffer->at(0) == 0x0030)
                            buffer->erase(0);

                        Basic::Globals::PortMap::iterator mapping = Basic::globals->scheme_to_port_map.find(this->scheme);
                        if (mapping != Basic::globals->scheme_to_port_map.end())
                        {
                            if (equals<UnicodeString, true>(buffer.get(), mapping->second.get()))
                                buffer = std::make_shared<UnicodeString>();
                        }

                        this->port = buffer;

                        if (state_override_is_given)
                            return true;

                        buffer = std::make_shared<UnicodeString>();
                        state = State::relative_path_start_state;
                        pointer -= 1;
                    }
                    else if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                    {
                        parse_error(codepoint);
                    }
                    else
                    {
                        parse_error(codepoint);
                        return false;
                    }
                }
                break;

            case State::relative_path_start_state:
                {
                    if (codepoint == '\\')
                        parse_error(codepoint);

                    state = State::relative_path_state;

                    if (codepoint != '/' && codepoint != '\\')
                        pointer -= 1;
                }
                break;

            case State::relative_path_state:
                {
                    if (codepoint == EOF || codepoint == '/' || codepoint == '\\'
                        || (state_override_is_given == false && (codepoint == '?' || codepoint == '#')))
                    {
                        if (codepoint == '\\')
                            parse_error(codepoint);

                        if (equals<UnicodeString, false>(buffer.get(), Basic::globals->percent_two_e.get()))
                            buffer = Basic::globals->dot;
                        else if (equals<UnicodeString, false>(buffer.get(), Basic::globals->dot_percent_two_e.get()))
                            buffer = Basic::globals->dot_dot;
                        else if (equals<UnicodeString, false>(buffer.get(), Basic::globals->percent_two_e_dot.get()))
                            buffer = Basic::globals->dot_dot;
                        else if (equals<UnicodeString, false>(buffer.get(), Basic::globals->dot_dot.get()))
                            buffer = Basic::globals->dot_dot;

                        if (equals<UnicodeString, true>(buffer.get(), Basic::globals->dot_dot.get()))
                        {
                            if (this->path.size() > 0)
                                this->path.pop_back();

                            if (codepoint != '/' && codepoint != '\\')
                                this->path.push_back(std::make_shared<UnicodeString>());
                        }
                        else if (equals<UnicodeString, true>(buffer.get(), Basic::globals->dot.get()) && codepoint != '/' && codepoint != '\\')
                        {
                            this->path.push_back(std::make_shared<UnicodeString>());
                        }
                        else if (!equals<UnicodeString, true>(buffer.get(), Basic::globals->dot.get()))
                        {
                            if (equals<UnicodeString, true>(this->scheme.get(), Basic::globals->file_scheme.get())
                                && this->path.size() == 0
                                && buffer->size() == 2 && is_ascii_alpha(buffer->at(0)) && buffer->at(1) == '|')
                            {
                                Codepoint colon = ':';
                                buffer->replace(1, 1, &colon, 1);
                            }

                            this->path.push_back(buffer);
                        }

                        buffer = std::make_shared<UnicodeString>();

                        if (codepoint == '?')
                        {
                            this->query = std::make_shared<UnicodeString>();
                            state = State::query_state;
                        }

                        if (codepoint == '#')
                        {
                            this->fragment = std::make_shared<UnicodeString>();
                            state = State::fragment_state;
                        }
                    }
                    else if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                    {
                        parse_error(codepoint);
                    }
                    else
                    {
                        if (!is_url_codepoint(codepoint) && codepoint != '%')
                            parse_error(codepoint);

                        if (codepoint == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
                            parse_error(codepoint);

                        utf_8_percent_encode(codepoint, Basic::globals->default_encode_anti_set, buffer);
                    }
                }
                break;

            case State::query_state:
                {
                    if (codepoint == EOF || (state_override_is_given == false && codepoint == '#'))
                    {
                        if (this->relative_flag == true)
                            encoding_override = Basic::globals->utf_8_label;

                        // 2. Set buffer to the result of running encoding override's encoder on buffer. Whenever the encoder
                        //    algorithm emits an encoder error, emit a 0x3F byte instead and do not terminate the algorithm. 
                        std::shared_ptr<ByteString> buffer_bytes = std::make_shared<ByteString>();

                        std::shared_ptr<IEncoder> encoder;
                        Basic::globals->GetEncoder(encoding_override, &encoder);

                        encoder->set_destination(buffer_bytes.get());
                        encoder->set_error_replacement_byte(0x3F);
                        encoder->write_elements(buffer->address(), buffer->size());

                        // 3. For each byte in buffer run these subsubsteps: 
                        for (uint32 i = 0; i < buffer_bytes->size(); i++)
                        {
                            byte b = buffer_bytes->at(i);

                            // 1. If byte is less than 0x21, greater than 0x7E, or is one of 0x22, 0x23, 0x3C, 0x3E, and 0x60,
                            //    append byte, percent encoded, to url's query. 
                            if (b < 0x21 || b > 0x7E || b == 0x22 || b == 0x23 || b == 0x3C || b == 0x3E || b == 0x60)
                            {
                                percent_encode(b, this->query.get());
                            }

                            // 2. Otherwise, append a code point whose value is byte to url's query. 
                            else
                            {
                                this->query->push_back(b);
                            }
                        }

                        // 4. Set buffer to the empty string. 
                        buffer = std::make_shared<UnicodeString>();

                        // 5. If codepoint is "#", set url's fragment to the empty string, and state to fragment state. 
                        if (codepoint == '#')
                        {
                            this->fragment = std::make_shared<UnicodeString>();
                            state = State::fragment_state;
                        }
                    }
                    else if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                    {
                        parse_error(codepoint);
                    }
                    else
                    {
                        if (!is_url_codepoint(codepoint) && codepoint != '%')
                            parse_error(codepoint);

                        if (codepoint == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
                            parse_error(codepoint);

                        buffer->push_back(codepoint);
                    }
                }
                break;

            case State::fragment_state:
                {
                    if (codepoint == EOF)
                    {
                        // Do nothing.
                    }
                    else if (codepoint == 0x0009 || codepoint == 0x000A || codepoint == 0x000D)
                    {
                        parse_error(codepoint);
                    }
                    else
                    {
                        if (!is_url_codepoint(codepoint) && codepoint != '%')
                            parse_error(codepoint);

                        if (codepoint == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
                            parse_error(codepoint);

                        utf_8_percent_encode(codepoint, Basic::globals->simple_encode_anti_set, this->fragment);
                    }
                }
                break;

            default:
                throw FatalError("Basic", "Uri", "Parse", "unhandled state", state);
            }
        }

        return true;
    }

    bool Uri::is_http_scheme()
    {
        return is_http_scheme(this->scheme);
    }

    bool Uri::is_secure_scheme()
    {
        return is_secure_scheme(this->scheme);
    }

    bool Uri::is_ascii_alpha(Codepoint codepoint)
    {
        if (codepoint >= 0x0041 && codepoint <= 0x005A)
            return true;

        if (codepoint >= 0x0061 && codepoint <= 0x007A)
            return true;

        return false;
    }

    bool Uri::is_ascii_alphanumeric(Codepoint codepoint)
    {
        if (is_ascii_alpha(codepoint))
            return true;

        if (is_ascii_digit(codepoint))
            return true;

        return false;
    }

    bool Uri::is_ascii_digit(Codepoint codepoint)
    {
        if (codepoint >= 0x0030 && codepoint <= 0x0039)
            return true;

        return false;
    }

    bool Uri::is_ascii_hex_digit(Codepoint codepoint)
    {
        if (is_ascii_digit(codepoint))
            return true;

        if (codepoint >= 0x0041 && codepoint <= 0x0046)
            return true;

        if (codepoint >= 0x0061 && codepoint <= 0x0066)
            return true;

        return false;
    }

    bool Uri::is_url_codepoint(Codepoint codepoint)
    {
        if (is_ascii_alphanumeric(codepoint))
            return true;

        if (codepoint == '!' || codepoint == '$' || codepoint == '&' || codepoint == '\'' || codepoint == '(' || codepoint == ')' || codepoint == '*' || codepoint == '+' || codepoint == ','
            || codepoint == '-' || codepoint == '.' || codepoint == '/' || codepoint == ':' || codepoint == ';' || codepoint == '=' || codepoint == '?' || codepoint == '@' || codepoint == '_' || codepoint == '~')
            return true;
        
        if (codepoint >= 0x00A0 && codepoint <= 0xD7FF)
            return true;
        
        if (codepoint >= 0xE000 && codepoint <= 0xFDCF)
            return true;
        
        if (codepoint >= 0xFDF0 && codepoint <= 0xFFEF)
            return true;
        
        if (codepoint >= 0x10000 && codepoint <= 0x1FFFD)
            return true;
        
        if (codepoint >= 0x20000 && codepoint <= 0x2FFFD)
            return true;
        
        if (codepoint >= 0x30000 && codepoint <= 0x3FFFD)
            return true;
        
        if (codepoint >= 0x40000 && codepoint <= 0x4FFFD)
            return true;
        
        if (codepoint >= 0x50000 && codepoint <= 0x5FFFD)
            return true;
        
        if (codepoint >= 0x60000 && codepoint <= 0x6FFFD)
            return true;
        
        if (codepoint >= 0x70000 && codepoint <= 0x7FFFD)
            return true;
        
        if (codepoint >= 0x80000 && codepoint <= 0x8FFFD)
            return true;
        
        if (codepoint >= 0x90000 && codepoint <= 0x9FFFD)
            return true;
        
        if (codepoint >= 0xA0000 && codepoint <= 0xAFFFD)
            return true;
        
        if (codepoint >= 0xB0000 && codepoint <= 0xBFFFD)
            return true;
        
        if (codepoint >= 0xC0000 && codepoint <= 0xCFFFD)
            return true;
        
        if (codepoint >= 0xD0000 && codepoint <= 0xDFFFD)
            return true;
        
        if (codepoint >= 0xE1000 && codepoint <= 0xEFFFD)
            return true;
        
        if (codepoint >= 0xF0000 && codepoint <= 0xFFFFD)
            return true;
        
        if (codepoint >= 0x100000 && codepoint <= 0x10FFFD)
            return true;
        
        return false;
    }

    bool Uri::is_http_scheme(UnicodeStringRef scheme)
    {
        if (equals<UnicodeString, false>(scheme.get(), Basic::globals->http_scheme.get()))
            return true;
        
        if (equals<UnicodeString, false>(scheme.get(), Basic::globals->https_scheme.get()))
            return true;

        return false;
    }

    uint16 Uri::get_port()
    {
        if (is_null_or_empty(this->port.get()) == false)
        {
            return this->port->as_base_10<uint16>(0);
        }

        Basic::Globals::PortMap::iterator mapping = Basic::globals->scheme_to_port_map.find(this->scheme);
        if (mapping != Basic::globals->scheme_to_port_map.end())
        {
            return mapping->second->as_base_10<uint16>(0);
        }

        return 0;
    }

    bool Uri::is_relative_scheme(UnicodeStringRef scheme)
    {
        Basic::Globals::PortMap::iterator mapping = Basic::globals->scheme_to_port_map.find(scheme);
        if (mapping != Basic::globals->scheme_to_port_map.end())
            return true;

        return false;
    }

    bool Uri::is_secure_scheme(UnicodeStringRef scheme)
    {
        if (equals<UnicodeString, false>(scheme.get(), Basic::globals->https_scheme.get()))
            return true;

        return false;
    }

    void Uri::percent_encode(byte b, IStream<Codepoint>* result)
    {
        TextWriter writer(result);
        writer.WriteFormat<3>("%%%02X", b);
    }

    void Uri::percent_decode(UnicodeString* string, IStream<byte>* bytes)
    {
        for (uint32 pointer = 0; pointer != string->size(); pointer++)
        {
            Codepoint codepoint = string->at(pointer);

            if (codepoint == '%' && (string->size() - pointer) >= 3)
            {
                UnicodeString remaining;
                remaining.insert(remaining.end(), string->begin() + pointer + 1, string->begin() + pointer + 3);

                bool all_digits;
                byte decoded = remaining.as_base_16<byte>(&all_digits);

                if (all_digits)
                {
                    bytes->write_element(decoded);
                    pointer += 2;
                    continue;
                }
            }

            bytes->write_element((byte)codepoint);
        }
    }

    void Uri::utf_8_percent_encode(Codepoint codepoint, const bool (&anti_set)[0x100], std::shared_ptr<IStream<Codepoint> > result)
    {
        if (codepoint < _countof(anti_set) && anti_set[codepoint])
        {
            result->write_element(codepoint);
            return;
        }

        // 2. Let bytes be the result of running utf-8 encode on code point. 
        std::shared_ptr<ByteString> bytes = std::make_shared<ByteString>();

        Utf8Encoder encoder;
        encoder.set_destination(bytes.get());
        encoder.write_element(codepoint);

        // 3. Percent encode each byte in bytes, and then return them concatenated, in the same order.
        for (uint32 i = 0; i < bytes->size(); i++)
        {
            percent_encode(bytes->at(i), result.get());
        }
    }

    bool Uri::host_parse(UnicodeStringRef input, std::shared_ptr<IStream<Codepoint> > result)
    {
        if (input->size() == 0)
            return false;

        ByteString percent_decoded;
        percent_decode(input.get(), &percent_decoded);

        Utf8Decoder decoder;
        decoder.set_destination(result.get());
        decoder.write_elements(percent_decoded.address(), percent_decoded.size());

        return true;
    }

    void Uri::write_to_stream(IStream<Codepoint>* output, bool exclude_fragment, bool path_only)
    {
        // based on http://url.spec.whatwg.org/#writing

        if (!path_only)
        {
            this->scheme->write_to_stream(output);

            output->write_element(':');
        }
        
        if (this->relative_flag)
        {
            if (!path_only)
            {
                output->write_element('/');
                output->write_element('/');

                if (!is_null_or_empty(this->username.get()) || this->password)
                {
                    this->username->write_to_stream(output);

                    if (this->password)
                    {
                        output->write_element(':');

                        this->password->write_to_stream(output);
                    }

                    output->write_element('@');
                }

                if (!is_null_or_empty(this->host.get()))
                {
                    this->host->write_to_stream(output);
                }

                if (!is_null_or_empty(this->port.get()))
                {
                    output->write_element(':');

                    this->port->write_to_stream(output);
                }
            }

            output->write_element('/');

            for (uint32 i = 0; i < this->path.size(); i++)
            {
                if (i > 0)
                {
                    output->write_element('/');
                }

                this->path.at(i)->write_to_stream(output);
            }
        }
        else
        {
            this->scheme_data->write_to_stream(output);
        }

        if (!is_null_or_empty(this->query.get()))
        {
            output->write_element('?');

            this->query->write_to_stream(output);
        }

        if (exclude_fragment == false && !is_null_or_empty(this->fragment.get()))
        {
            output->write_element('#');

            this->fragment->write_to_stream(output);
        }
    }
}