#pragma once

#include "Basic.IRefCounted.h"
#include "Basic.Cng.h"

namespace Basic
{
	class HashAlgorithm : public IRefCounted
	{
	public:
		typedef Basic::Ref<HashAlgorithm> Ref;

		Basic::BCRYPT_ALG_HANDLE hash_algorithm;
		DWORD hash_output_length;
		bool hmac;

		HashAlgorithm();
		virtual ~HashAlgorithm();

		void Initialize(LPCWSTR algorithm, bool hmac);
	};
}