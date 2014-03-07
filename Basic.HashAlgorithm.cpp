// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.Globals.h"

namespace Basic
{
	HashAlgorithm::HashAlgorithm()
	{
	}

	HashAlgorithm::~HashAlgorithm()
	{
	}

	void HashAlgorithm::Initialize(LPCWSTR algorithm, bool hmac)
	{
		this->hmac = hmac;

		ULONG flags = hmac ? BCRYPT_ALG_HANDLE_HMAC_FLAG : 0;

		NTSTATUS error = BCryptOpenAlgorithmProvider(&this->hash_algorithm, algorithm, 0, flags);
		if (error != 0)
			throw new Exception("BCryptOpenAlgorithmProvider", error);

		DWORD value_length;
		error = BCryptGetProperty(this->hash_algorithm, BCRYPT_HASH_LENGTH, (byte*)&this->hash_output_length, sizeof(this->hash_output_length), &value_length, 0);
		if (error != 0)
			throw new Exception("BCryptGetProperty", error);
	}
}