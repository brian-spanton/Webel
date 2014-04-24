// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.Event.h"

namespace Json
{
    using namespace Basic;

    class Token : public IRefCounted
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

        typedef Basic::Ref<Token> Ref;

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
        typedef Basic::Ref<BeginScriptToken> Ref;

        BeginScriptToken() : Token(begin_script_token)
        {
        }
    };

    class EndScriptToken : public Token
    {
    public:
        typedef Basic::Ref<EndScriptToken> Ref;

        EndScriptToken() : Token(end_script_token)
        {
        }
    };

    class BeginParameterToken : public Token
    {
    public:
        typedef Basic::Ref<BeginParameterToken> Ref;

        BeginParameterToken() : Token(begin_parameter_token)
        {
        }
    };

    class EndParameterToken : public Token
    {
    public:
        typedef Basic::Ref<EndParameterToken> Ref;

        EndParameterToken() : Token(end_parameter_token)
        {
        }
    };

    class TokenSeparatorToken : public Token
    {
    public:
        typedef Basic::Ref<TokenSeparatorToken> Ref;

        TokenSeparatorToken() : Token(token_separator_token)
        {
        }
    };

    class TokenToken : public Token
    {
    public:
        typedef Basic::Ref<TokenToken> Ref;

        UnicodeString::Ref value; // REF

        TokenToken() : Token(token_token)
        {
        }
    };

    class BeginArrayToken : public Token
    {
    public:
        typedef Basic::Ref<BeginArrayToken> Ref;

        BeginArrayToken() : Token(begin_array_token)
        {
        }
    };

    class BeginObjectToken : public Token
    {
    public:
        typedef Basic::Ref<BeginObjectToken> Ref;

        BeginObjectToken() : Token(begin_object_token)
        {
        }
    };

    class EndArrayToken : public Token
    {
    public:
        typedef Basic::Ref<EndArrayToken> Ref;

        EndArrayToken() : Token(end_array_token)
        {
        }
    };

    class EndObjectToken : public Token
    {
    public:
        typedef Basic::Ref<EndObjectToken> Ref;

        EndObjectToken() : Token(end_object_token)
        {
        }
    };

    class NameSeparatorToken : public Token
    {
    public:
        typedef Basic::Ref<NameSeparatorToken> Ref;

        NameSeparatorToken() : Token(name_separator_token)
        {
        }
    };

    class ValueSeparatorToken : public Token
    {
    public:
        typedef Basic::Ref<ValueSeparatorToken> Ref;

        ValueSeparatorToken() : Token(value_separator_token)
        {
        }
    };

    class StringToken : public Token
    {
    public:
        typedef Basic::Ref<StringToken> Ref;

        UnicodeString::Ref value; // REF

        StringToken() : Token(string_token)
        {
        }
    };

    class NumberToken : public Token
    {
    public:
        typedef Basic::Ref<NumberToken> Ref;

        long double value;

        NumberToken() : Token(number_token)
        {
        }
    };

    class BoolToken : public Token
    {
    public:
        typedef Basic::Ref<BoolToken> Ref;

        bool value;

        BoolToken() : Token(bool_token)
        {
        }
    };

    class NullToken : public Token
    {
    public:
        typedef Basic::Ref<NullToken> Ref;

        NullToken() : Token(null_token)
        {
        }
    };

    struct Value : public IRefCounted
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

        typedef Basic::Ref<Value> Ref;

        Type type;

        virtual void write_to(Basic::IStream<Codepoint>* stream) = 0;

    protected:
        Value(Type type) : type(type)
        {
        }
    };

    typedef std::vector<Value::Ref> ValueList; // REF

    struct Array : public Value
    {
        typedef Basic::Ref<Array> Ref;

        ValueList elements;

        Array() : Value(Type::array_value)
        {
        }

        virtual void write_to(Basic::IStream<Codepoint>* stream);
    };

    typedef std::unordered_map<UnicodeString::Ref, Value::Ref> MemberList; // REF

    struct Object : public Value
    {
        typedef Basic::Ref<Object> Ref;

        MemberList members;

        Object() : Value(Type::object_value)
        {
        }

        virtual void write_to(Basic::IStream<Codepoint>* stream);
    };

    struct Number : public Value
    {
        typedef Basic::Ref<Number> Ref;

        long double value;

        Number() : Value(Type::number_value)
        {
        }

        virtual void write_to(Basic::IStream<Codepoint>* stream);
    };

    struct String : public Value
    {
        typedef Basic::Ref<String> Ref;

        UnicodeString::Ref value; // REF

        String() : Value(Type::string_value)
        {
        }

        virtual void write_to(Basic::IStream<Codepoint>* stream);
        static void write_value(Basic::UnicodeString::Ref value, Basic::IStream<Codepoint>* stream);
    };

    struct Bool : public Value
    {
        typedef Basic::Ref<Bool> Ref;

        bool value;

        Bool() : Value(Type::bool_value)
        {
        }

        virtual void write_to(Basic::IStream<Codepoint>* stream);
    };

    struct Null : public Value
    {
        typedef Basic::Ref<Null> Ref;

        Null() : Value(Type::null_value)
        {
        }

        virtual void write_to(Basic::IStream<Codepoint>* stream);
    };

    enum EventType
    {
        ready_for_read_token_pointer_event = 0x3000,
    };

    struct ReadyForReadTokenPointerEvent : public IEvent
    {
    public:
        Basic::Ref<IElementSource<Token::Ref> > element_source; // REF

        virtual uint32 get_type();

        void Initialize(IElementSource<Token::Ref>* element_source);

        static bool ReadNext(IEvent* event, Token::Ref* element, bool* yield);
        static void UndoReadNext(IEvent* event);
    };

    class TokenVector : public std::vector<Token::Ref>, public IStream<Token::Ref>
    {
    public:
        typedef Basic::Ref<TokenVector> Ref;

        void IStream<Token::Ref>::Write(const Token::Ref* elements, uint32 count);
        void IStream<Token::Ref>::WriteEOF();

        void write_to(IStream<Token::Ref>* dest);
    };
}