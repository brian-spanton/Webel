#pragma once

#include "Basic.Ref.h"
#include "Basic.ISerializable.h"

namespace Basic
{
	template <class character_type>
	inline character_type lower_case(character_type character)
	{
		if (character >= 'A' && character <= 'Z')
			character = character - 'A' + 'a';

		return character;
	}

	template <class character_type>
	inline bool base_10(character_type character, byte* value)
	{
		if (character >= '0' && character <= '9')
			(*value) = (byte)(character - '0');
		else
			return false;

		return true;
	}

	template <class character_type>
	inline bool base_16(character_type character, byte* value)
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

	template <class character_type, bool case_sensitive>
	int compare_strings(const character_type* value1, uint32 value1_length, const character_type* value2, uint32 value2_length)
	{
		for (uint32 index = 0; true; index++)
		{
			if (index == value1_length && index == value2_length)
				return 0;

			if (index == value1_length)
				return -1;

			if (index == value2_length)
				return 1;

			character_type value1_character = case_sensitive ? value1[index] : lower_case(value1[index]);
			character_type value2_character = case_sensitive ? value2[index] : lower_case(value2[index]);

			if (value1_character < value2_character)
				return -1;

			if (value1_character > value2_character)
				return 1;
		}
	}

	template <class character_type, bool case_sensitive>
	bool starts_with(const character_type* value1, uint32 value1_length, const character_type* value2, uint32 value2_length)
	{
		if (value1_length < value2_length)
			return false;

		for (uint32 index = 0; index != value2_length; index++)
		{
			character_type value1_character = case_sensitive ? value1[index] : lower_case(value1[index]);
			character_type value2_character = case_sensitive ? value2[index] : lower_case(value2[index]);

			if (value1_character != value2_character)
				return false;
		}

		return true;
	}

	class UnicodeString;

	class StringRef : public Basic::Ref<UnicodeString>
	{
	public:
		StringRef();
		StringRef(const char* name);
		StringRef(UnicodeString* instance);
		StringRef(const StringRef& ref);

		bool operator == (const StringRef& value) const
		{
			return equals<true>(value.instance);
		}

		bool operator != (const StringRef& value) const
		{
			return !equals<true>(value.instance);
		}

		bool operator < (const StringRef& value) const;

		bool operator == (UnicodeString* value) const
		{
			return equals<true>(value);
		}

		bool operator != (UnicodeString* value) const
		{
			return !equals<true>(value);
		}

		void operator = (UnicodeString* value)
		{
			__super::operator = (value);
		}

		void operator = (const StringRef& value)
		{
			__super::operator = (value.instance);
		}

		template <bool case_sensitive>
		bool equals(UnicodeString* value) const
		{
			if (instance == value)
				return true;

			if (instance == 0)
				return false;

			if (instance->equals<case_sensitive>(value))
				return true;

			return false;
		}

		template <bool case_sensitive>
		bool less_than(UnicodeString* value) const
		{
			if (instance == value)
				return true;

			if (instance == 0)
				return false;

			if (instance->less_than<case_sensitive>(value))
				return true;

			return false;
		}

		template <int Count>
		void Initialize(const char (&value)[Count])
		{
			if (value[Count - 1] == 0)
				Initialize(value, Count - 1);
			else
				Initialize(value, Count);
		}

		void Initialize(const char* value, int count);

		bool is_null_or_empty();
	};

	template <class character_type>
	class String : public std::basic_string<character_type>, public IStream<character_type>
	{
	public:
		typedef Basic::Ref<String<character_type> > Ref;

		template <bool case_sensitive>
		int compared_to(const std::basic_string<character_type>* value) const
		{
			return compared_to<case_sensitive>(value->c_str(), value->size());
		}

		template <bool case_sensitive>
		int compared_to(const character_type* value, int value_length) const
		{
			return compare_strings<character_type, case_sensitive>(this->c_str(), this->size(), value, value_length);
		}

