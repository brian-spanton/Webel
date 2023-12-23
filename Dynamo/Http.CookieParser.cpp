#include "stdafx.h"
#include "Http.CookieParser.h"
#include "Http.Globals.h"

namespace Http
{
	using namespace Basic;

	void CookieParser::InitializeStatics()
	{
		attr_map.insert(StringMap::value_type(Http::globals->expires_av_name, State::expires_state));
		attr_map.insert(StringMap::value_type(Http::globals->max_age_av_name, State::max_age_state));
		attr_map.insert(StringMap::value_type(Http::globals->domain_av_name, State::domain_state));
		attr_map.insert(StringMap::value_type(Http::globals->path_av_name, State::path_state));
		attr_map.insert(StringMap::value_type(Http::globals->secure_av_name, State::secure_state));
		attr_map.insert(StringMap::value_type(Http::globals->httponly_av_name, State::http_only_state));
	}

	void CookieParser::Initialize(Cookie* cookie)
	{
		this->cookie = cookie;
		this->state = State::in_name_state;
	}

	void CookieParser::Write(const Codepoint* elements, uint32 count)
	{
		// $ fully conform to RFC6265 section 5.2

		for (uint32 element_index = 0; element_index < count; element_index++)
		{
			Codepoint codepoint = elements[element_index];

			switch (this->state)
			{
			case State::in_name_state:
				{
					if (codepoint == Http::globals->EQ)
					{
						while (Http::globals->WSP[this->cookie->name->back()])
						{
							this->cookie->name->pop_back();
						}

						this->cookie->value = New<UnicodeString>();
						this->state = State::before_value_state;
					}
					else
					{
						this->cookie->name->push_back(codepoint);
					}
				}
				break;

			case State::before_value_state:
				{
					if (codepoint == ';')
					{
						this->state = State::before_attribute_name_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->cookie->value->push_back(codepoint);
						this->state = State::in_value_state;
					}
				}
				break;

			case State::in_value_state:
				{
					if (codepoint == ';')
					{
						while (Http::globals->WSP[this->cookie->value->back()])
						{
							this->cookie->value->pop_back();
						}

						this->state = State::before_attribute_name_state;
					}
					else
					{
						this->cookie->value->push_back(codepoint);
					}
				}
				break;

			case State::before_attribute_name_state:
				{
					if (!Http::globals->WSP[codepoint])
					{
						for (this->attr = attr_map.begin(); this->attr != attr_map.end(); this->attr++)
						{
							if (lower_case(codepoint) == this->attr->first->at(0))
								break;
						}

						if (this->attr != attr_map.end())
						{
							this->matched = 1;
							this->state = State::in_attribute_name_state;
						}
						else
						{
							this->state = State::ignore_attribute_state;
						}
					}
				}
				break;

			case State::in_attribute_name_state:
				{
					if (codepoint == ';')
					{
						this->state = State::before_attribute_name_state;
					}
					else if (lower_case(codepoint) == this->attr->first->at(this->matched))
					{
						this->matched++;

						if (this->matched == this->attr->first->size())
							this->state = this->attr->second;
					}
					else
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			case State::ignore_attribute_state:
				{
					if (codepoint == ';')
					{
						this->state = State::before_attribute_name_state;
					}
				}
				break;

			case State::expires_state:
				{
					if (codepoint == Http::globals->EQ)
					{
						this->state = State::ignore_attribute_state;
						// $ this->state = State::expires_value_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			case State::max_age_state:
				{
					if (codepoint == Http::globals->EQ)
					{
						this->state = State::ignore_attribute_state;
						// $ this->state = State::max_age_value_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			case State::domain_state:
				{
					if (codepoint == Http::globals->EQ)
					{
						this->state = State::domain_value_start_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			case State::domain_value_start_state:
				{
					if (codepoint == ';')
					{
						this->state = State::before_attribute_name_state;
					}
					else if (codepoint == '.')
					{
						this->node = New<UnicodeString>();
						this->state = State::domain_value_state;
					}
					else if (Http::globals->TOKEN[codepoint])
					{
						this->node = New<UnicodeString>();
						this->node->push_back(lower_case(codepoint));
						this->state = State::domain_value_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						ParseError(codepoint);
					}
				}
				break;

			case State::domain_value_state:
				{
					if (codepoint == ';')
					{
						if (this->node->size() > 0)
							this->cookie->domain.insert(this->cookie->domain.begin(), this->node);

						this->node = (UnicodeString*)0;

						this->state = State::before_attribute_name_state;
					}
					else if (codepoint == '.')
					{
						if (this->node->size() > 0)
						{
							this->cookie->domain.insert(this->cookie->domain.begin(), this->node);
							this->node = New<UnicodeString>();
						}
					}
					else if (Http::globals->TOKEN[codepoint])
					{
						this->node->push_back(codepoint);
					}
					else
					{
						ParseError(codepoint);
					}
				}
				break;

			case State::path_state:
				{
					if (codepoint == Http::globals->EQ)
					{
						this->state = State::path_value_start_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			case State::path_value_start_state:
				{
					if (codepoint == ';')
					{
						this->state = State::before_attribute_name_state;
					}
					else if (codepoint == Http::globals->FS)
					{
						this->node = New<UnicodeString>();
						this->state = State::path_value_state;
					}
					else if (Http::globals->TOKEN[codepoint])
					{
						this->node = New<UnicodeString>();
						this->node->push_back(codepoint);
						this->state = State::path_value_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						ParseError(codepoint);
					}
				}
				break;

			case State::path_value_state:
				{
					if (codepoint == ';')
					{
						if (this->node->size() > 0)
							this->cookie->path.push_back(this->node);

						this->node = (UnicodeString*)0;

						this->state = State::before_attribute_name_state;
					}
					else if (codepoint == Http::globals->FS)
					{
						this->cookie->path.push_back(this->node);
						this->node = New<UnicodeString>();
					}
					else if (Http::globals->TOKEN[codepoint])
					{
						this->node->push_back(codepoint);
					}
					else
					{
						ParseError(codepoint);
					}
				}
				break;

			case State::secure_state:
				{
					if (codepoint == ';')
					{
						this->cookie->secure_only_flag = true;
						this->state = State::before_attribute_name_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			case State::http_only_state:
				{
					if (codepoint == ';')
					{
						this->cookie->http_only_flag = true;
						this->state = State::before_attribute_name_state;
					}
					else if (!Http::globals->WSP[codepoint])
					{
						this->state = State::ignore_attribute_state;
					}
				}
				break;

			default:
				throw new Exception("CookieParser::Process unexpected state");
			}
		}
	}

	void CookieParser::ParseError(Codepoint codepoint)
	{
		HandleError("Cookie parse error");
		this->state = State::parse_error;
	}

	void CookieParser::WriteEOF()
	{
		switch (this->state)
		{
		case State::in_value_state:
		case State::ignore_attribute_state:
		case State::expires_value_state:
		case State::max_age_value_state:
		case State::domain_value_state:
		case State::path_value_state:
		case State::before_attribute_name_state:
		case State::secure_state:
		case State::http_only_state:
			break;

		default:
			ParseError(0);
			break;
		}
	}
}