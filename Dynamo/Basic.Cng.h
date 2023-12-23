#pragma once

namespace Basic
{
	class __declspec(novtable) BCRYPT_KEY_HANDLE
	{
	public:
		::BCRYPT_KEY_HANDLE handle;

		BCRYPT_KEY_HANDLE();
		~BCRYPT_KEY_HANDLE();

		::BCRYPT_KEY_HANDLE* operator &();
		operator ::BCRYPT_KEY_HANDLE () const;
	};

	class __declspec(novtable) BCRYPT_ALG_HANDLE
	{
	public:
		::BCRYPT_ALG_HANDLE handle;

		BCRYPT_ALG_HANDLE();
		~BCRYPT_ALG_HANDLE();

		::BCRYPT_ALG_HANDLE* operator &();
		operator ::BCRYPT_ALG_HANDLE () const;
	};

	class __declspec(novtable) NCRYPT_KEY_HANDLE
	{
	public:
		::NCRYPT_KEY_HANDLE handle;

		NCRYPT_KEY_HANDLE();
		~NCRYPT_KEY_HANDLE();

		::NCRYPT_KEY_HANDLE* operator &();
		operator ::NCRYPT_KEY_HANDLE () const;
	};

	class __declspec(novtable) HCERTSTORE
	{
	public:
		::HCERTSTORE handle;

		HCERTSTORE();
		HCERTSTORE(::HCERTSTORE handle);
		~HCERTSTORE();

		void operator = (::HCERTSTORE handle);
		operator ::HCERTSTORE () const;
	};

	class __declspec(novtable) PCCERT_CONTEXT
	{
	public:
		::PCCERT_CONTEXT handle;

		PCCERT_CONTEXT();
		PCCERT_CONTEXT(::PCCERT_CONTEXT handle);
		~PCCERT_CONTEXT();

		void operator = (::PCCERT_CONTEXT handle);
		operator ::PCCERT_CONTEXT () const;
		::PCCERT_CONTEXT operator -> () const;
	};
}