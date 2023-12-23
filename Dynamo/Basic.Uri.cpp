#include "stdafx.h"
#include "Basic.Uri.h"
#include "Basic.Globals.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.Utf8Decoder.h"

namespace Basic
{
	void Uri::Initialize()
	{
		this->scheme = New<UnicodeString>();
		this->scheme_data = New<UnicodeString>();
		this->username = New<UnicodeString>();
		this->port = New<UnicodeString>();
		this->relative_flag = false;
 	}

	void Uri::Initialize(UnicodeString::Ref input)
	{
		bool success = Parse(input, (Uri*)0, (UnicodeString*)0, State::scheme_start_state, false);
		if (!success)
			throw new Exception("Uri::Initialize Parse failed");
	}

	void Uri::parse_error(Codepoint c)
	{
		HandleError("Uri::parse_error");
	}

	bool Uri::Parse(UnicodeString::Ref input, Uri::Ref base)
	{
		return Parse(input, base, (UnicodeString*)0, State::scheme_start_state, false);
	}

	bool Uri::Parse(UnicodeString::Ref input, Uri::Ref base, UnicodeString::Ref encoding_override, State state_override, bool state_override_is_given)
	{
		State state;
		if (state_override_is_given)
			state = state_override;
		else
			state = State::scheme_start_state;

		if (encoding_override.item() == 0)
			encoding_override = Basic::globals->utf_8_label;

		UnicodeString::Ref buffer = New<UnicodeString>();
		bool at_flag = false;
		bool brackets_flag = false;

		for (uint32 pointer = 0; pointer <= (int)input->size(); pointer++)
		{
			Codepoint c = EOF;
			uint32 remaining_size = 0;
			Codepoint remaining_0 = 0xfffffffe;
			Codepoint remaining_1 = 0xfffffffe;
			
			if (pointer < input->size())
			{
				c = input->at(pointer);

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
					if (is_ascii_alpha(c))
					{
						buffer->push_back(lower_case(c));
						state = State::scheme_state;
					}
					else if (!state_override_is_given)
					{
						state = State::no_scheme_state;
						pointer -= 1;
					}
					else
					{
						parse_error(c);
						return true;
					}
				}
				break;

			case State::scheme_state:
				{
					if (is_ascii_alphanumeric(c) || c == '+' || c == '-' || c == '.')
					{
						buffer->push_back(lower_case(c));
					}
					else if (c == ':')
					{
						this->scheme = buffer;
						buffer = New<UnicodeString>();

						if (state_override_is_given)
							return true;

						if (is_relative_scheme(this->scheme))
							this->relative_flag = true;

						if (this->scheme.equals<true>(Basic::globals->file_scheme))
						{
							state = State::relative_state;
						}
						else if (this->relative_flag
							&& base.item() != 0
							&& base->scheme.equals<true>(this->scheme))
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
						buffer = New<UnicodeString>();
						state = State::no_scheme_state;
						pointer = -1;
					}
					else if (c == EOF)
					{
						return true;
					}
					else
					{
						parse_error(c);
						return true;
					}
				}
				break;

			case State::scheme_data_state:
				{
					if (c == '?')
					{
						this->query = New<UnicodeString>();
						state = State::query_state;
					}
					else if (c == '#')
					{
						this->fragment = New<UnicodeString>();
						state = State::fragment_state;
					}
					else
					{
						if (c != EOF && is_url_codepoint(c) == false && c != '%')
						{
							parse_error(c);
						}
						else if (c == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
						{
							parse_error(c);
						}
						else if (!(c == EOF || c == 0x0009 || c == 0x000A || 0x000D))
						{
							utf_8_percent_encode(c, Basic::globals->simple_encode_anti_set, this->scheme_data);
						}
					}
				}
				break;

			case State::no_scheme_state:
				{
					if (base.item() == 0 || !is_relative_scheme(base->scheme))
					{
						parse_error(c);
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
					if (c == '/' && remaining_0 == '/')
					{
						state = authority_ignore_slashes_state;
						pointer += 1;
					}
					else
					{
						parse_error(c);
						state = State::relative_state;
						pointer -= 1;
					}
				}
				break;

			case State::relative_state:
				{
					this->relative_flag = true;

					if (!this->scheme.equals<true>(Basic::globals->file_scheme))
						this->scheme = base->scheme;

					switch(c)
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
						parse_error(c);
						state = State::relative_slash_state;
						break;

					case '?':
						this->host = base->host;
						this->port = base->port;
						this->path = base->path;
						this->query = New<UnicodeString>();
						state = State::query_state;
						break;

					case '#':
						this->host = base->host;
						this->port = base->port;
						this->path = base->path;
						this->query = base->query;
						this->fragment = New<UnicodeString>();
						state = State::fragment_state;
						break;

					default:
						{
							if (this->scheme.equals<true>(Basic::globals->file_scheme) == false || !is_ascii_alpha(c)
								|| !(remaining_0 == ':' || remaining_0 == '|') || remaining_size != 1
								|| !(remaining_1 == '/' || remaining_1 == '\\' || remaining_1 == '?' || remaining_1 == '#'))
							{
								// 1. If url's scheme is not "file", or c is not an ASCII alpha, or remaining does not start with
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
					if (c == '/' || c == '\\')
					{
						if (c == '\\')
							parse_error(c);

						if (this->scheme.equals<true>(Basic::globals->file_scheme))
							state = State::file_host_state;
						else
							state = State::authority_ignore_slashes_state;
					}
					else
					{
						if (!this->scheme.equals<true>(Basic::globals->file_scheme))
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
					if (c == '/')
					{
						state = State::authority_second_slash_state;
					}
					else
					{
						parse_error(c);
						state = State::authority_ignore_slashes_state;
						pointer -= 1;
					}
				}
				break;

			case State::authority_second_slash_state:
				{
					if (c == '/')
					{
						state = State::authority_ignore_slashes_state;
					}
					else
					{
						parse_error(c);
						state = State::authority_ignore_slashes_state;
						pointer -= 1;
					}
				}
				break;

			case State::authority_ignore_slashes_state:
				{
					if (c != '/' && c != '\\')
					{
						state = State::authority_state;
						pointer -= 1;
					}
					else
					{
						parse_error(c);
					}
				}
				break;

			case State::authority_state:
				{
					if (c == '@')
					{
						if (at_flag)
						{
							parse_error(c);
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

							if (codepoint == ':' && this->password.item() == 0)
							{
								this->password = New<UnicodeString>();
								continue;
							}

							if (this->password.item() != 0)
							{
								utf_8_percent_encode(codepoint, Basic::globals->default_encode_anti_set, this->password);
							}
							else
							{
								utf_8_percent_encode(codepoint, Basic::globals->default_encode_anti_set, this->username);
							}
						}

						buffer = New<UnicodeString>();
					}
					else if (c == EOF || c == '/' || c == '\\' || c == '?' || c == '#')
					{
						pointer -= (buffer->size() + 1);
						buffer = New<UnicodeString>();
						state = State::host_state;
					}
					else
					{
						buffer->push_back(c);
					}
				}
				break;

			case State::file_host_state:
				{
					if (c == EOF || c == '/' || c == '\\' || c == '?' || c == '#')
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
							UnicodeString::Ref host = New<UnicodeString>();
							bool success = host_parse(buffer, host);
							if (!success)
								return false;

							this->host = host;
							buffer = New<UnicodeString>();
							state = State::relative_path_start_state;
						}
					}
					else if (c == 0x0009 || c == 0x000A || c == 0x000D)
					{
						parse_error(c);
					}
					else
					{
						buffer->push_back(c);
					}
				}
				break;

			case State::host_state:
			case State::hostname_state:
				{
					if (c == ':' && brackets_flag == false)
					{
						UnicodeString::Ref host = New<UnicodeString>();
						bool success = host_parse(buffer, host);
						if (!success)
							return false;

						this->host = host;
						buffer = New<UnicodeString>();
						state = State::port_state;

						if (state_override_is_given && state_override == State::hostname_state)
							return true;
					}
					else if (c == EOF || c == '\\' || c == '/' || c == '?' || c == '#')
					{
						pointer -= 1;

						UnicodeString::Ref host = New<UnicodeString>();
						bool success = host_parse(buffer, host);
						if (!success)
							return false;

						this->host = host;
						buffer = New<UnicodeString>();
						state = State::relative_path_start_state;

						if (state_override_is_given)
							return true;
					}
					else if (c == 0x0009 || c == 0x000A || c == 0x000D)
					{
						parse_error(c);
					}
					else
					{
						if (c == '[')
							brackets_flag = true;
						else if (c == ']')
							brackets_flag = false;

						buffer->push_back(c);
					}
				}
				break;

			case State::port_state:
				{
					if (is_ascii_digit(c))
					{
						buffer->push_back(c);
					}
					else if (c == EOF || c == '/' || c == '\\' || c == '?' || c == '#' || state_override_is_given)
					{
						while (buffer->size() > 1 && buffer->at(0) == 0x0030)
							buffer->erase(0);

						Basic::Globals::PortMap::iterator mapping = Basic::globals->scheme_to_port_map.find(this->scheme);
						if (mapping != Basic::globals->scheme_to_port_map.end())
						{
							if (buffer.equals<true>(mapping->second))
								buffer = New<UnicodeString>();
						}

						this->port = buffer;

						if (state_override_is_given)
							return true;

						buffer = New<UnicodeString>();
						state = State::relative_path_start_state;
						pointer -= 1;
					}
					else if (c == 0x0009 || c == 0x000A || c == 0x000D)
					{
						parse_error(c);
					}
					else
					{
						parse_error(c);
						return false;
					}
				}
				break;

			case State::relative_path_start_state:
				{
					if (c == '\\')
						parse_error(c);

					state = State::relative_path_state;

					if (c != '/' && c != '\\')
						pointer -= 1;
				}
				break;

			case State::relative_path_state:
				{
					if (c == EOF || c == '/' || c == '\\'
						|| (state_override_is_given == false && (c == '?' || c == '#')))
					{
						if (c == '\\')
							parse_error(c);

						if (buffer.equals<false>(Basic::globals->percent_two_e))
							buffer = Basic::globals->dot;
						else if (buffer.equals<false>(Basic::globals->dot_percent_two_e))
							buffer = Basic::globals->dot_dot;
						else if (buffer.equals<false>(Basic::globals->percent_two_e_dot))
							buffer = Basic::globals->dot_dot;
						else if (buffer.equals<false>(Basic::globals->dot_dot))
							buffer = Basic::globals->dot_dot;

						if (buffer.equals<true>(Basic::globals->dot_dot))
						{
							if (this->path.size() > 0)
								this->path.pop_back();

							if (c != '/' && c != '\\')
								this->path.push_back(New<UnicodeString>());
						}
						else if (buffer.equals<true>(Basic::globals->dot) && c != '/' && c != '\\')
						{
							this->path.push_back(New<UnicodeString>());
						}
						else if (!buffer.equals<true>(Basic::globals->dot))
						{
							if (this->scheme.equals<true>(Basic::globals->file_scheme)
								&& this->path.size() == 0
								&& buffer->size() == 2 && is_ascii_alpha(buffer->at(0)) && buffer->at(1) == '|')
							{
								Codepoint colon = ':';
								buffer->replace(1, 1, &colon, 1);
							}

							this->path.push_back(buffer);
						}

						buffer = New<UnicodeString>();

						if (c == '?')
						{
							this->query = New<UnicodeString>();
							state = State::query_state;
						}

						if (c == '#')
						{
							this->fragment = New<UnicodeString>();
							state = State::fragment_state;
						}
					}
					else if (c == 0x0009 || c == 0x000A || c == 0x000D)
					{
						parse_error(c);
					}
					else
					{
						if (!is_url_codepoint(c) && c != '%')
							parse_error(c);

						if (c == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
							parse_error(c);

						utf_8_percent_encode(c, Basic::globals->default_encode_anti_set, buffer);
					}
				}
				break;

			case State::query_state:
				{
					if (c == EOF || (state_override_is_given == false && c == '#'))
					{
						if (this->relative_flag == true)
							encoding_override = Basic::globals->utf_8_label;

						// 2. Set buffer to the result of running encoding override's encoder on buffer. Whenever the encoder
						//    algorithm emits an encoder error, emit a 0x3F byte instead and do not terminate the algorithm. 
						Inline<ByteString> buffer_bytes;

						Basic::Ref<IEncoder> encoder;
						Basic::globals->GetEncoder(encoding_override, &encoder);

						encoder->set_destination(&buffer_bytes);
						encoder->set_error_replacement_byte(0x3F);
						encoder->Write(buffer->c_str(), buffer->size());

						// 3. For each byte in buffer run these subsubsteps: 
						for (uint32 i = 0; i < buffer_bytes.size(); i++)
						{
							byte b = buffer_bytes.at(i);

							// 1. If byte is less than 0x21, greater than 0x7E, or is one of 0x22, 0x23, 0x3C, 0x3E, and 0x60,
							//    append byte, percent encoded, to url's query. 
							if (b < 0x21 || b > 0x7E || b == 0x22 || b == 0x23 || b == 0x3C || b == 0x3E || b == 0x60)
							{
								percent_encode(b, this->query);
							}

							// 2. Otherwise, append a code point whose value is byte to url's query. 
							else
							{
								this->query->push_back(b);
							}
						}

						// 4. Set buffer to the empty string. 
						buffer = New<UnicodeString>();

						// 5. If c is "#", set url's fragment to the empty string, and state to fragment state. 
						if (c == '#')
						{
							this->fragment = New<UnicodeString>();
							state = State::fragment_state;
						}
					}
					else if (c == 0x0009 || c == 0x000A || c == 0x000D)
					{
						parse_error(c);
					}
					else
					{
						if (!is_url_codepoint(c) && c != '%')
							parse_error(c);

						if (c == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
							parse_error(c);

						buffer->push_back(c);
					}
				}
				break;

			case State::fragment_state:
				{
					if (c == EOF)
					{
						// Do nothing.
					}
					else if (c == 0x0009 || c == 0x000A || c == 0x000D)
					{
						parse_error(c);
					}
					else
					{
						if (!is_url_codepoint(c) && c != '%')
							parse_error(c);

						if (c == '%' && !(is_ascii_hex_digit(remaining_0) && is_ascii_hex_digit(remaining_1)))
							parse_error(c);

						utf_8_percent_encode(c, Basic::globals->simple_encode_anti_set, this->fragment);
					}
				}
				break;

			default:
				throw new Exception("Http::UriStream::Process unexpected state");
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

	bool Uri::is_ascii_alpha(Codepoint c)
	{
		if (c >= 0x0041 && c <= 0x005A)
			return true;

		if (c >= 0x0061 && c <= 0x007A)
			return true;

		return false;
	}

	bool Uri::is_ascii_alphanumeric(Codepoint c)
	{
		if (is_ascii_alpha(c))
			return true;

		if (is_ascii_digit(c))
			return true;

		return false;
	}

	bool Uri::is_ascii_digit(Codepoint c)
	{
		if (c >= 0x0030 && c <= 0x0039)
			return true;

		return false;
	}

	bool Uri::is_ascii_hex_digit(Codepoint c)
	{
		if (is_ascii_digit(c))
			return true;

		if (c >= 0x0041 && c <= 0x0046)
			return true;

		if (c >= 0x0061 && c <= 0x0066)
			return true;

		return false;
	}

	bool Uri::is_url_codepoint(Codepoint c)
	{
		if (is_ascii_alphanumeric(c))
			return true;

		if (c == '!' || c == '$' || c == '&' || c == '\'' || c == '(' || c == ')' || c == '*' || c == '+' || c == ','
			|| c == '-' || c == '.' || c == '/' || c == ':' || c == ';' || c == '=' || c == '?' || c == '@' || c == '_' || c == '~')
			return true;
		
		if (c >= 0x00A0 && c <= 0xD7FF)
			return true;
		
		if (c >= 0xE000 && c <= 0xFDCF)
			return true;
		
		if (c >= 0xFDF0 && c <= 0xFFEF)
			return true;
		
		if (c >= 0x10000 && c <= 0x1FFFD)
			return true;
		
		if (c >= 0x20000 && c <= 0x2FFFD)
			return true;
		
		if (c >= 0x30000 && c <= 0x3FFFD)
			return true;
		
		if (c >= 0x40000 && c <= 0x4FFFD)
			return true;
		
		if (c >= 0x50000 && c <= 0x5FFFD)
			return true;
		
		if (c >= 0x60000 && c <= 0x6FFFD)
			return true;
		
		if (c >= 0x70000 && c <= 0x7FFFD)
			return true;
		
		if (c >= 0x80000 && c <= 0x8FFFD)
			return true;
		
		if (c >= 0x90000 && c <= 0x9FFFD)
			return true;
		
		if (c >= 0xA0000 && c <= 0xAFFFD)
			return true;
		
		if (c >= 0xB0000 && c <= 0xBFFFD)
			return true;
		
		if (c >= 0xC0000 && c <= 0xCFFFD)
			return true;
		
		if (c >= 0xD0000 && c <= 0xDFFFD)
			return true;
		
		if (c >= 0xE1000 && c <= 0xEFFFD)
			return true;
		
		if (c >= 0xF0000 && c <= 0xFFFFD)
			return true;
		
		if (c >= 0x100000 && c <= 0x10FFFD)
			return true;
		
		return false;
	}

	bool Uri::is_http_scheme(UnicodeString::Ref scheme)
	{
		if (scheme.equals<false>(Basic::globals->http_scheme))
			return true;
		
		if (scheme.equals<false>(Basic::globals->https_scheme))
			return true;

		return false;
	}

	uint16 Uri::get_port()
	{
		if (this->port.is_null_or_empty() == false)
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

	bool Uri::is_relative_scheme(UnicodeString::Ref scheme)
	{
		Basic::Globals::PortMap::iterator mapping = Basic::globals->scheme_to_port_map.find(scheme);
		if (mapping != Basic::globals->scheme_to_port_map.end())
			return true;

		return false;
	}

	bool Uri::is_secure_scheme(UnicodeString::Ref scheme)
	{
		if (scheme.equals<false>(Basic::globals->https_scheme))
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
			Codepoint c = string->at(pointer);

			if (c == '%' && (string->size() - pointer) >= 3)
			{
				Inline<UnicodeString> remaining;
				remaining.insert(remaining.end(), string->begin() + pointer + 1, string->begin() + pointer + 3);

				bool all_digits;
				byte decoded = remaining.as_base_16<byte>(&all_digits);

				if (all_digits)
				{
					bytes->Write(&decoded, 1);
					pointer += 2;
					continue;
				}
			}

			byte b = (byte)c;
			bytes->Write(&b, 1);
		}
	}

	void Uri::utf_8_percent_encode(Codepoint codepoint, const bool (&anti_set)[0x100], IStream<Codepoint>* result)
	{
		if (codepoint < _countof(anti_set) && anti_set[codepoint])
		{
			result->Write(&codepoint, 1);
			return;
		}

		// 2. Let bytes be the result of running utf-8 encode on code point. 
		Inline<ByteString> bytes;

		Inline<Utf8Encoder> encoder;
		encoder.set_destination(&bytes);
		encoder.Write(&codepoint, 1);

		// 3. Percent encode each byte in bytes, and then return them concatenated, in the same order.
		for (uint32 i = 0; i < bytes.size(); i++)
		{
			percent_encode(bytes.at(i), result);
		}
	}

	bool Uri::host_parse(UnicodeString::Ref input, IStream<Codepoint>* result)
	{
		if (input->size() == 0)
			return false;

		Inline<ByteString> percent_decoded;
		percent_decode(input, &percent_decoded);

		Inline<Utf8Decoder> decoder;
		decoder.set_destination(result);
		decoder.Write(percent_decoded.c_str(), percent_decoded.size());

		return true;
	}

	void Uri::SerializeTo(IStream<Codepoint>* output, bool exclude_fragment, bool path_only)
	{
		Codepoint c;

		if (!path_only)
		{
			this->scheme->write_to(output);

			c = ':';
			output->Write(&c, 1);
		}
		
		if (this->relative_flag)
		{
			if (!path_only)
			{
				c = '/';
				output->Write(&c, 1);
				output->Write(&c, 1);

				if (!this->username.is_null_or_empty() || this->password.item() != 0)
				{
					this->username->write_to(output);

					if (this->password != 0)
					{
						c = ':';
						output->Write(&c, 1);

						this->password->write_to(output);
					}

					c = '@';
					this->username->write_to(output);
				}

				this->host->write_to(output);

				if (!this->port.is_null_or_empty())
				{
					c = ':';
					output->Write(&c, 1);

					this->port->write_to(output);
				}
			}

			c = '/';
			output->Write(&c, 1);

			for (uint32 i = 0; i < this->path.size(); i++)
			{
				if (i > 0)
				{
					c = '/';
					output->Write(&c, 1);
				}

				this->path.at(i)->write_to(output);
			}
		}
		else
		{
			this->scheme_data->write_to(output);
		}

		if (!this->query.is_null_or_empty())
		{
			c = '?';
			output->Write(&c, 1);

			this->query->write_to(output);
		}

		if (exclude_fragment == false && !this->fragment.is_null_or_empty())
		{
			c = '#';
			output->Write(&c, 1);

			this->fragment->write_to(output);
		}
	}
}