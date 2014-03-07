// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Tokenizer.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Parser.h"
#include "Basic.StreamFrame.h"
#include "Basic.Event.h"

namespace Json
{
	using namespace Basic;

	void Tokenizer::InitializeStatics()
	{
		literal_map.insert(LiteralMap::value_type(Json::globals->json_false, Json::globals->false_token.item()));
		literal_map.insert(LiteralMap::value_type(Json::globals->json_null, Json::globals->null_token.item()));
		literal_map.insert(LiteralMap::value_type(Json::globals->json_true, Json::globals->true_token.item()));
	}

	void Tokenizer::Initialize(IStream<Token::Ref>* output)
	{
		__super::Initialize();
		this->output = output;
	}

	void Tokenizer::Emit(Token::Ref token)
	{
		this->output->Write(&token, 1);
	}

	void Tokenizer::Error(const char* error)
	{
		HandleError(error);
		switch_to_state(State::error_state);
	}

	void Tokenizer::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				if (c == Json::globals->begin_script)
				{
					BeginScriptToken::Ref token = New<BeginScriptToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->end_script)
				{
					EndScriptToken::Ref token = New<EndScriptToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->begin_parameter)
				{
					BeginParameterToken::Ref token = New<BeginParameterToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->end_parameter)
				{
					EndParameterToken::Ref token = New<EndParameterToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->token_separator)
				{
					TokenSeparatorToken::Ref token = New<TokenSeparatorToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->begin_array)
				{
					BeginArrayToken::Ref token = New<BeginArrayToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->begin_object)
				{
					BeginObjectToken::Ref token = New<BeginObjectToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->end_array)
				{
					EndArrayToken::Ref token = New<EndArrayToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->end_object)
				{
					EndObjectToken::Ref token = New<EndObjectToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->name_separator)
				{
					NameSeparatorToken::Ref token = New<NameSeparatorToken>();
					Emit(token.item());
				}
				else if (c == Json::globals->value_separator)
				{
					ValueSeparatorToken::Ref token = New<ValueSeparatorToken>();
					Emit(token.item());
				}
				else if (c == '\"')
				{
					this->string = New<UnicodeString>();
					switch_to_state(State::string_state);
				}
				else if (c == '-')
				{
					this->sign = -1;
					this->dec_number_stream.Initialize(&this->whole);
					switch_to_state(State::number_state);
				}
				else if (c >= '0' && c <= '9')
				{
					this->sign = 1;
					this->dec_number_stream.Initialize(&this->whole);

					Event::UndoReadNext(event);
					switch_to_state(State::number_state);
				}
				else
				{
					for (this->literal_it = literal_map.begin(); this->literal_it != literal_map.end(); this->literal_it++)
					{
						if (c == this->literal_it->first->at(0))
							break;
					}

					if (this->literal_it != literal_map.end())
					{
						this->matched = 1;
						switch_to_state(State::literal_state);
					}
					else if (c == Json::globals->token_separator || Json::globals->ws->find(c) == UnicodeString::npos)
					{
						this->string = New<UnicodeString>();

						Event::UndoReadNext(event);
						switch_to_state(State::token_state);
					}
				}
			}
			break;

		case State::literal_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				if (c == this->literal_it->first->at(this->matched))
				{
					this->matched++;

					if (this->matched == this->literal_it->first->size())
					{
						Emit(this->literal_it->second);
						switch_to_state(State::start_state);
					}
				}
				else
				{
					this->string = New<UnicodeString>();
					this->string->insert(this->string->end(), this->literal_it->first->begin(), this->literal_it->first->begin() + this->matched);

					Event::UndoReadNext(event);
					switch_to_state(State::token_state);
				}
			}
			break;

		case State::token_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				if (c == Json::globals->token_separator)
				{
					TokenToken::Ref token = New<TokenToken>();
					token->value = this->string;
					Emit(token.item());

					TokenSeparatorToken::Ref token2 = New<TokenSeparatorToken>();
					Emit(token2.item());

					this->string = New<UnicodeString>();
				}
				else if (c == Json::globals->begin_parameter)
				{
					TokenToken::Ref token = New<TokenToken>();
					token->value = this->string;
					Emit(token.item());

					BeginParameterToken::Ref token2 = New<BeginParameterToken>();
					Emit(token2.item());

					switch_to_state(State::start_state);
				}
				else if (c == Json::globals->end_script)
				{
					TokenToken::Ref token = New<TokenToken>();
					token->value = this->string;
					Emit(token.item());

					EndScriptToken::Ref token2 = New<EndScriptToken>();
					Emit(token2.item());

					switch_to_state(State::start_state);
				}
				else if (Json::globals->ws->find(c) != UnicodeString::npos)
				{
					TokenToken::Ref token = New<TokenToken>();
					token->value = this->string;
					Emit(token.item());

					switch_to_state(State::start_state);
				}
				else
				{
					this->string->push_back(c);
				}
			}
			break;

		case State::string_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				if (c == '\"')
				{
					StringToken::Ref token = New<StringToken>();
					token->value = this->string;
					Emit(token.item());

					switch_to_state(State::start_state);
				}
				else if (c == '\\')
				{
					switch_to_state(State::escape_state);
				}
				else
				{
					this->string->push_back(c);
				}
			}
			break;

		case State::escape_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				switch (c)
				{
				case '\"':
					this->string->push_back(0x0022);
					switch_to_state(State::string_state);
					break;

				case '\\':
					this->string->push_back(0x005C);
					switch_to_state(State::string_state);
					break;

				case '/':
					this->string->push_back(0x002F);
					switch_to_state(State::string_state);
					break;

				case 'b':
					this->string->push_back(0x0008);
					switch_to_state(State::string_state);
					break;

				case 'f':
					this->string->push_back(0x000C);
					switch_to_state(State::string_state);
					break;

				case 'n':
					this->string->push_back(0x000A);
					switch_to_state(State::string_state);
					break;

				case 'r':
					this->string->push_back(0x000D);
					switch_to_state(State::string_state);
					break;

				case 't':
					this->string->push_back(0x0009);
					switch_to_state(State::string_state);
					break;

				case 'u':
					this->hex_number_stream.Initialize(&this->whole);
					switch_to_state(State::character_code_state);
					break;
				}
			}
			break;

		case State::character_code_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				bool success = this->hex_number_stream.WriteDigit(c);
				if (!success)
				{
					if (this->hex_number_stream.get_digit_count() != 4)
					{
						Event::UndoReadNext(event);
						Error("character code does not have 4 digits");
					}
					else
					{
						// $ handle utf-16 surrogate pairs

						this->string->push_back((Codepoint)this->whole);

						Event::UndoReadNext(event);
						switch_to_state(State::string_state);
					}
				}
			}
			break;

		case State::number_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				bool success = this->dec_number_stream.WriteDigit(c);
				if (!success)
				{
					if (this->dec_number_stream.get_digit_count() == 0)
					{
						Event::UndoReadNext(event);
						Error("number token has no digits");
					}
					else if (c == '.')
					{
						this->number = (long double)this->whole * (long double)this->sign;
						this->dec_number_stream.Initialize(&this->fraction);
						switch_to_state(State::fraction_state);
					}
					else if (Basic::lower_case(c) == 'e')
					{
						this->number = (long double)this->whole * (long double)this->sign;
						this->dec_number_stream.Initialize(&this->exponent);
						switch_to_state(State::exponent_state);
					}
					else
					{
						this->number = (long double)this->whole * (long double)this->sign;

						NumberToken::Ref token = New<NumberToken>();
						token->value = this->number;
						Emit(token.item());

						Event::UndoReadNext(event);
						switch_to_state(State::start_state);
					}
				}
			}
			break;

		case State::fraction_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				bool success = this->dec_number_stream.WriteDigit(c);
				if (!success)
				{
					if (this->dec_number_stream.get_digit_count() == 0)
					{
						Event::UndoReadNext(event);
						Error("fraction part has no digits");
					}
					else if (Basic::lower_case(c) == 'e')
					{
						this->number += this->fraction / (long double)pow((long double)10, (long double)this->dec_number_stream.get_digit_count());
						this->dec_number_stream.Initialize(&this->exponent);
						switch_to_state(State::exponent_state);
					}
					else
					{
						this->number += this->fraction / (long double)pow((long double)10, (long double)this->dec_number_stream.get_digit_count());

						NumberToken::Ref token = New<NumberToken>();
						token->value = this->number;
						Emit(token.item());

						Event::UndoReadNext(event);
						switch_to_state(State::start_state);
					}
				}
			}
			break;

		case State::exponent_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				bool success = this->dec_number_stream.WriteDigit(c);
				if (!success)
				{
					if (this->dec_number_stream.get_digit_count() == 0)
					{
						Event::UndoReadNext(event);
						Error("exponent part has no digits");
					}
					else
					{
						this->number = (long double)pow(this->number, (long double)this->exponent);

						NumberToken::Ref token = New<NumberToken>();
						token->value = this->number;
						Emit(token.item());

						Event::UndoReadNext(event);
						switch_to_state(State::start_state);
					}
				}
			}
			break;

		default:
			throw new Exception("Json::Tokenizer::Write unexpected state");
		}
	}
}