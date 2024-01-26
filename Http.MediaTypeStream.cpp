// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.MediaTypeStream.h"
#include "Http.Globals.h"

namespace Http
{
    void MediaTypeStream::Initialize(MediaType* mediaType)
    {
        this->mediaType = mediaType;
        this->state = State::type_state;
    }

    void MediaTypeStream::ParseError(Codepoint codepoint)
    {
        char message[0x100];
        int result = sprintf_s(message, "codepoint=%04X", codepoint);
        if (result == -1)
            throw FatalError("Http", "MediaTypeStream", "ParseError", "sprintf_s", result);

        Basic::LogDebug("Http", "MediaTypeStream", "ParseError", message);

        this->state = State::parse_error;
    }

    void MediaTypeStream::write_element(Codepoint codepoint)
    {
        //3.7 Media Types
        //
        //   HTTP uses Internet Media Types [17] in the Content-Type (section
        //   14.17) and Accept (section 14.1) header fields in order to provide
        //   open and extensible data typing and type negotiation.
        //
        //       media-type     = type "/" subtype *( ";" parameter )
        //       type           = token
        //       subtype        = token
        //
        //   Parameters MAY follow the type/subtype in the form of attribute/value
        //   pairs (as defined in section 3.6).
        //
        //   The type, subtype, and parameter attribute names are case-
        //   insensitive. Parameter values might or might not be case-sensitive,
        //   depending on the semantics of the parameter name. Linear white space
        //   (LWS) MUST NOT be used between the type and subtype, nor between an
        //   attribute and its value. The presence or absence of a parameter might
        //   be significant to the processing of a media-type, depending on its
        //   definition within the media type registry.

        switch (this->state)
        {
        case State::type_state:
            {
                if (codepoint == Http::globals->FS)
                {
                    this->state = State::subtype_state;
                }
                else if (Http::globals->TOKEN[codepoint])
                {
                    this->mediaType->type->push_back(codepoint);
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::subtype_state:
            {
                if (codepoint == Http::globals->SC)
                {
                    this->name = std::make_shared<UnicodeString>();
                    this->state = State::before_name_state;
                }
                else if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                    this->state = State::after_subtype_state;
                }
                else if (Http::globals->TOKEN[codepoint])
                {
                    this->mediaType->subtype->push_back(codepoint);
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::after_subtype_state:
            {
                if (codepoint == Http::globals->SC)
                {
                    this->name = std::make_shared<UnicodeString>();
                    this->state = State::before_name_state;
                }
                else if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::before_name_state:
            {
                if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                }
                else if (Http::globals->TOKEN[codepoint])
                {
                    this->name->push_back(codepoint);
                    this->state = State::name_state;
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::name_state:
            {
                if (codepoint == Http::globals->EQ)
                {
                    this->value = std::make_shared<UnicodeString>();
                    this->state = State::before_value_state;
                }
                else if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                    this->value = std::make_shared<UnicodeString>();
                    this->state = State::after_name_state;
                }
                else if (Http::globals->TOKEN[codepoint])
                {
                    this->name->push_back(codepoint);
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::after_name_state:
            {
                if (codepoint == Http::globals->EQ)
                {
                    this->state = State::before_value_state;
                }
                else if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::before_value_state:
            {
                if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                }
                else if (codepoint == '\"')
                {
                    this->state = State::value_quoted_state;
                }
                else if (Http::globals->TOKEN[codepoint])
                {
                    this->value->push_back(codepoint);
                    this->state = State::value_state;
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::value_state:
            {
                if (codepoint == Http::globals->SC)
                {
                    NameValueCollection::value_type nv(this->name, this->value);
                    this->mediaType->parameters->insert(nv);

                    this->value = 0;
                    this->name = std::make_shared<UnicodeString>();

                    this->state = State::before_name_state;
                }
                else if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                    NameValueCollection::value_type nv(this->name, this->value);
                    this->mediaType->parameters->insert(nv);

                    this->value = 0;
                    this->name = 0;

                    this->state = State::after_value_state;
                }
                else if (Http::globals->TOKEN[codepoint])
                {
                    this->value->push_back(codepoint);
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::value_quoted_state:
            {
                if (codepoint == '\"')
                {
                    NameValueCollection::value_type nv(this->name, this->value);
                    this->mediaType->parameters->insert(nv);

                    this->value = 0;
                    this->name = 0;

                    this->state = State::after_value_state;
                }
                else
                {
                    this->value->push_back(codepoint);
                }
            }
            break;

        case State::after_value_state:
            {
                if (codepoint == Http::globals->SC)
                {
                    this->name = std::make_shared<UnicodeString>();
                    this->state = State::before_name_state;
                }
                else if (codepoint == Http::globals->SP || codepoint == Http::globals->HT)
                {
                }
                else
                {
                    ParseError(codepoint);
                }
            }
            break;

        case State::parse_error:
            break;

        default:
            throw FatalError("Http", "MediaTypeStream", "write_element", "unhandled state", this->state);
        }
    }

    void MediaTypeStream::write_eof()
    {
        if (this->state == State::value_state)
        {
            // complete the final in progress name-value pair
            NameValueCollection::value_type nv(this->name, this->value);
            this->mediaType->parameters->insert(nv);
            this->name = 0;
            this->value = 0;
        }
    }
}