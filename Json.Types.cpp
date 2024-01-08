// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Types.h"
#include "Json.Globals.h"
#include "Basic.Frame.h"

namespace Json
{
    using namespace Basic;

    void Token::GetDebugString(Type type, char* debug_string, int count)
    {
        switch (type)
        {
#define CASE(e) \
        case e: \
            strcpy_s(debug_string, count, #e); \
            break

            CASE(begin_array_token);
            CASE(begin_object_token);
            CASE(end_array_token);
            CASE(end_object_token);
            CASE(name_separator_token);
            CASE(value_separator_token);
            CASE(string_token);
            CASE(number_token);
            CASE(bool_token);
            CASE(null_token);

#undef CASE

        default:
            throw FatalError("Json", "Token::GetDebugString unhandled token type");
        }
    }

    void Array::write_to_stream(Basic::IStream<Codepoint>* stream) const
    {
        TextWriter writer(stream);

        bool has_complex = false;

        for (ValueList::const_iterator it = this->elements.cbegin(); it != this->elements.cend(); it++)
        {
            if ((*it)->type == Value::Type::array_value
                || (*it)->type == Value::Type::object_value)
            {
                has_complex = true;
            }
        }

        stream->write_element(Json::globals->begin_array);

        if (this->elements.size() > 0)
        {
            if (!has_complex)
            {
                writer.write_literal(" ");

                for (ValueList::const_iterator it = this->elements.cbegin(); it != this->elements.cend(); it++)
                {
                    if (it != this->elements.begin())
                    {
                        stream->write_element(Json::globals->value_separator);
                        writer.write_literal(" ");
                    }

                    (*it)->write_to_stream(stream);
                }

                writer.write_literal(" ");
            }
            else
            {
                // $ stream->Indent();
                writer.WriteLine();

                for (ValueList::const_iterator it = this->elements.cbegin(); it != this->elements.cend(); it++)
                {
                    if (it != this->elements.begin())
                    {
                        stream->write_element(Json::globals->value_separator);
                        writer.WriteLine();
                    }

                    (*it)->write_to_stream(stream);
                }

                // $ stream->Unindent();
            }
        }

        stream->write_element(Json::globals->end_array);
    }

    void Object::write_to_stream(Basic::IStream<Codepoint>* stream) const
    {
        TextWriter writer(stream);

        bool has_complex = false;

        for (MemberList::const_iterator it = this->members.cbegin(); it != this->members.cend(); it++)
        {
            if (it->second->type == Value::Type::array_value
                || it->second->type == Value::Type::object_value)
            {
                has_complex = true;
            }
        }

        stream->write_element(Json::globals->begin_object);

        if (this->members.size() > 0)
        {
            if (!has_complex)
            {
                writer.write_literal(" ");

                for (MemberList::const_iterator it = this->members.cbegin(); it != this->members.cend(); it++)
                {
                    if (it != this->members.begin())
                    {
                        stream->write_element(Json::globals->value_separator);
                        writer.write_literal(" ");
                    }

                    Json::String::write_value(it->first, stream);
                    writer.write_literal(" ");
                    stream->write_element(Json::globals->name_separator);
                    writer.write_literal(" ");
                    it->second->write_to_stream(stream);
                }

                writer.write_literal(" ");
            }
            else
            {
                // $ stream->Indent();
                writer.WriteLine();

                for (MemberList::const_iterator it = this->members.cbegin(); it != this->members.cend(); it++)
                {
                    if (it != this->members.begin())
                    {
                        stream->write_element(Json::globals->value_separator);
                        writer.WriteLine();
                    }

                    Json::String::write_value(it->first, stream);
                    writer.write_literal(" "); // $ isn't there a more efficient way to write a space?  change TextWriter to a set of global functions?
                    stream->write_element(Json::globals->name_separator);
                    writer.write_literal(" ");
                    it->second->write_to_stream(stream);
                }

                // $ stream->Unindent();
            }
        }

        stream->write_element(Json::globals->end_object);
    }

    void Number::write_to_stream(Basic::IStream<Codepoint>* stream) const
    {
        TextWriter writer(stream);

        // $ handle exponents and fractions
        writer.WriteFormat<64>("%d", (uint64)this->value);
    }

    void String::write_to_stream(Basic::IStream<Codepoint>* stream) const
    {
        write_value(this->value, stream);
    }

    void String::write_value(Basic::UnicodeStringRef value, Basic::IStream<Codepoint>* stream)
    {
        stream->write_element('\"');

        for (UnicodeString::iterator it = value->begin(); it != value->end(); it++)
        {
            switch (*it)
            {
            case 0x0022:
                stream->write_element('\\');
                stream->write_element('\"');
                break;

            case 0x005C:
                stream->write_element('\\');
                stream->write_element('\\');
                break;

            case 0x002F:
                stream->write_element('\\');
                stream->write_element('/');
                break;

            case 0x0008:
                stream->write_element('\\');
                stream->write_element('b');
                break;

            case 0x000C:
                stream->write_element('\\');
                stream->write_element('f');
                break;

            case 0x000A:
                stream->write_element('\\');
                stream->write_element('n');
                break;

            case 0x000D:
                stream->write_element('\\');
                stream->write_element('r');
                break;

            case 0x0009:
                stream->write_element('\\');
                stream->write_element('t');
                break;

            default:
                stream->write_element(*it);
                break;
            }
        }

        stream->write_element('\"');
    }

    void Bool::write_to_stream(Basic::IStream<Codepoint>* stream) const
    {
        UnicodeStringRef string = this->value ? Json::globals->json_true : Json::globals->json_false;
        string->write_to_stream(stream);
    }

    void Null::write_to_stream(Basic::IStream<Codepoint>* stream) const
    {
        Json::globals->json_null->write_to_stream(stream);
    }
}
