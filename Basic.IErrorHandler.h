// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IRefCounted.h"
#include "Basic.IStream.h"
#include "Basic.TextWriter.h"

namespace Basic
{
	__interface IErrorHandler : public IRefCounted
	{
		bool HandleError(const char* context, uint32 error);
		Basic::IStream<Codepoint>* DebugStream();
		Basic::TextWriter* DebugWriter();
	};
}