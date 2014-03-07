// Copyright © 2013 Brian Spanton

#pragma once

namespace Http
{
	using namespace Basic;

	struct MediaType : public IRefCounted
	{
		typedef Basic::Ref<MediaType> Ref;

		UnicodeString::Ref type; // REF
		UnicodeString::Ref subtype; // REF
		NameValueCollection::Ref parameters; // REF

		void Initialize();
		void Initialize(UnicodeString* value);
		bool equals(MediaType* value);
	};
}
