// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.String.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.SingleByteDecoder.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    void initialize_unicode(std::shared_ptr<UnicodeString>* variable, const char* value, int count)
    {
        (*variable) = std::make_shared<UnicodeString>();
        (*variable)->reserve(count);

        SingleByteDecoder decoder(Basic::globals->ascii_index, variable->get());
        decoder.write_elements((const byte*)value, count);
    }

    void initialize_ascii(std::shared_ptr<ByteString>* variable, const char* value, int count)
    {
        (*variable) = std::make_shared<ByteString>();
        (*variable)->reserve(count);

        (*variable)->write_elements((const byte*)value, count);
    }

    void initialize_ascii(ByteString* variable, const char* value, int count)
    {
        variable->write_elements((const byte*)value, count);
    }

    void ascii_encode(UnicodeString* value, IStream<byte>* stream)
    {
        SingleByteEncoder encoder;
        encoder.Initialize(Basic::globals->ascii_index, stream);
        encoder.write_elements(value->address(), value->size());
    }

    void ascii_decode(ByteString* bytes, UnicodeString* value)
    {
        value->clear();
        value->reserve(bytes->size());

        SingleByteDecoder decoder(Basic::globals->ascii_index, value);
        decoder.write_elements(bytes->address(), bytes->size());
    }

    void utf_8_encode(UnicodeString* value, IStream<byte>* stream)
    {
        Utf8Encoder encoder;
        encoder.set_destination(stream);
        encoder.write_elements(value->address(), value->size());
    }

    void utf_8_decode(ByteString* bytes, UnicodeString* value)
    {
        value->clear();
        value->reserve(bytes->size());

        Utf8Decoder decoder;
        decoder.set_destination(value);
        decoder.write_elements(bytes->address(), bytes->size());
    }
}

bool operator == (const Basic::UnicodeStringRef& left_value, const Basic::UnicodeStringRef& right_value)
{
    return Basic::equals<Basic::UnicodeString, true>(left_value.get(), right_value.get());
}

bool operator != (const Basic::UnicodeStringRef& left_value, const Basic::UnicodeStringRef& right_value)
{
    return !Basic::equals<Basic::UnicodeString, true>(left_value.get(), right_value.get());
}
