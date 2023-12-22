// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Event.h"
#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Basic.CountStream.h"
#include "Basic.Frame.h"

namespace Tls
{
    using namespace Basic;

    ///////////////////////////////////////////////////////////////////////////
    // Vector
    //
    // Allows vector types to carry serialization parameters
    ///////////////////////////////////////////////////////////////////////////

    template <typename element_type, uint32 encoded_length_min, uint32 encoded_length_max = 0, uint32 length_of_encoded_length = 0>
    struct Vector : public std::vector<element_type>, public IStream<element_type>, public IStreamWriter<element_type>, public IVector<element_type>
    {
        typedef element_type element_type;
        static const uint32 encoded_length_min = encoded_length_min;
        static const uint32 encoded_length_max = encoded_length_max;
        static const uint32 length_of_encoded_length = length_of_encoded_length;

        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count)
        {
            this->insert(this->end(), elements, elements + count);
        }

        virtual void IStream<element_type>::write_element(element_type element)
        {
            write_elements(&element, 1);
        }

        virtual void IStream<element_type>::write_eof()
        {
            HandleError("unexpected eof");
        }

        virtual void IStreamWriter<element_type>::write_to_stream(IStream<element_type>* stream) const
        {
            stream->write_elements(this->address(), this->size());
        }

        void operator = (const Vector<element_type, encoded_length_min, encoded_length_max, length_of_encoded_length>& rvalue)
        {
            std::vector<element_type>& lvalue(*this);
            lvalue = (const std::vector<element_type>&)rvalue;
        }

        element_type* address() const
        {
            return (element_type*)&*begin();
        }

