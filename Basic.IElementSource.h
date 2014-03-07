// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
	template <class T>
	__interface IElementSource : public IRefCounted
	{
		bool Read(uint32 count, const T** out_address, uint32* out_count, bool* yield);
		bool ReadNext(T* element, bool* yield);
		void UndoReadNext();
		void AddObserver(IStream<T>* stream);
		void RemoveObserver(IStream<T>* stream);
	};
}