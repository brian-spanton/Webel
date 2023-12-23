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
	StringRef::StringRef() :
		Basic::Ref<UnicodeString>()
	{
	}

	StringRef::StringRef(UnicodeString* instance) :
		Basic::Ref<UnicodeString>(instance)
	{
	}

	StringRef::StringRef(const StringRef& ref) :
		Basic::Ref<UnicodeString>(ref)
	{
	}

	void StringRef::Initialize(const char* value, int count)
	{
		(*this) = New<UnicodeString>();

		Inline<SingleByteDecoder> decoder;
		decoder.Initialize(Basic::globals->ascii_index, this->item());
		decoder.Write((const byte*)value, count);
	}

	bool StringRef::is_null_or_empty()
	{
		if (instance == 0)
			return true;

		if (instance->size() == 0)
			return true;

		return false;
	}

	bool StringRef::operator < (const StringRef& value) const
	{
		return instance->compared_to<true>(value.instance) == -1;
	}

	void ByteString::SerializeTo(IStream<byte>* stream)
	{
		stream->Write(c_str(), size());
	}

	void UnicodeString::ascii_encode(IStream<byte>* bytes)
	{
		Inline<SingleByteEncoder> encoder;
		encoder.Initialize(Basic::globals->ascii_index, bytes);
		encoder.Write(c_str(), size());
	}

	void UnicodeString::ascii_decode(ByteString* bytes)
	{
		clear();
		reserve(bytes->size());

		Inline<SingleByteDecoder> decoder;
		decoder.Initialize(Basic::globals->ascii_index, this);
		decoder.Write(bytes->c_str(), bytes->size());
	}

	void UnicodeString::utf_8_encode(IStream<byte>* bytes)
	{
		Inline<Utf8Encoder> encoder;
		encoder.set_destination(bytes);
		encoder.Write(c_str(), size());
	}

	void UnicodeString::utf_8_decode(ByteString* bytes)
	{
		clear();
		reserve(bytes->size());

		Inline<Utf8Decoder> decoder;
		decoder.set_destination(this);
		decoder.Write(bytes->c_str(), bytes->size());
	}
}