        uint32 size() const
        {
            return std::vector<element_type>::size();
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // enums
    ///////////////////////////////////////////////////////////////////////////

    enum ConnectionEnd : uint8
    {
        server,
        client,
    };

    enum PRFAlgorithm : uint8
    {
        tls_prf_tls_v1,
        tls_prf_sha256,
    };

    enum InterimCipher : uint8
    {
        ic_NULL,
        ic_RC4_128,
        ic_3DES_EDE_CBC,
        ic_AES_128_CBC,
        ic_AES_256_CBC,
    };

    enum BulkCipherAlgorithm : uint8
    {
        bca_null,
        rc4,
        _3des,
        aes,
    };

    enum CipherType : uint8
    {
        stream,
        block,
        aead,
    };

    enum MACAlgorithm : uint8
    {
        ma_null,
        hmac_md5,
        hmac_sha1,
        hmac_sha256,
        hmac_sha384,
        hmac_sha512,
    };

    enum CompressionMethod : uint8
    {
        cm_null,
    };

    enum ContentType : uint8
    {
        change_cipher_spec = 20,
        alert = 21,
        handshake = 22,
        application_data = 23,
        heartbeat_content_type = 24,
    };

    enum AlertLevel : uint8
    {
        warning = 1,
        fatal = 2,
    };

    enum AlertDescription : uint8
    {
        close_notify = 0,
        unexpected_message = 10,
        bad_record_mac = 20,
        decryption_failed_RESERVED = 21,
        record_overflow = 22,
        decompression_failure = 30,
        handshake_failure = 40,
        no_certificate_RESERVED = 41,
        bad_certificate = 42,
        unsupported_certificate = 43,
        certificate_revoked = 44,
        certificate_expired = 45,
        certificate_unknown = 46,
        illegal_parameter = 47,
        unknown_ca = 48,
        access_denied = 49,
        decode_error = 50,
        decrypt_error = 51,
        export_restriction_RESERVED = 60,
        protocol_version = 70,
        insufficient_security = 71,
        internal_error = 80,
        user_canceled = 90,
        no_renegotiation = 100,
        unsupported_extension = 110,
    };

    enum ExtensionType : uint16
    {
        // http:// www.iana.org/assignments/tls-extensiontype-values

        server_name = 0, // RFC6066 
        max_fragment_length = 1, // RFC6066 
        client_certificate_url = 2, // RFC6066 
        trusted_ca_keys = 3, // RFC6066 
        truncated_hmac = 4, // RFC6066 
        status_request = 5, // RFC6066 
        user_mapping = 6, // RFC4681 
        client_authz = 7, // RFC5878 
        server_authz = 8, // RFC5878 
        cert_type = 9, // RFC6091 
        elliptic_curves = 10, // RFC4492 
        ec_point_formats = 11, // RFC4492 
        srp = 12, // RFC5054 
        signature_algorithms = 13, // RFC5246 
        use_srtp = 14, // RFC5764 
        heartbeat_extension_type = 15, // RFC6520 
        SessionTicket_TLS = 35, // RFC4507 
        renegotiation_info = 0xff01, // RFC5746 
    };

    enum HeartbeatMode : uint8
    {
        peer_allowed_to_send = 1,
        peer_not_allowed_to_send = 2,
    };

    enum HeartbeatMessageType : uint8
    {
        heartbeat_request = 1,
        heartbeat_response = 2,
    };

    enum HashAlgorithm : uint8
    {
        none = 0,
        md5 = 1,
        sha1 = 2,
        sha224 = 3,
        sha256 = 4,
        sha384 = 5,
        sha512 = 6,
    };

    enum SignatureAlgorithm : uint8
    {
        anonymous = 0,
        _sa_rsa = 1,
        dsa = 2,
        ecdsa = 3,
    };

    enum KeyExchangeAlgorithm : uint8
    {
        DHE_DSS,
        DHE_RSA,
        DH_ANON,
        _KEA_RSA,
        DH_DSS,
        DH_RSA,
        RSA_PSK,
        ECDHE_RSA,
        ECDH_ECDSA,
        ECDH_RSA,
        ECDHE_ECDSA,
        /* may be extended, e.g., for ECDH -- see [TLSECC] */
    };

    enum ClientCertificateType : uint8
    {
        rsa_sign = 1,
        dss_sign = 2,
        rsa_fixed_dh = 3,
        dss_fixed_dh = 4,
        rsa_ephemeral_dh_RESERVED = 5,
        dss_ephemeral_dh_RESERVED = 6,
        fortezza_dms_RESERVED = 20,
    };

    enum PublicValueEncoding : uint8
    {
        implicit,
        _explicit,
    };

    enum HandshakeType : uint8
    {
        hello_request = 0,
        client_hello = 1,
        server_hello = 2,
        certificate = 11,
        server_key_exchange = 12,
        certificate_request = 13,
        server_hello_done = 14,
        certificate_verify = 15,
        client_key_exchange = 16,
        finished = 20,
    };

    enum NameType : uint8
    {
        host_name = 0,
    };

    enum CipherSuite : uint16
    {
        cs_TLS_NULL_WITH_NULL_NULL = 0x0000, // [RFC5246] 
        cs_TLS_RSA_WITH_NULL_MD5 = 0x0001, // [RFC5246] 
        cs_TLS_RSA_WITH_NULL_SHA = 0x0002, // [RFC5246] 
        cs_TLS_RSA_EXPORT_WITH_RC4_40_MD5 = 0x0003, // [RFC4346][RFC6347] 
        cs_TLS_RSA_WITH_RC4_128_MD5 = 0x0004, // [RFC5246][RFC6347] 
        cs_TLS_RSA_WITH_RC4_128_SHA = 0x0005, // [RFC5246][RFC6347] 
        cs_TLS_RSA_EXPORT_WITH_RC2_CBC_40_MD5 = 0x0006, // [RFC4346] 
        cs_TLS_RSA_WITH_IDEA_CBC_SHA = 0x0007, // [RFC5469] 
        cs_TLS_RSA_EXPORT_WITH_DES40_CBC_SHA = 0x0008, // [RFC4346] 
        cs_TLS_RSA_WITH_DES_CBC_SHA = 0x0009, // [RFC5469] 
        cs_TLS_RSA_WITH_3DES_EDE_CBC_SHA = 0x000A, // [RFC5246] 
        cs_TLS_DH_DSS_EXPORT_WITH_DES40_CBC_SHA = 0x000B, // [RFC4346] 
        cs_TLS_DH_DSS_WITH_DES_CBC_SHA = 0x000C, // [RFC5469] 
        cs_TLS_DH_DSS_WITH_3DES_EDE_CBC_SHA = 0x000D, // [RFC5246] 
        cs_TLS_DH_RSA_EXPORT_WITH_DES40_CBC_SHA = 0x000E, // [RFC4346] 
        cs_TLS_DH_RSA_WITH_DES_CBC_SHA = 0x000F, // [RFC5469] 
        cs_TLS_DH_RSA_WITH_3DES_EDE_CBC_SHA = 0x0010, // [RFC5246] 
        cs_TLS_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA = 0x0011, // [RFC4346] 
        cs_TLS_DHE_DSS_WITH_DES_CBC_SHA = 0x0012, // [RFC5469] 
        cs_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA = 0x0013, // [RFC5246] 
        cs_TLS_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA = 0x0014, // [RFC4346] 
        cs_TLS_DHE_RSA_WITH_DES_CBC_SHA = 0x0015, // [RFC5469] 
        cs_TLS_DHE_RSA_WITH_3DES_EDE_CBC_SHA = 0x0016, // [RFC5246] 
        cs_TLS_DH_anon_EXPORT_WITH_RC4_40_MD5 = 0x0017, // [RFC4346][RFC6347] 
        cs_TLS_DH_anon_WITH_RC4_128_MD5 = 0x0018, // [RFC5246][RFC6347] 
        cs_TLS_DH_anon_EXPORT_WITH_DES40_CBC_SHA = 0x0019, // [RFC4346] 
        cs_TLS_DH_anon_WITH_DES_CBC_SHA = 0x001A, // [RFC5469] 
        cs_TLS_DH_anon_WITH_3DES_EDE_CBC_SHA = 0x001B, // [RFC5246] 
        cs_TLS_KRB5_WITH_DES_CBC_SHA = 0x001E, // [RFC2712] 
        cs_TLS_KRB5_WITH_3DES_EDE_CBC_SHA = 0x001F, // [RFC2712] 
        cs_TLS_KRB5_WITH_RC4_128_SHA = 0x0020, // [RFC2712][RFC6347] 
        cs_TLS_KRB5_WITH_IDEA_CBC_SHA = 0x0021, // [RFC2712] 
        cs_TLS_KRB5_WITH_DES_CBC_MD5 = 0x0022, // [RFC2712] 
        cs_TLS_KRB5_WITH_3DES_EDE_CBC_MD5 = 0x0023, // [RFC2712] 
        cs_TLS_KRB5_WITH_RC4_128_MD5 = 0x0024, // [RFC2712][RFC6347] 
        cs_TLS_KRB5_WITH_IDEA_CBC_MD5 = 0x0025, // [RFC2712] 
        cs_TLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA = 0x0026, // [RFC2712] 
        cs_TLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA = 0x0027, // [RFC2712] 
        cs_TLS_KRB5_EXPORT_WITH_RC4_40_SHA = 0x0028, // [RFC2712][RFC6347] 
        cs_TLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5 = 0x0029, // [RFC2712] 
        cs_TLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5 = 0x002A, // [RFC2712] 
        cs_TLS_KRB5_EXPORT_WITH_RC4_40_MD5 = 0x002B, // [RFC2712][RFC6347] 
        cs_TLS_PSK_WITH_NULL_SHA = 0x002C, // [RFC4785] 
        cs_TLS_DHE_PSK_WITH_NULL_SHA = 0x002D, // [RFC4785] 
        cs_TLS_RSA_PSK_WITH_NULL_SHA = 0x002E, // [RFC4785] 
        cs_TLS_RSA_WITH_AES_128_CBC_SHA = 0x002F, // [RFC5246] 
        cs_TLS_DH_DSS_WITH_AES_128_CBC_SHA = 0x0030, // [RFC5246] 
        cs_TLS_DH_RSA_WITH_AES_128_CBC_SHA = 0x0031, // [RFC5246] 
        cs_TLS_DHE_DSS_WITH_AES_128_CBC_SHA = 0x0032, // [RFC5246] 
        cs_TLS_DHE_RSA_WITH_AES_128_CBC_SHA = 0x0033, // [RFC5246] 
        cs_TLS_DH_anon_WITH_AES_128_CBC_SHA = 0x0034, // [RFC5246] 
        cs_TLS_RSA_WITH_AES_256_CBC_SHA = 0x0035, // [RFC5246] 
        cs_TLS_DH_DSS_WITH_AES_256_CBC_SHA = 0x0036, // [RFC5246] 
        cs_TLS_DH_RSA_WITH_AES_256_CBC_SHA = 0x0037, // [RFC5246] 
        cs_TLS_DHE_DSS_WITH_AES_256_CBC_SHA = 0x0038, // [RFC5246] 
        cs_TLS_DHE_RSA_WITH_AES_256_CBC_SHA = 0x0039, // [RFC5246] 
        cs_TLS_DH_anon_WITH_AES_256_CBC_SHA = 0x003A, // [RFC5246] 
        cs_TLS_RSA_WITH_NULL_SHA256 = 0x003B, // [RFC5246] 
        cs_TLS_RSA_WITH_AES_128_CBC_SHA256 = 0x003C, // [RFC5246] 
        cs_TLS_RSA_WITH_AES_256_CBC_SHA256 = 0x003D, // [RFC5246] 
        cs_TLS_DH_DSS_WITH_AES_128_CBC_SHA256 = 0x003E, // [RFC5246] 
        cs_TLS_DH_RSA_WITH_AES_128_CBC_SHA256 = 0x003F, // [RFC5246] 
        cs_TLS_DHE_DSS_WITH_AES_128_CBC_SHA256 = 0x0040, // [RFC5246] 
        cs_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA = 0x0041, // [RFC5932] 
        cs_TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA = 0x0042, // [RFC5932] 
        cs_TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA = 0x0043, // [RFC5932] 
        cs_TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA = 0x0044, // [RFC5932] 
        cs_TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA = 0x0045, // [RFC5932] 
        cs_TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA = 0x0046, // [RFC5932] 
        cs_TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 = 0x0067, // [RFC5246] 
        cs_TLS_DH_DSS_WITH_AES_256_CBC_SHA256 = 0x0068, // [RFC5246] 
        cs_TLS_DH_RSA_WITH_AES_256_CBC_SHA256 = 0x0069, // [RFC5246] 
        cs_TLS_DHE_DSS_WITH_AES_256_CBC_SHA256 = 0x006A, // [RFC5246] 
        cs_TLS_DHE_RSA_WITH_AES_256_CBC_SHA256 = 0x006B, // [RFC5246] 
        cs_TLS_DH_anon_WITH_AES_128_CBC_SHA256 = 0x006C, // [RFC5246] 
        cs_TLS_DH_anon_WITH_AES_256_CBC_SHA256 = 0x006D, // [RFC5246] 
        cs_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA = 0x0084, // [RFC5932] 
        cs_TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA = 0x0085, // [RFC5932] 
        cs_TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA = 0x0086, // [RFC5932] 
        cs_TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA = 0x0087, // [RFC5932] 
        cs_TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA = 0x0088, // [RFC5932] 
        cs_TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA = 0x0089, // [RFC5932] 
        cs_TLS_PSK_WITH_RC4_128_SHA = 0x008A, // [RFC4279][RFC6347] 
        cs_TLS_PSK_WITH_3DES_EDE_CBC_SHA = 0x008B, // [RFC4279] 
        cs_TLS_PSK_WITH_AES_128_CBC_SHA = 0x008C, // [RFC4279] 
        cs_TLS_PSK_WITH_AES_256_CBC_SHA = 0x008D, // [RFC4279] 
        cs_TLS_DHE_PSK_WITH_RC4_128_SHA = 0x008E, // [RFC4279][RFC6347] 
        cs_TLS_DHE_PSK_WITH_3DES_EDE_CBC_SHA = 0x008F, // [RFC4279] 
        cs_TLS_DHE_PSK_WITH_AES_128_CBC_SHA = 0x0090, // [RFC4279] 
        cs_TLS_DHE_PSK_WITH_AES_256_CBC_SHA = 0x0091, // [RFC4279] 
        cs_TLS_RSA_PSK_WITH_RC4_128_SHA = 0x0092, // [RFC4279][RFC6347] 
        cs_TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA = 0x0093, // [RFC4279] 
        cs_TLS_RSA_PSK_WITH_AES_128_CBC_SHA = 0x0094, // [RFC4279] 
        cs_TLS_RSA_PSK_WITH_AES_256_CBC_SHA = 0x0095, // [RFC4279] 
        cs_TLS_RSA_WITH_SEED_CBC_SHA = 0x0096, // [RFC4162] 
        cs_TLS_DH_DSS_WITH_SEED_CBC_SHA = 0x0097, // [RFC4162] 
        cs_TLS_DH_RSA_WITH_SEED_CBC_SHA = 0x0098, // [RFC4162] 
        cs_TLS_DHE_DSS_WITH_SEED_CBC_SHA = 0x0099, // [RFC4162] 
        cs_TLS_DHE_RSA_WITH_SEED_CBC_SHA = 0x009A, // [RFC4162] 
        cs_TLS_DH_anon_WITH_SEED_CBC_SHA = 0x009B, // [RFC4162] 
        cs_TLS_RSA_WITH_AES_128_GCM_SHA256 = 0x009C, // [RFC5288] 
        cs_TLS_RSA_WITH_AES_256_GCM_SHA384 = 0x009D, // [RFC5288] 
        cs_TLS_DHE_RSA_WITH_AES_128_GCM_SHA256 = 0x009E, // [RFC5288] 
        cs_TLS_DHE_RSA_WITH_AES_256_GCM_SHA384 = 0x009F, // [RFC5288] 
        cs_TLS_DH_RSA_WITH_AES_128_GCM_SHA256 = 0x00A0, // [RFC5288] 
        cs_TLS_DH_RSA_WITH_AES_256_GCM_SHA384 = 0x00A1, // [RFC5288] 
        cs_TLS_DHE_DSS_WITH_AES_128_GCM_SHA256 = 0x00A2, // [RFC5288] 
        cs_TLS_DHE_DSS_WITH_AES_256_GCM_SHA384 = 0x00A3, // [RFC5288] 
        cs_TLS_DH_DSS_WITH_AES_128_GCM_SHA256 = 0x00A4, // [RFC5288] 
        cs_TLS_DH_DSS_WITH_AES_256_GCM_SHA384 = 0x00A5, // [RFC5288] 
        cs_TLS_DH_anon_WITH_AES_128_GCM_SHA256 = 0x00A6, // [RFC5288] 
        cs_TLS_DH_anon_WITH_AES_256_GCM_SHA384 = 0x00A7, // [RFC5288] 
        cs_TLS_PSK_WITH_AES_128_GCM_SHA256 = 0x00A8, // [RFC5487] 
        cs_TLS_PSK_WITH_AES_256_GCM_SHA384 = 0x00A9, // [RFC5487] 
        cs_TLS_DHE_PSK_WITH_AES_128_GCM_SHA256 = 0x00AA, // [RFC5487] 
        cs_TLS_DHE_PSK_WITH_AES_256_GCM_SHA384 = 0x00AB, // [RFC5487] 
        cs_TLS_RSA_PSK_WITH_AES_128_GCM_SHA256 = 0x00AC, // [RFC5487] 
        cs_TLS_RSA_PSK_WITH_AES_256_GCM_SHA384 = 0x00AD, // [RFC5487] 
        cs_TLS_PSK_WITH_AES_128_CBC_SHA256 = 0x00AE, // [RFC5487] 
        cs_TLS_PSK_WITH_AES_256_CBC_SHA384 = 0x00AF, // [RFC5487] 
        cs_TLS_PSK_WITH_NULL_SHA256 = 0x00B0, // [RFC5487] 
        cs_TLS_PSK_WITH_NULL_SHA384 = 0x00B1, // [RFC5487] 
        cs_TLS_DHE_PSK_WITH_AES_128_CBC_SHA256 = 0x00B2, // [RFC5487] 
        cs_TLS_DHE_PSK_WITH_AES_256_CBC_SHA384 = 0x00B3, // [RFC5487] 
        cs_TLS_DHE_PSK_WITH_NULL_SHA256 = 0x00B4, // [RFC5487] 
        cs_TLS_DHE_PSK_WITH_NULL_SHA384 = 0x00B5, // [RFC5487] 
        cs_TLS_RSA_PSK_WITH_AES_128_CBC_SHA256 = 0x00B6, // [RFC5487] 
        cs_TLS_RSA_PSK_WITH_AES_256_CBC_SHA384 = 0x00B7, // [RFC5487] 
        cs_TLS_RSA_PSK_WITH_NULL_SHA256 = 0x00B8, // [RFC5487] 
        cs_TLS_RSA_PSK_WITH_NULL_SHA384 = 0x00B9, // [RFC5487] 
        cs_TLS_RSA_WITH_CAMELLIA_128_CBC_SHA256 = 0x00BA, // [RFC5932] 
        cs_TLS_DH_DSS_WITH_CAMELLIA_128_CBC_SHA256 = 0x00BB, // [RFC5932] 
        cs_TLS_DH_RSA_WITH_CAMELLIA_128_CBC_SHA256 = 0x00BC, // [RFC5932] 
        cs_TLS_DHE_DSS_WITH_CAMELLIA_128_CBC_SHA256 = 0x00BD, // [RFC5932] 
        cs_TLS_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 = 0x00BE, // [RFC5932] 
        cs_TLS_DH_anon_WITH_CAMELLIA_128_CBC_SHA256 = 0x00BF, // [RFC5932] 
        cs_TLS_RSA_WITH_CAMELLIA_256_CBC_SHA256 = 0x00C0, // [RFC5932] 
        cs_TLS_DH_DSS_WITH_CAMELLIA_256_CBC_SHA256 = 0x00C1, // [RFC5932] 
        cs_TLS_DH_RSA_WITH_CAMELLIA_256_CBC_SHA256 = 0x00C2, // [RFC5932] 
        cs_TLS_DHE_DSS_WITH_CAMELLIA_256_CBC_SHA256 = 0x00C3, // [RFC5932] 
        cs_TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256 = 0x00C4, // [RFC5932] 
        cs_TLS_DH_anon_WITH_CAMELLIA_256_CBC_SHA256 = 0x00C5, // [RFC5932] 
        cs_TLS_EMPTY_RENEGOTIATION_INFO_SCSV = 0x00FF, // [RFC5746] 
        cs_TLS_ECDH_ECDSA_WITH_NULL_SHA = 0xC001, // [RFC4492] 
        cs_TLS_ECDH_ECDSA_WITH_RC4_128_SHA = 0xC002, // [RFC4492][RFC6347] 
        cs_TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA = 0xC003, // [RFC4492] 
        cs_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA = 0xC004, // [RFC4492] 
        cs_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA = 0xC005, // [RFC4492] 
        cs_TLS_ECDHE_ECDSA_WITH_NULL_SHA = 0xC006, // [RFC4492] 
        cs_TLS_ECDHE_ECDSA_WITH_RC4_128_SHA = 0xC007, // [RFC4492][RFC6347] 
        cs_TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA = 0xC008, // [RFC4492] 
        cs_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA = 0xC009, // [RFC4492] 
        cs_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA = 0xC00A, // [RFC4492] 
        cs_TLS_ECDH_RSA_WITH_NULL_SHA = 0xC00B, // [RFC4492] 
        cs_TLS_ECDH_RSA_WITH_RC4_128_SHA = 0xC00C, // [RFC4492][RFC6347] 
        cs_TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA = 0xC00D, // [RFC4492] 
        cs_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA = 0xC00E, // [RFC4492] 
        cs_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA = 0xC00F, // [RFC4492] 
        cs_TLS_ECDHE_RSA_WITH_NULL_SHA = 0xC010, // [RFC4492] 
        cs_TLS_ECDHE_RSA_WITH_RC4_128_SHA = 0xC011, // [RFC4492][RFC6347] 
        cs_TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA = 0xC012, // [RFC4492] 
        cs_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA = 0xC013, // [RFC4492] 
        cs_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA = 0xC014, // [RFC4492] 
        cs_TLS_ECDH_anon_WITH_NULL_SHA = 0xC015, // [RFC4492] 
        cs_TLS_ECDH_anon_WITH_RC4_128_SHA = 0xC016, // [RFC4492][RFC6347] 
        cs_TLS_ECDH_anon_WITH_3DES_EDE_CBC_SHA = 0xC017, // [RFC4492] 
        cs_TLS_ECDH_anon_WITH_AES_128_CBC_SHA = 0xC018, // [RFC4492] 
        cs_TLS_ECDH_anon_WITH_AES_256_CBC_SHA = 0xC019, // [RFC4492] 
        cs_TLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA = 0xC01A, // [RFC5054] 
        cs_TLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA = 0xC01B, // [RFC5054] 
        cs_TLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA = 0xC01C, // [RFC5054] 
        cs_TLS_SRP_SHA_WITH_AES_128_CBC_SHA = 0xC01D, // [RFC5054] 
        cs_TLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA = 0xC01E, // [RFC5054] 
        cs_TLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA = 0xC01F, // [RFC5054] 
        cs_TLS_SRP_SHA_WITH_AES_256_CBC_SHA = 0xC020, // [RFC5054] 
        cs_TLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA = 0xC021, // [RFC5054] 
        cs_TLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA = 0xC022, // [RFC5054] 
        cs_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 = 0xC023, // [RFC5289] 
        cs_TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 = 0xC024, // [RFC5289] 
        cs_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256 = 0xC025, // [RFC5289] 
        cs_TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384 = 0xC026, // [RFC5289] 
        cs_TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256 = 0xC027, // [RFC5289] 
        cs_TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384 = 0xC028, // [RFC5289] 
        cs_TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256 = 0xC029, // [RFC5289] 
        cs_TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384 = 0xC02A, // [RFC5289] 
        cs_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256 = 0xC02B, // [RFC5289] 
        cs_TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384 = 0xC02C, // [RFC5289] 
        cs_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 = 0xC02D, // [RFC5289] 
        cs_TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384 = 0xC02E, // [RFC5289] 
        cs_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256 = 0xC02F, // [RFC5289] 
        cs_TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384 = 0xC030, // [RFC5289] 
        cs_TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256 = 0xC031, // [RFC5289] 
        cs_TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384 = 0xC032, // [RFC5289] 
        cs_TLS_ECDHE_PSK_WITH_RC4_128_SHA = 0xC033, // [RFC5489][RFC6347] 
        cs_TLS_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA = 0xC034, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA = 0xC035, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA = 0xC036, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_AES_128_CBC_SHA256 = 0xC037, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_AES_256_CBC_SHA384 = 0xC038, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_NULL_SHA = 0xC039, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_NULL_SHA256 = 0xC03A, // [RFC5489] 
        cs_TLS_ECDHE_PSK_WITH_NULL_SHA384 = 0xC03B, // [RFC5489] 
        cs_TLS_RSA_WITH_ARIA_128_CBC_SHA256 = 0xC03C, // [RFC6209] 
        cs_TLS_RSA_WITH_ARIA_256_CBC_SHA384 = 0xC03D, // [RFC6209] 
        cs_TLS_DH_DSS_WITH_ARIA_128_CBC_SHA256 = 0xC03E, // [RFC6209] 
        cs_TLS_DH_DSS_WITH_ARIA_256_CBC_SHA384 = 0xC03F, // [RFC6209] 
        cs_TLS_DH_RSA_WITH_ARIA_128_CBC_SHA256 = 0xC040, // [RFC6209] 
        cs_TLS_DH_RSA_WITH_ARIA_256_CBC_SHA384 = 0xC041, // [RFC6209] 
        cs_TLS_DHE_DSS_WITH_ARIA_128_CBC_SHA256 = 0xC042, // [RFC6209] 
        cs_TLS_DHE_DSS_WITH_ARIA_256_CBC_SHA384 = 0xC043, // [RFC6209] 
        cs_TLS_DHE_RSA_WITH_ARIA_128_CBC_SHA256 = 0xC044, // [RFC6209] 
        cs_TLS_DHE_RSA_WITH_ARIA_256_CBC_SHA384 = 0xC045, // [RFC6209] 
        cs_TLS_DH_anon_WITH_ARIA_128_CBC_SHA256 = 0xC046, // [RFC6209] 
        cs_TLS_DH_anon_WITH_ARIA_256_CBC_SHA384 = 0xC047, // [RFC6209] 
        cs_TLS_ECDHE_ECDSA_WITH_ARIA_128_CBC_SHA256 = 0xC048, // [RFC6209] 
        cs_TLS_ECDHE_ECDSA_WITH_ARIA_256_CBC_SHA384 = 0xC049, // [RFC6209] 
        cs_TLS_ECDH_ECDSA_WITH_ARIA_128_CBC_SHA256 = 0xC04A, // [RFC6209] 
        cs_TLS_ECDH_ECDSA_WITH_ARIA_256_CBC_SHA384 = 0xC04B, // [RFC6209] 
        cs_TLS_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256 = 0xC04C, // [RFC6209] 
        cs_TLS_ECDHE_RSA_WITH_ARIA_256_CBC_SHA384 = 0xC04D, // [RFC6209] 
        cs_TLS_ECDH_RSA_WITH_ARIA_128_CBC_SHA256 = 0xC04E, // [RFC6209] 
        cs_TLS_ECDH_RSA_WITH_ARIA_256_CBC_SHA384 = 0xC04F, // [RFC6209] 
        cs_TLS_RSA_WITH_ARIA_128_GCM_SHA256 = 0xC050, // [RFC6209] 
        cs_TLS_RSA_WITH_ARIA_256_GCM_SHA384 = 0xC051, // [RFC6209] 
        cs_TLS_DHE_RSA_WITH_ARIA_128_GCM_SHA256 = 0xC052, // [RFC6209] 
        cs_TLS_DHE_RSA_WITH_ARIA_256_GCM_SHA384 = 0xC053, // [RFC6209] 
        cs_TLS_DH_RSA_WITH_ARIA_128_GCM_SHA256 = 0xC054, // [RFC6209] 
        cs_TLS_DH_RSA_WITH_ARIA_256_GCM_SHA384 = 0xC055, // [RFC6209] 
        cs_TLS_DHE_DSS_WITH_ARIA_128_GCM_SHA256 = 0xC056, // [RFC6209] 
        cs_TLS_DHE_DSS_WITH_ARIA_256_GCM_SHA384 = 0xC057, // [RFC6209] 
        cs_TLS_DH_DSS_WITH_ARIA_128_GCM_SHA256 = 0xC058, // [RFC6209] 
        cs_TLS_DH_DSS_WITH_ARIA_256_GCM_SHA384 = 0xC059, // [RFC6209] 
        cs_TLS_DH_anon_WITH_ARIA_128_GCM_SHA256 = 0xC05A, // [RFC6209] 
        cs_TLS_DH_anon_WITH_ARIA_256_GCM_SHA384 = 0xC05B, // [RFC6209] 
        cs_TLS_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256 = 0xC05C, // [RFC6209] 
        cs_TLS_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384 = 0xC05D, // [RFC6209] 
        cs_TLS_ECDH_ECDSA_WITH_ARIA_128_GCM_SHA256 = 0xC05E, // [RFC6209] 
        cs_TLS_ECDH_ECDSA_WITH_ARIA_256_GCM_SHA384 = 0xC05F, // [RFC6209] 
        cs_TLS_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256 = 0xC060, // [RFC6209] 
        cs_TLS_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384 = 0xC061, // [RFC6209] 
        cs_TLS_ECDH_RSA_WITH_ARIA_128_GCM_SHA256 = 0xC062, // [RFC6209] 
        cs_TLS_ECDH_RSA_WITH_ARIA_256_GCM_SHA384 = 0xC063, // [RFC6209] 
        cs_TLS_PSK_WITH_ARIA_128_CBC_SHA256 = 0xC064, // [RFC6209] 
        cs_TLS_PSK_WITH_ARIA_256_CBC_SHA384 = 0xC065, // [RFC6209] 
        cs_TLS_DHE_PSK_WITH_ARIA_128_CBC_SHA256 = 0xC066, // [RFC6209] 
        cs_TLS_DHE_PSK_WITH_ARIA_256_CBC_SHA384 = 0xC067, // [RFC6209] 
        cs_TLS_RSA_PSK_WITH_ARIA_128_CBC_SHA256 = 0xC068, // [RFC6209] 
        cs_TLS_RSA_PSK_WITH_ARIA_256_CBC_SHA384 = 0xC069, // [RFC6209] 
        cs_TLS_PSK_WITH_ARIA_128_GCM_SHA256 = 0xC06A, // [RFC6209] 
        cs_TLS_PSK_WITH_ARIA_256_GCM_SHA384 = 0xC06B, // [RFC6209] 
        cs_TLS_DHE_PSK_WITH_ARIA_128_GCM_SHA256 = 0xC06C, // [RFC6209] 
        cs_TLS_DHE_PSK_WITH_ARIA_256_GCM_SHA384 = 0xC06D, // [RFC6209] 
        cs_TLS_RSA_PSK_WITH_ARIA_128_GCM_SHA256 = 0xC06E, // [RFC6209] 
        cs_TLS_RSA_PSK_WITH_ARIA_256_GCM_SHA384 = 0xC06F, // [RFC6209] 
        cs_TLS_ECDHE_PSK_WITH_ARIA_128_CBC_SHA256 = 0xC070, // [RFC6209] 
        cs_TLS_ECDHE_PSK_WITH_ARIA_256_CBC_SHA384 = 0xC071, // [RFC6209] 
        cs_TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 = 0xC072, // [RFC6367] 
        cs_TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 = 0xC073, // [RFC6367] 
        cs_TLS_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 = 0xC074, // [RFC6367] 
        cs_TLS_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 = 0xC075, // [RFC6367] 
        cs_TLS_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 = 0xC076, // [RFC6367] 
        cs_TLS_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384 = 0xC077, // [RFC6367] 
        cs_TLS_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256 = 0xC078, // [RFC6367] 
        cs_TLS_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384 = 0xC079, // [RFC6367] 
        cs_TLS_RSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC07A, // [RFC6367] 
        cs_TLS_RSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC07B, // [RFC6367] 
        cs_TLS_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC07C, // [RFC6367] 
        cs_TLS_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC07D, // [RFC6367] 
        cs_TLS_DH_RSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC07E, // [RFC6367] 
        cs_TLS_DH_RSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC07F, // [RFC6367] 
        cs_TLS_DHE_DSS_WITH_CAMELLIA_128_GCM_SHA256 = 0xC080, // [RFC6367] 
        cs_TLS_DHE_DSS_WITH_CAMELLIA_256_GCM_SHA384 = 0xC081, // [RFC6367] 
        cs_TLS_DH_DSS_WITH_CAMELLIA_128_GCM_SHA256 = 0xC082, // [RFC6367] 
        cs_TLS_DH_DSS_WITH_CAMELLIA_256_GCM_SHA384 = 0xC083, // [RFC6367] 
        cs_TLS_DH_anon_WITH_CAMELLIA_128_GCM_SHA256 = 0xC084, // [RFC6367] 
        cs_TLS_DH_anon_WITH_CAMELLIA_256_GCM_SHA384 = 0xC085, // [RFC6367] 
        cs_TLS_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC086, // [RFC6367] 
        cs_TLS_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC087, // [RFC6367] 
        cs_TLS_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC088, // [RFC6367] 
        cs_TLS_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC089, // [RFC6367] 
        cs_TLS_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC08A, // [RFC6367] 
        cs_TLS_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC08B, // [RFC6367] 
        cs_TLS_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256 = 0xC08C, // [RFC6367] 
        cs_TLS_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384 = 0xC08D, // [RFC6367] 
        cs_TLS_PSK_WITH_CAMELLIA_128_GCM_SHA256 = 0xC08E, // [RFC6367] 
        cs_TLS_PSK_WITH_CAMELLIA_256_GCM_SHA384 = 0xC08F, // [RFC6367] 
        cs_TLS_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256 = 0xC090, // [RFC6367] 
        cs_TLS_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384 = 0xC091, // [RFC6367] 
        cs_TLS_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256 = 0xC092, // [RFC6367] 
        cs_TLS_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384 = 0xC093, // [RFC6367] 
        cs_TLS_PSK_WITH_CAMELLIA_128_CBC_SHA256 = 0xC094, // [RFC6367] 
        cs_TLS_PSK_WITH_CAMELLIA_256_CBC_SHA384 = 0xC095, // [RFC6367] 
        cs_TLS_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 = 0xC096, // [RFC6367] 
        cs_TLS_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 = 0xC097, // [RFC6367] 
        cs_TLS_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256 = 0xC098, // [RFC6367] 
        cs_TLS_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384 = 0xC099, // [RFC6367] 
        cs_TLS_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 = 0xC09A, // [RFC6367] 
        cs_TLS_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 = 0xC09B, // [RFC6367] 
        cs_TLS_RSA_WITH_AES_128_CCM = 0xC09C, // [RFC6655] 
        cs_TLS_RSA_WITH_AES_256_CCM = 0xC09D, // [RFC6655] 
        cs_TLS_DHE_RSA_WITH_AES_128_CCM = 0xC09E, // [RFC6655] 
        cs_TLS_DHE_RSA_WITH_AES_256_CCM = 0xC09F, // [RFC6655] 
        cs_TLS_RSA_WITH_AES_128_CCM_8 = 0xC0A0, // [RFC6655] 
        cs_TLS_RSA_WITH_AES_256_CCM_8 = 0xC0A1, // [RFC6655] 
        cs_TLS_DHE_RSA_WITH_AES_128_CCM_8 = 0xC0A2, // [RFC6655] 
        cs_TLS_DHE_RSA_WITH_AES_256_CCM_8 = 0xC0A3, // [RFC6655] 
        cs_TLS_PSK_WITH_AES_128_CCM = 0xC0A4, // [RFC6655] 
        cs_TLS_PSK_WITH_AES_256_CCM = 0xC0A5, // [RFC6655] 
        cs_TLS_DHE_PSK_WITH_AES_128_CCM = 0xC0A6, // [RFC6655] 
        cs_TLS_DHE_PSK_WITH_AES_256_CCM = 0xC0A7, // [RFC6655] 
        cs_TLS_PSK_WITH_AES_128_CCM_8 = 0xC0A8, // [RFC6655] 
        cs_TLS_PSK_WITH_AES_256_CCM_8 = 0xC0A9, // [RFC6655] 
        cs_TLS_PSK_DHE_WITH_AES_128_CCM_8 = 0xC0AA, // [RFC6655] 
        cs_TLS_PSK_DHE_WITH_AES_256_CCM_8 = 0xC0AB, // [RFC6655] 
    };

    enum NamedCurve : uint16
    {
        sect163k1 = 1,
        sect163r1 = 2,
        sect163r2 = 3,
        sect193r1 = 4,
        sect193r2 = 5,
        sect233k1 = 6,
        sect233r1 = 7,
        sect239k1 = 8,
        sect283k1 = 9,
        sect283r1 = 10,
        sect409k1 = 11,
        sect409r1 = 12,
        sect571k1 = 13,
        sect571r1 = 14,
        secp160k1 = 15,
        secp160r1 = 16,
        secp160r2 = 17,
        secp192k1 = 18,
        secp192r1 = 19,
        secp224k1 = 20,
        secp224r1 = 21,
        secp256k1 = 22,
        secp256r1 = 23,
        secp384r1 = 24,
        secp521r1 = 25,
        arbitrary_explicit_prime_curves = 0xFF01,
        arbitrary_explicit_char2_curves = 0xFF02,
    };

    enum ECPointFormat : uint8
    {
        uncompressed = 0,
        ansiX962_compressed_prime = 1,
        ansiX962_compressed_char2 = 2,
    };

    enum EventType
    {
        change_cipher_spec_event = 0x2000,
    };

    ///////////////////////////////////////////////////////////////////////////
    // enum vectors
    ///////////////////////////////////////////////////////////////////////////

    typedef Vector<CompressionMethod, 1, 0xff, 1> CompressionMethods;
    typedef Vector<CipherSuite, 2, 0xfffe, 2> CipherSuites;
    typedef Vector<NamedCurve, 1, 0xffff, 2> EllipticCurveList;
    typedef Vector<ECPointFormat, 1, 0xff, 1> ECPointFormatList;

    ///////////////////////////////////////////////////////////////////////////
    // static const
    ///////////////////////////////////////////////////////////////////////////

    static const uint8 ChangeCipherSpec = 1;

    ///////////////////////////////////////////////////////////////////////////
    // basic typedefs
    ///////////////////////////////////////////////////////////////////////////

    typedef uint16 ProtocolVersion;

    ///////////////////////////////////////////////////////////////////////////
    // basic vectors
    ///////////////////////////////////////////////////////////////////////////

    typedef Vector<byte, 1, 0xffff, 2> ResponderID;
    typedef Vector<byte, 1, 0xffffff, 3> Certificate;
    typedef Vector<byte, 0, 0x20, 1> SessionId;
    typedef Vector<byte, 1, 0xffff, 2> HostName;
    typedef Vector<byte, 0, 0xff, 1> RenegotiationInfo;
    typedef Vector<byte, 0, 0xffffff, 2> Extensions;
    typedef Vector<byte, 0, 0xffff, 2> EncryptedPreMasterSecret;
    typedef Vector<byte, 1, 0xffff, 2> DhP;
    typedef Vector<byte, 1, 0xffff, 2> DhG;
    typedef Vector<byte, 1, 0xffff, 2> DhYs;

    ///////////////////////////////////////////////////////////////////////////
    // vectors of vectors
    ///////////////////////////////////////////////////////////////////////////

    typedef Vector<ResponderID, 0, 0xffff, 2> ResponderIDList;
    typedef Vector<Certificate, 0, 0xffffff, 3> Certificates;

    ///////////////////////////////////////////////////////////////////////////
    // complex types
    ///////////////////////////////////////////////////////////////////////////

    struct Random
    {
        uint32 gmt_unix_time;
        byte random_bytes[28];
    };

    struct SignatureAndHashAlgorithm
    {
        HashAlgorithm hash;
        SignatureAlgorithm signature;
    };

    struct ServerName
    {
        NameType name_type;
        HostName name;
    };

    struct Record
    {
        ContentType type;
        ProtocolVersion version;
        uint16 length;
        std::shared_ptr<ByteString> fragment;
    };

    struct Handshake
    {
        HandshakeType msg_type;
        uint32 length;
    };

    struct ExtensionHeader
    {
        ExtensionType type;
        uint16 length;
    };

    enum CertificateStatusType : uint8
    {
        ocsp = 1,
    };

    struct OCSPStatusRequest
    {
        ResponderIDList responder_id_list;
        Extensions request_extensions;
    };

    struct CertificateStatusRequest
    {
        CertificateStatusType status_type;
        OCSPStatusRequest ocsp_status_request;
    };

    struct HeartbeatExtension
    {
        HeartbeatMode mode;
    };

    struct PreMasterSecret
    {
        ProtocolVersion client_version;
        byte random[46];
    };

    struct Alert
    {
        AlertLevel level;
        AlertDescription description;
    };

    struct ChangeCipherSpecEvent : public Basic::IEvent
    {
        virtual uint32 get_type();
    };

    struct HeartbeatMessage
    {
        HeartbeatMessageType type;
        uint16 payload_length;
        ByteString payload;
        ByteString padding;
    };

    struct ServerDHParams
    {
        DhP dh_p;
        DhG dh_g;
        DhYs dh_Ys;
    };

    ///////////////////////////////////////////////////////////////////////////
    // vectors of complex types
    ///////////////////////////////////////////////////////////////////////////

    typedef Vector<SignatureAndHashAlgorithm, 2, 0xfffe, 2> SignatureAndHashAlgorithms;
    typedef Vector<ServerName, 1, 0xffff, 2> ServerNameList;

    ///////////////////////////////////////////////////////////////////////////
    // complex types with vectors of complex types
    ///////////////////////////////////////////////////////////////////////////

    struct ClientHello
    {
        ProtocolVersion client_version;
        Random random;
        SessionId session_id;
        CipherSuites cipher_suites;
        CompressionMethods compression_methods;
        ServerNameList server_name_list;
        SignatureAndHashAlgorithms supported_signature_algorithms;
        RenegotiationInfo renegotiation_info;
        CertificateStatusRequest certificate_status_request;
        EllipticCurveList elliptic_curve_list;
        ECPointFormatList ec_point_format_list;
        HeartbeatExtension heartbeat_extension;
        bool heartbeat_extension_initialized;
    };

    struct ServerHello
    {
        ProtocolVersion server_version;
        Random random;
        SessionId session_id;
        CipherSuite cipher_suite;
        CompressionMethod compression_method;
        HeartbeatExtension heartbeat_extension;
        bool heartbeat_extension_initialized;
    };

    ///////////////////////////////////////////////////////////////////////////
    // serialization meta template
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) serialize
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
        	static_assert(false, "No Tls::serialize defined for this type");
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // memory range serialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type, int value_size = sizeof(value_type)>
    struct __declspec(novtable) serialize_memory
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
            stream->write_elements((byte*)value, value_size);
        }
    };

    template <> struct __declspec(novtable) serialize<byte> : public serialize_memory<byte> {};
    template <> struct __declspec(novtable) serialize<CompressionMethod> : public serialize_memory<CompressionMethod> {};
    template <> struct __declspec(novtable) serialize<ContentType> : public serialize_memory<ContentType> {};
    template <> struct __declspec(novtable) serialize<AlertLevel> : public serialize_memory<AlertLevel> {};
    template <> struct __declspec(novtable) serialize<AlertDescription> : public serialize_memory<AlertDescription> {};
    template <> struct __declspec(novtable) serialize<HeartbeatMode> : public serialize_memory<HeartbeatMode> {};
    template <> struct __declspec(novtable) serialize<HandshakeType> : public serialize_memory<HandshakeType> {};
    template <> struct __declspec(novtable) serialize<HeartbeatMessageType> : public serialize_memory<HeartbeatMessageType> {};
    template <> struct __declspec(novtable) serialize<NameType> : public serialize_memory<NameType> {};
    template <> struct __declspec(novtable) serialize<SignatureAlgorithm> : public serialize_memory<SignatureAlgorithm> {};
    template <> struct __declspec(novtable) serialize<HashAlgorithm> : public serialize_memory<HashAlgorithm> {};

    ///////////////////////////////////////////////////////////////////////////
    // array serialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) serialize_array
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
            stream->write_elements((byte*)(*value), sizeof(value_type));
        }
    };

    template <> struct __declspec(novtable) serialize<const byte[28]> : public serialize_array<const byte[28]> {};
    template <> struct __declspec(novtable) serialize<const byte[46]> : public serialize_array<const byte[46]> {};

    ///////////////////////////////////////////////////////////////////////////
    // byte vector serialization
    ///////////////////////////////////////////////////////////////////////////

    template <>
    struct __declspec(novtable) serialize<std::vector<byte> >
    {
        void operator()(const std::vector<byte>* value, IStream<byte>* stream) const
        {
            stream->write_elements(&*value->begin(), value->size());
        }
    };

    template <>
    struct __declspec(novtable) serialize<ByteString>
    {
        void operator()(const ByteString* value, IStream<byte>* stream) const
        {
            value->write_to_stream(stream);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // number serialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type, int encoded_length = sizeof(value_type)>
    struct __declspec(novtable) serialize_number
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
            byte* value_bytes = (byte*)value;
            int most_significant = sizeof(value_type) - 1;
            int overflow = 0;

            if (encoded_length < sizeof(value_type))
            {
                overflow = sizeof(value_type) - encoded_length;

                for (int i = 0; i < overflow; i++)
                {
                    if (value_bytes[most_significant - i] != 0)
                        throw FatalError("Tls::NumberFrame::write_to_stream overflow");
                }
            }
            else
            {
                int padding = encoded_length - sizeof(value_type);

                for (int i = 0; i < padding; i++)
                {
                    stream->write_element(0);
                }
            }

            for (int i = overflow; i < sizeof(value_type); i++)
            {
                stream->write_element(value_bytes[most_significant - i]);
            }
        }
    };

    template <> struct __declspec(novtable) serialize<uint16> : public serialize_number<uint16> {};
    template <> struct __declspec(novtable) serialize<uint32> : public serialize_number<uint32> {};
    template <> struct __declspec(novtable) serialize<uint64> : public serialize_number<uint64> {};
    template <> struct __declspec(novtable) serialize<ExtensionType> : public serialize_number<ExtensionType> {};
    template <> struct __declspec(novtable) serialize<CipherSuite> : public serialize_number<CipherSuite> {};

    ///////////////////////////////////////////////////////////////////////////
    // tls vector serialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename vector_type>
    struct __declspec(novtable) serialize_vector
    {
        static void serialize_items(const vector_type* value, IStream<byte>* stream)
        {
            for (vector_type::const_iterator it = value->cbegin(); it != value->cend(); it++)
            {
                serialize<vector_type::element_type>()(&(*it), stream);
            }
        }

        void operator()(const vector_type* value, IStream<byte>* stream) const
        {
            CountStream<byte> count_stream;
            serialize_items(value, &count_stream);

            if (count_stream.count < vector_type::encoded_length_min)
                throw FatalError("vector too short");

            if (count_stream.count > vector_type::encoded_length_max)
                throw FatalError("vector too long");

            serialize_number<uint32, vector_type::length_of_encoded_length>()(&count_stream.count, stream);
            serialize_items(value, stream);
        }
    };

    template <> struct __declspec(novtable) serialize<CompressionMethods> : public serialize_vector<CompressionMethods> {};
    template <> struct __declspec(novtable) serialize<CipherSuites> : public serialize_vector<CipherSuites> {};
    template <> struct __declspec(novtable) serialize<ResponderID> : public serialize_vector<ResponderID> {};
    template <> struct __declspec(novtable) serialize<ResponderIDList> : public serialize_vector<ResponderIDList> {};
    template <> struct __declspec(novtable) serialize<Certificate> : public serialize_vector<Certificate> {};
    template <> struct __declspec(novtable) serialize<Certificates> : public serialize_vector<Certificates> {};
    template <> struct __declspec(novtable) serialize<SessionId> : public serialize_vector<SessionId> {};
    template <> struct __declspec(novtable) serialize<RenegotiationInfo> : public serialize_vector<RenegotiationInfo> {};
    template <> struct __declspec(novtable) serialize<Extensions> : public serialize_vector<Extensions> {};
    template <> struct __declspec(novtable) serialize<EncryptedPreMasterSecret> : public serialize_vector<EncryptedPreMasterSecret> {};
    template <> struct __declspec(novtable) serialize<SignatureAndHashAlgorithms> : public serialize_vector<SignatureAndHashAlgorithms> {};
    template <> struct __declspec(novtable) serialize<ServerNameList> : public serialize_vector<ServerNameList> {};
    template <> struct __declspec(novtable) serialize<EllipticCurveList> : public serialize_vector<EllipticCurveList> {};
    template <> struct __declspec(novtable) serialize<ECPointFormatList> : public serialize_vector<ECPointFormatList> {};

    ///////////////////////////////////////////////////////////////////////////
    // serializer object
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    class Serializer : public IStreamWriter<byte>
    {
    private:
        value_type* value;

    public:
        Serializer(value_type* value)
        {
            this->value = value;
        }

        virtual void IStreamWriter<byte>::write_to_stream(IStream<byte>* stream) const
        {
            serialize<value_type>()(value, stream);
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // deserialization meta template
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) make_deserializer
    {
        void operator()(value_type* value, std::shared_ptr<IProcess>* deserializer) const
        {
        	static_assert(false, "No Tls::make_deserializer defined for this type");
        }
    };

    template <typename value_type, typename frame_type>
    struct __declspec(novtable) make_frame_deserializer
    {
        void operator()(value_type* value, std::shared_ptr<IProcess>* deserializer) const
        {
            std::shared_ptr<frame_type> frame = std::make_shared<frame_type>(value);
            (*deserializer) = frame;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // memory range deserialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) make_memory_deserializer
    {
        void operator()(value_type* value, std::shared_ptr<IProcess>* deserializer) const
        {
            std::shared_ptr<MemoryRange> memory_range = std::make_shared<MemoryRange>((byte*)value, sizeof(value_type));
            (*deserializer) = memory_range;
        }
    };

    template <> struct __declspec(novtable) make_deserializer<byte> : public make_memory_deserializer<byte> {};
    template <> struct __declspec(novtable) make_deserializer<CompressionMethod> : public make_memory_deserializer<CompressionMethod> {};
    template <> struct __declspec(novtable) make_deserializer<ECPointFormat> : public make_memory_deserializer<ECPointFormat> {};

    ///////////////////////////////////////////////////////////////////////////
    // number deserialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type, int encoded_length = sizeof(value_type)>
    class NumberFrame : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            receiving_state,
            done_state = Succeeded_State,
        };

        uint32 received;
        value_type* value;

    public:
        NumberFrame(value_type* value)
        {
            this->received = 0;
            this->value = value;
        }

        void reset()
        {
            __super::reset();
            this->received = 0;
        }

        virtual event_result IProcess::consider_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::start_state:
                ZeroMemory(this->value, sizeof(value_type));
                switch_to_state(State::receiving_state);
                break;

            case State::receiving_state:
                {
                    byte b;
                    event_result result = Event::ReadNext(event, &b);
                    if (result == event_result_yield)
                        return event_result_yield;

                    byte* value_bytes = reinterpret_cast<byte*>(this->value);
                    int index = encoded_length - this->received - 1;
                    value_bytes[index] = b;

                    this->received++;

                    if (this->received == encoded_length)
                        switch_to_state(State::done_state);
                }
                break;

            default:
                throw FatalError("Tls::NumberFrame::handle_event unexpected state");
            }

            return event_result_continue;
        }
    };

    template <typename value_type, int encoded_length = sizeof(value_type)>
    struct __declspec(novtable) make_number_deserializer : public make_frame_deserializer<value_type, NumberFrame<value_type, encoded_length> > {};

    template <> struct __declspec(novtable) make_deserializer<uint16> : public make_number_deserializer<uint16> {};
    template <> struct __declspec(novtable) make_deserializer<uint32> : public make_number_deserializer<uint32> {};
    template <> struct __declspec(novtable) make_deserializer<uint64> : public make_number_deserializer<uint64> {};
    template <> struct __declspec(novtable) make_deserializer<CipherSuite> : public make_number_deserializer<CipherSuite> {};
    template <> struct __declspec(novtable) make_deserializer<NamedCurve> : public make_number_deserializer<NamedCurve> {};

    ///////////////////////////////////////////////////////////////////////////
    // tls vector deserialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename vector_type>
    class VectorFrame : public Frame
    {
    private:
        enum State
        {
            length_frame_pending_state = Start_State,
            length_known_state,
            next_item_state,
            item_pending_state,
            items_received_state,
            done_state = Succeeded_State,
            item_frame_failed,
            length_mismatch_error,
            vector_complete_failed,
            length_frame_failed,
            unexpected_length_error,
        };

        uint32 encoded_length;
        std::shared_ptr<CountStream<byte> > counter;
        NumberFrame<uint32, vector_type::length_of_encoded_length> length_frame;
        std::shared_ptr<IProcess> item_frame;

    protected:
        void switch_to_state(IEvent* event, State state)
        {
            __super::switch_to_state(state);

            if (!this->in_progress())
                Event::RemoveObserver<byte>(event, this->counter);
        }

    public:
        vector_type* items;

        VectorFrame(vector_type* items) :
            items(items),
            length_frame(&this->encoded_length) // initialization is in order of declaration in class def
        {
        }

        virtual event_result IProcess::consider_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::length_frame_pending_state:
                {
                    event_result result = delegate_event_change_state_on_fail(&this->length_frame, event, State::length_frame_failed);
                    if (result == event_result_yield)
                        return event_result_yield;

                    switch_to_state(event, State::length_known_state);
                }
                break;

            case State::length_known_state:
                if (!(this->encoded_length >= vector_type::encoded_length_min && this->encoded_length <= vector_type::encoded_length_max))
                {
                    switch_to_state(event, State::unexpected_length_error);
                    return event_result_continue;
                }

                this->counter = std::make_shared<CountStream<byte> >();
                Event::AddObserver<byte>(event, this->counter);

                this->items->clear();

                // since each item must be at least 1 byte big, may as well
                // reserve at that much.  in the case where sizeof(vector_type::element_type) == 1
                // this should be all the space ever needed
                this->items->reserve(this->encoded_length);

                switch_to_state(event, State::next_item_state);
                break;

            case State::next_item_state:
                {
                    uint32 received = this->counter->count;

                    if (received > this->encoded_length)
                    {
                        switch_to_state(event, State::length_mismatch_error);
                    }
                    else if (received == this->encoded_length)
                    {
                        switch_to_state(event, State::items_received_state);
                    }
                    else
                    {
                        this->items->push_back(vector_type::element_type());

                        make_deserializer<vector_type::element_type>()(&this->items->back(), &this->item_frame);

                        switch_to_state(event, State::item_pending_state);
                    }
                }
                break;

            case State::item_pending_state:
                {
                    event_result result = delegate_event_change_state_on_fail(this->item_frame.get(), event, State::item_frame_failed);
                    if (result == event_result_yield)
                        return event_result_yield;

                    switch_to_state(event, State::next_item_state);
                }
                break;

            case State::items_received_state:
                {
                    switch_to_state(event, State::done_state);
                }
                break;

            default:
                throw FatalError("Tls::VectorFrame unexpected state");
            }

            return event_result_continue;
        }
    };

    template <typename vector_type>
    struct __declspec(novtable) make_vector_deserializer : public make_frame_deserializer<vector_type, VectorFrame<vector_type> > {};

    template <> struct __declspec(novtable) make_deserializer<CompressionMethods> : public make_vector_deserializer<CompressionMethods> {};
    template <> struct __declspec(novtable) make_deserializer<CipherSuites> : public make_vector_deserializer<CipherSuites> {};
    template <> struct __declspec(novtable) make_deserializer<ResponderID> : public make_vector_deserializer<ResponderID> {};
    template <> struct __declspec(novtable) make_deserializer<ResponderIDList> : public make_vector_deserializer<ResponderIDList> {};
    template <> struct __declspec(novtable) make_deserializer<Certificate> : public make_vector_deserializer<Certificate> {};
    template <> struct __declspec(novtable) make_deserializer<Certificates> : public make_vector_deserializer<Certificates> {};
    template <> struct __declspec(novtable) make_deserializer<SessionId> : public make_vector_deserializer<SessionId> {};
    template <> struct __declspec(novtable) make_deserializer<RenegotiationInfo> : public make_vector_deserializer<RenegotiationInfo> {};
    template <> struct __declspec(novtable) make_deserializer<Extensions> : public make_vector_deserializer<Extensions> {};
    template <> struct __declspec(novtable) make_deserializer<EncryptedPreMasterSecret> : public make_vector_deserializer<EncryptedPreMasterSecret> {};
    template <> struct __declspec(novtable) make_deserializer<SignatureAndHashAlgorithms> : public make_vector_deserializer<SignatureAndHashAlgorithms> {};
    template <> struct __declspec(novtable) make_deserializer<ServerNameList> : public make_vector_deserializer<ServerNameList> {};
    template <> struct __declspec(novtable) make_deserializer<EllipticCurveList> : public make_vector_deserializer<EllipticCurveList> {};
    template <> struct __declspec(novtable) make_deserializer<ECPointFormatList> : public make_vector_deserializer<ECPointFormatList> {};
}