// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.MediaType.h"
#include "Http.MediaTypeStream.h"

namespace Http
{
	void MediaType::Initialize()
	{
		this->type = New<UnicodeString>();
		this->subtype = New<UnicodeString>();
		this->parameters = New<NameValueCollection >();
	}

	void MediaType::Initialize(UnicodeString* value)
	{
		Initialize();

		Inline<MediaTypeStream> frame;
		frame.Initialize(this);

		frame.Write(value->c_str(), value->size());
		frame.WriteEOF();
	}

	bool MediaType::equals(MediaType* value)
	{
		if (!value->type.equals<false>(this->type))
			return false;

		if (!value->subtype.equals<false>(this->subtype))
			return false;

		return true;
	}
}
