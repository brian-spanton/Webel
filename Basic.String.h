// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IStreamWriter.h"

namespace Basic
{
    template <typename element_type>
    inline element_type lower_case(element_type character)
    {
        if (character >= 'A' && character <= 'Z')
            character = character - 'A' + 'a';

        return character;
    }

    template <typename element_type>
    inline bool base_10(element_type character, byte* value)
    {
        if (character >= '0' && character <= '9')
            (*value) = (byte)(character - '0');
        else
            return false;

        return true;
    }

    template <typename element_type>
    inline bool base_16(element_type character, byte* value)
    {
        if (character >= '0' && character <= '9')
            (*value) = (byte)(character - '0');
        else if (character >= 'a' && character <= 'f')
            (*value) = (byte)(10 + character - 'a');
        else if (character >= 'A' && character <= 'F')
            (*value) = (byte)(10 + character - 'A');
        else
            return false;

        return true;
    }

    template <typename element_type, bool case_sensitive>
    int compare_strings(const element_type* value1, uint32 value1_length, const element_type* value2, uint32 value2_length)
    {
        for (uint32 index = 0; true; index++)
        {
            if (index == value1_length && index == value2_length)
                return 0;

            if (index == value1_length)
                return -1;

            if (index == value2_length)
                return 1;

            element_type value1_character = case_sensitive ? value1[index] : lower_case(value1[index]);
            element_type value2_character = case_sensitive ? value2[index] : lower_case(value2[index]);

            if (value1_character < value2_character)
                return -1;

            if (value1_character > value2_character)
                return 1;
        }
    }

    template <typename element_type, bool case_sensitive>
    bool starts_with(const element_type* value1, uint32 value1_length, const element_type* value2, uint32 value2_length)
    {
        if (value1_length < value2_length)
            return false;

        for (uint32 index = 0; index != value2_length; index++)
        {
            element_type value1_character = case_sensitive ? value1[index] : lower_case(value1[index]);
            element_type value2_character = case_sensitive ? value2[index] : lower_case(value2[index]);

            if (value1_character != value2_character)
                return false;
        }

        return true;
    }

    template <typename element_type>
    class String : public std::basic_string<element_type>, public IStream<element_type>, public IStreamWriter<element_type>, public IVector<element_type>
    {
    public:
        template <bool case_sensitive>
        int compared_to(const std::basic_string<element_type>* value) const
        {
            return compared_to<case_sensitive>(value->c_str(), value->size());
        }

        template <bool case_sensitive>
        int compared_to(const element_type* value, int value_length) const
        {
            return compare_strings<element_type, case_sensitive>(this->address(), this->size(), value, value_length);
        }

        template <class number_type>
        number_type as_base_10(bool* all_digits) const
        {
            number_type value = 0;

            for (uint32 index = 0; index < this->size(); index++)
            {
                value *= 10;

                byte digit_value;

                bool success = base_10<element_type>(this->at(index), &digit_value);
                if (!success)
                {
                    if (all_digits != 0)
                        (*all_digits) = false;

                    return value;
                }

                value += digit_value;
            }

            if (all_digits != 0)
                (*all_digits) = true;

            return value;
        }

        template <class number_type>
        number_type as_base_16(bool* all_digits) const
        {
            number_type value = 0;

            for (uint32 index = 0; index < this->size(); index++)
            {
                value *= 0x10;

                byte digit_value;

                bool success = base_16<element_type>(this->at(index), &digit_value);
                if (!success)
                {
                    if (all_digits != 0)
                        (*all_digits) = false;

                    return value;
                }

                value += digit_value;
            }

            if (all_digits != 0)
                (*all_digits) = true;

            return value;
        }

        template <bool case_sensitive>
        bool equals(const std::basic_string<element_type>* value) const
        {
            if (value == 0)
                return false;

            int result = compared_to<case_sensitive>(value);
            if (result != 0)
                return false;

            return true;
        }

        template <bool case_sensitive>
        bool less_than(const std::basic_string<element_type>* value) const
        {
            int result = compared_to<case_sensitive>(value);
            if (result != -1)
                return false;

            return true;
        }

        template <bool case_sensitive>
        bool starts_with(const std::basic_string<element_type>* value) const
        {
            return Basic::starts_with<element_type, case_sensitive>(this->address(), this->size(), value->c_str(), value->size());
        }

        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count)
        {
            append(elements, elements + count);
        }

        virtual void IStream<element_type>::write_element(element_type element)
        {
            push_back(element);
        }

        virtual void IStream<element_type>::write_eof()
        {
        }

        virtual void IStreamWriter<element_type>::write_to_stream(IStream<element_type>* stream) const
        {
            stream->write_elements(this->address(), this->size());
        }

        element_type* address() const
        {
            return (element_type*)c_str();
        }