		template <class number_type>
		number_type as_base_10(bool* all_digits) const
		{
			number_type value = 0;

			for (uint32 index = 0; index < this->size(); index++)
			{
				value *= 10;

				byte digit_value;

				bool success = base_10<character_type>(this->at(index), &digit_value);
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

				bool success = base_16<character_type>(this->at(index), &digit_value);
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
		bool equals(const std::basic_string<character_type>* value) const
		{
			if (value == 0)
				return false;

			int result = compared_to<case_sensitive>(value);
			if (result != 0)
				return false;

			return true;
		}

		template <bool case_sensitive>
		bool less_than(const std::basic_string<character_type>* value) const
		{
			int result = compared_to<case_sensitive>(value);
			if (result != -1)
				return false;

			return true;
		}

		void write_to(IStream<character_type>* stream)
		{
			stream->Write(this->c_str(), this->size());
		}

		virtual void IStream<character_type>::Write(const character_type* elements, uint32 count)
		{
			append(elements, elements + count);
		}

		virtual void IStream<character_type>::WriteEOF()
		{
		}

		template <bool case_sensitive>
		bool starts_with(const std::basic_string<character_type>* value) const
		{
			return Basic::starts_with<character_type, case_sensitive>(this->c_str(), this->size(), value->c_str(), value->size());
		}
	};

	class ByteString : public String<byte>, public ISerializable
	{
	public:
		typedef Basic::Ref<ByteString, ISerializable> Ref;

		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};

	class UnicodeString : public String<Codepoint>
	{
	public:
		typedef StringRef Ref;

		void ascii_encode(IStream<byte>* bytes);
		void ascii_decode(ByteString* bytes);
		void utf_8_encode(IStream<byte>* bytes);
		void utf_8_decode(ByteString* bytes);
	};

	struct CaseInsensitiveHash : public std::unary_function<StringRef, size_t>
	{
		size_t operator()(const StringRef& value) const
		{
			std::basic_string<Codepoint> lower_case_string;
			lower_case_string.reserve(value->size());

			for (std::basic_string<Codepoint>::iterator it = value->begin(); it != value->end(); it++)
			{
				Codepoint lower_case_character = lower_case(*it);
				lower_case_string.push_back(lower_case_character);
			}

			return std::_Hash_seq((const unsigned char *)lower_case_string.c_str(), lower_case_string.size() * sizeof(Codepoint));
		}
	};

	struct CaseInsensitiveEqualTo : public std::binary_function<StringRef, StringRef, bool>
	{
		bool operator()(const StringRef& left, const StringRef& right) const
		{
			return left.equals<false>(right);
		}
	};

	struct CaseInsensitiveLessThan : public std::binary_function<StringRef, StringRef, bool>
	{
		bool operator()(const StringRef& left, const StringRef& right) const
		{
			return left.less_than<false>(right);
		}
	};

	template <class value_type>
	class StringMapCaseInsensitive : public std::unordered_map<StringRef, value_type, CaseInsensitiveHash, CaseInsensitiveEqualTo>
	{
	};

	template <class value_type>
	class StringMultiMapCaseInsensitive : public std::unordered_multimap<StringRef, value_type, CaseInsensitiveHash, CaseInsensitiveEqualTo>
	{
	};
}

namespace std
{
	template <>
	struct hash<Basic::StringRef> : public unary_function<Basic::StringRef, size_t>
	{
		size_t operator()(const Basic::StringRef& value) const
		{
			return _Hash_seq((const unsigned char *)value->c_str(), value->size() * sizeof(Codepoint));
		}
	};
}

namespace Basic
{
	template <class value_type>
	class StringMapCaseSensitive : public std::unordered_map<StringRef, value_type>
	{
	};

	class StringMap : public StringMapCaseSensitive<UnicodeString::Ref>, public IRefCounted
	{
	public:
		typedef Basic::Ref<StringMap> Ref;

		void set_string(UnicodeString* name, UnicodeString* value)
		{
			iterator it = find(name);
			if (it == end())
				insert(value_type(name, value));
			else
				it->second = value;
		}
	};

	typedef std::vector<UnicodeString::Ref> StringList; // $$$
}
