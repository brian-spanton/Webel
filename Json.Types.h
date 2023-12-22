// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.Event.h"

namespace Json
{
    using namespace Basic;

    class Token
    {
    public:
        enum Type
        {
            begin_script_token,
            end_script_token,
            begin_parameter_token,
            end_parameter_token,
            token_separator_token,
            token_token,
            begin_array_token,
            begin_object_token,
            end_array_token,
            end_object_token,
            name_separator_token,
            value_separator_token,
            string_token,
            number_token,
            bool_token,
            null_token,
        };

        Type type;

        void GetDebugString(char* debug_string, int count)
        {
            GetDebugString(this->type, debug_string, count);
        }

        static void GetDebugString(Type type, char* debug_string, int count);

    protected:
        Token(Type type) : type(type)
        {
        }
    };

    class BeginScriptToken : public Token
    {
    public:
        BeginScriptToken() : Token(begin_script_token)
        {
        }
    };

    class EndScriptToken : public Token
    {
    public:
        EndScriptToken() : Token(end_script_token)
        {
        }
    };

    class BeginParameterToken : public Token
    {
    public:
        BeginParameterToken() : Token(begin_parameter_token)
        {
        }
    };

    class EndParameterToken : public Token
    {
    public:
        EndParameterToken() : Token(end_parameter_token)
        {
        }
    };

    class TokenSeparatorToken : public Token
    {
    public:
        TokenSeparatorToken() : Token(token_separator_token)
        {
        }
    };

    class TokenToken : public Token
    {
    public:
        UnicodeStringRef value;

        TokenToken() : Token(token_token)
        {
        }
    };

    class BeginArrayToken : public Token
    {
    public:
        BeginArrayToken() : Token(begin_array_token)
        {
        }
    };

    class BeginObjectToken : public Token
    {
    public:
        BeginObjectToken() : Token(begin_object_token)
        {
        }
    };

    class EndArrayToken : public Token
    {
    public:
        EndArrayToken() : Token(end_array_token)
        {
        }
    };

    class EndObjectToken : public Token
    {
    public:
        EndObjectToken() : Token(end_object_token)
        {
        }
    };

    class NameSeparatorToken : public Token
    {
    public:
        NameSeparatorToken() : Token(name_separator_token)
        {
        }
    };

    class ValueSeparatorToken : public Token
    {
    public:
        ValueSeparatorToken() : Token(value_separator_token)
        {
        }
    };

    class StringToken : public Token
    {
    public:
        UnicodeStringRef value;

        StringToken() : Token(string_token)
        {
        }
    };

    class NumberToken : public Token
    {
    public:
        long double value = 0;

        NumberToken() : Token(number_token)
        {
        }
    };

    class BoolToken : public Token
    {
    public:
        bool value = false;

        BoolToken() : Token(bool_token)
        {
        }
    };

    class NullToken : public Token
    {
    public:
        NullToken() : Token(null_token)
        {
        }
    };

    struct Value : public IStreamWriter<Codepoint>
    {
        enum Type
        {
            null_value,
            bool_value,
            object_value,
            array_value,
            number_value,
            string_value,
        };

        Type type = Type::null_value;

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const = 0;

    protected:
        Value(Type type) : type(type)
        {
        }
    };

    typedef std::vector<std::shared_ptr<Value> > ValueList;

    struct Array : public Value
    {
        ValueList elements;

        Array() : Value(Type::array_value)
        {
        }

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const;
    };

    typedef std::unordered_map<UnicodeStringRef, std::shared_ptr<Value> > MemberList;

    struct Object : public Value
    {
        MemberList members;

        Object() : Value(Type::object_value)
        {
        }

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const;
    };

    struct Number : public Value
    {
        long double value = 0;

        Number() : Value(Type::number_value)
        {
        }

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const;
    };

    struct String : public Value
    {
        UnicodeStringRef value;

        String() : Value(Type::string_value)
        {
        }

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const;
        static void write_value(Basic::UnicodeStringRef value, Basic::IStream<Codepoint>* stream);
    };

    struct Bool : public Value
    {
        bool value = false;

        Bool() : Value(Type::bool_value)
        {
        }

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const;
    };

    struct Null : public Value
    {
        Null() : Value(Type::null_value)
        {
        }

        virtual void IStreamWriter<Codepoint>::write_to_stream(Basic::IStream<Codepoint>* stream) const;
    };

    typedef std::vector<std::shared_ptr<Token> > TokenVector;
}