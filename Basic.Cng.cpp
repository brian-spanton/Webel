// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Cng.h"
#include "Basic.Globals.h"

namespace Basic
{
    BCRYPT_KEY_HANDLE::BCRYPT_KEY_HANDLE() :
        handle(0)
    {
    }

    BCRYPT_KEY_HANDLE::~BCRYPT_KEY_HANDLE()
    {
        if (this->handle != 0)
        {
            NTSTATUS error = BCryptDestroyKey(this->handle);
            if (error != 0)
                Basic::LogError("Basic", "BCRYPT_KEY_HANDLE::~BCRYPT_KEY_HANDLE BCryptDestroyKey failed", error);
        }
    }

    ::BCRYPT_KEY_HANDLE* BCRYPT_KEY_HANDLE::operator &()
    {
        return &this->handle;
    }

    BCRYPT_KEY_HANDLE::operator ::BCRYPT_KEY_HANDLE () const
    {
        return this->handle;
    }

    BCRYPT_ALG_HANDLE::BCRYPT_ALG_HANDLE() :
        handle(0)
    {
    }

    BCRYPT_ALG_HANDLE::~BCRYPT_ALG_HANDLE()
    {
        if (this->handle != 0)
        {
            NTSTATUS error = BCryptCloseAlgorithmProvider(this->handle, 0);
            if (error != 0)
                Basic::LogError("Basic", "BCRYPT_ALG_HANDLE::~BCRYPT_ALG_HANDLE BCryptCloseAlgorithmProvider failed", error);
        }
    }

    ::BCRYPT_ALG_HANDLE* BCRYPT_ALG_HANDLE::operator &()
    {
        return &this->handle;
    }

    BCRYPT_ALG_HANDLE::operator ::BCRYPT_ALG_HANDLE () const
    {
        return this->handle;
    }

    NCRYPT_KEY_HANDLE::NCRYPT_KEY_HANDLE() :
        handle(0)
    {
    }

    NCRYPT_KEY_HANDLE::~NCRYPT_KEY_HANDLE()
    {
        if (this->handle != 0)
        {
            NTSTATUS error = NCryptFreeObject(this->handle);
            if (error != 0)
                Basic::LogError("Basic", "NCRYPT_KEY_HANDLE::~NCRYPT_KEY_HANDLE NCryptFreeObject failed", error);
        }
    }

    ::NCRYPT_KEY_HANDLE* NCRYPT_KEY_HANDLE::operator &()
    {
        return &this->handle;
    }

    NCRYPT_KEY_HANDLE::operator ::NCRYPT_KEY_HANDLE () const
    {
        return this->handle;
    }

    HCERTSTORE::HCERTSTORE() :
        handle(0)
    {
    }

    HCERTSTORE::HCERTSTORE(::HCERTSTORE handle) :
        handle(handle)
    {
    }

    HCERTSTORE::~HCERTSTORE()
    {
        if (this->handle != 0)
        {
            BOOL success = CertCloseStore(this->handle, CERT_CLOSE_STORE_CHECK_FLAG);
            if (!success)
                Basic::LogError("Basic", "HCERTSTORE::~HCERTSTORE CertCloseStore failed", GetLastError());
        }
    }

    void HCERTSTORE::operator = (::HCERTSTORE handle)
    {
        this->handle = handle;
    }

    HCERTSTORE::operator ::HCERTSTORE () const
    {
        return this->handle;
    }

    PCCERT_CONTEXT::PCCERT_CONTEXT() :
        handle(0)
    {
    }

    PCCERT_CONTEXT::PCCERT_CONTEXT(::PCCERT_CONTEXT handle) :
        handle(handle)
    {
    }

    PCCERT_CONTEXT::~PCCERT_CONTEXT()
    {
        if (this->handle != 0)
        {
            BOOL success = CertFreeCertificateContext(this->handle);
            if (!success)
                Basic::LogError("Basic", "PCCERT_CONTEXT::~PCCERT_CONTEXT CertFreeCertificateContext failed", GetLastError());
        }
    }

    void PCCERT_CONTEXT::operator = (::PCCERT_CONTEXT handle)
    {
        this->handle = handle;
    }

    PCCERT_CONTEXT::operator ::PCCERT_CONTEXT () const
    {
        return this->handle;
    }

    ::PCCERT_CONTEXT PCCERT_CONTEXT::operator -> () const
    {
        return this->handle;
    }
}