#pragma once

#include "Basic.IProcess.h"

namespace Dynamo
{
	using namespace Basic;

	__interface IStorage
	{
		void Store(UnicodeString::Ref name_space, UnicodeString::Ref name, Basic::Ref<IStream<byte> >* stream);
		void Find(UnicodeString::Ref name_space, UnicodeString::Ref query, Basic::Ref<IProcess> results);
	};
}