        uint32 size() const
        {
            return std::basic_string<element_type>::size();
        }
    };

    typedef String<byte> ByteString;
    typedef std::shared_ptr<ByteString> ByteStringRef;

    typedef String<Codepoint> UnicodeString;
    typedef std::shared_ptr<UnicodeString> UnicodeStringRef;
   
    void ascii_encode(UnicodeString* value, IStream<byte>* bytes);
    void ascii_decode(ByteString* bytes, UnicodeString* value);
    void utf_8_encode(UnicodeString* value, IStream<byte>* bytes);
    void utf_8_decode(ByteString* bytes, UnicodeString* value);

    template <typename string_type, bool case_sensitive>
    bool equals(string_type* left_value, string_type* right_value)
    {
        if (left_value == right_value)
            return true;

        if (left_value == 0)
            return false;

        if (left_value->equals<case_sensitive>(right_value))
            return true;

        return false;
    }

    void initialize_unicode(std::shared_ptr<UnicodeString>* variable, const char* value, int count);

    template <int Count>
    void initialize_unicode(std::shared_ptr<UnicodeString>* variable, const char (&value)[Count])
    {
        if (value[Count - 1] == 0)
            initialize_unicode(variable, value, Count - 1);
        else
            initialize_unicode(variable, value, Count);
    }

    void initialize_ascii(std::shared_ptr<ByteString>* variable, const char* value, int count);

    template <int Count>
    void initialize_ascii(std::shared_ptr<ByteString>* variable, const char (&value)[Count])
    {
        if (value[Count - 1] == 0)
            initialize_ascii(variable, value, Count - 1);
        else
            initialize_ascii(variable, value, Count);
    }

    void initialize_ascii(ByteString* variable, const char* value, int count);

    template <int Count>
    void initialize_ascii(ByteString* variable, const char (&value)[Count])
    {
        if (value[Count - 1] == 0)
            initialize_ascii(variable, value, Count - 1);
        else
            initialize_ascii(variable, value, Count);
    }

    template <class string_type>
    bool is_null_or_empty(string_type* string_ref)
    {
        if (string_ref == 0)
            return true;

        if (string_ref->size() == 0)
            return true;

        return false;
    }
}

bool operator == (const Basic::UnicodeStringRef& left_value, const Basic::UnicodeStringRef& right_value);
bool operator != (const Basic::UnicodeStringRef& left_value, const Basic::UnicodeStringRef& right_value);

/*
VCSamples
Copyright (c) Microsoft Corporation
All rights reserved.
MIT License
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stddef.h>

inline size_t fnv1a_hash_bytes(const unsigned char* first, size_t count) {
#if defined(_WIN64)
    static_assert(sizeof(size_t) == 8, "This code is for 64-bit size_t.");
    const size_t fnv_offset_basis = 14695981039346656037ULL;
    const size_t fnv_prime = 1099511628211ULL;
#else /* defined(_WIN64) */
    static_assert(sizeof(size_t) == 4, "This code is for 32-bit size_t.");
    const size_t fnv_offset_basis = 2166136261U;
    const size_t fnv_prime = 16777619U;
#endif /* defined(_WIN64) */

    size_t result = fnv_offset_basis;
    for (size_t next = 0; next < count; ++next)
    {
        // fold in another byte
        result ^= (size_t)first[next];
        result *= fnv_prime;
    }
    return (result);
}

namespace std
{
    template <>
    struct hash<Basic::UnicodeStringRef> : public unary_function<Basic::UnicodeStringRef, size_t>
    {
        size_t operator()(const Basic::UnicodeStringRef& value) const
        {
            return fnv1a_hash_bytes((const unsigned char *)value->address(), value->size() * sizeof(Codepoint));
        }
    };

    template <>
    struct equal_to<Basic::UnicodeStringRef> : public binary_function<Basic::UnicodeStringRef, Basic::UnicodeStringRef, bool>
    {
        bool operator()(const Basic::UnicodeStringRef& left_value, const Basic::UnicodeStringRef& right_value) const
        {
            return Basic::equals<Basic::UnicodeString, true>(left_value.get(), right_value.get());
        }
    };
}

namespace Basic
{
    struct CaseInsensitiveHash : public std::unary_function<UnicodeStringRef, size_t>
    {
        size_t operator()(const UnicodeStringRef& value) const
        {
            String<Codepoint> lower_case_string;
            lower_case_string.reserve(value->size());

            for (std::basic_string<Codepoint>::iterator it = value->begin(); it != value->end(); it++)
            {
                Codepoint lower_case_character = lower_case(*it);
                lower_case_string.push_back(lower_case_character);
            }

            return fnv1a_hash_bytes((const unsigned char *)lower_case_string.address(), lower_case_string.size() * sizeof(Codepoint));
        }
    };

    struct CaseInsensitiveEqualTo : public std::binary_function<UnicodeStringRef, UnicodeStringRef, bool>
    {
        bool operator()(const UnicodeStringRef& left, const UnicodeStringRef& right) const
        {
            return equals<UnicodeString, false>(left.get(), right.get());
        }
    };

    template <class value_type>
    class StringMapCaseInsensitive : public std::unordered_map<UnicodeStringRef, value_type, CaseInsensitiveHash, CaseInsensitiveEqualTo>
    {
    };

    template <class value_type>
    class StringMultiMapCaseInsensitive : public std::unordered_multimap<UnicodeStringRef, value_type, CaseInsensitiveHash, CaseInsensitiveEqualTo>
    {
    };

    template <class value_type>
    class StringMapCaseSensitive : public std::unordered_map<UnicodeStringRef, value_type>
    {
    };

    class StringMap : public StringMapCaseSensitive<UnicodeStringRef>
    {
    public:
        void set_string(const UnicodeStringRef& name, const UnicodeStringRef& value)
        {
            iterator it = find(name);
            if (it == end())
                insert(value_type(name, value));
            else
                it->second = value;
        }
    };
}
