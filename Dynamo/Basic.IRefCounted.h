#pragma once

namespace Basic
{
	__interface IRefHolder
	{
	};

	__interface IRefCounted
	{
		void AddRef(IRefHolder* holder);
		void Release(IRefHolder* holder);
	};
}