// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.TextWriter.h"

namespace Basic
{
	class NameValueCollection : public StringMultiMapCaseInsensitive<Basic::StringRef>, public IRefCounted
	{
	public:
		typedef Basic::Ref<NameValueCollection> Ref;

		bool get_string(UnicodeString* name, UnicodeString::Ref* value)
		{
			iterator it = find(name);
			if (it == end())
				return false;

			(*value) = it->second;
			return true;
		}

		template <class number_type>
		bool get_base_10(UnicodeString* name, number_type* value)
		{
			iterator it = find(name);
			if (it == end())
				return false;

			bool all_digits;
			(*value) = it->second->as_base_10<number_type>(&all_digits);

			if (!all_digits)
				return false;

			return true;
		}

		void set_string(UnicodeString* name, UnicodeString* value)
		{
			// adds new, or updates the first matching element

			iterator it = find(name);
			if (it == end())
				insert(value_type(name, value));
			else
				it->second = value;
		}

		void set_base_10(UnicodeString* name, int value)
		{
			UnicodeString::Ref value_string = New<UnicodeString>();

			TextWriter writer(value_string);
			writer.WriteFormat<0x10>("%d", value);

			set_string(name, value_string);
		}
	};
}