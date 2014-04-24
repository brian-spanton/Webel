// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ByteVector.h"
#include "Basic.Event.h"

namespace Tls
{
    using namespace Basic;

    typedef byte opaque;

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

    enum ContentType : byte
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

    typedef std::vector<opaque> OpaqueVector;
    typedef uint16 ProtocolVersion;

    struct Random
    {
        uint32 gmt_unix_time;
        opaque random_bytes[28];
    };

    struct SignatureAndHashAlgorithm
    {
        HashAlgorithm hash;
        SignatureAlgorithm signature;
    };

    struct ServerName
    {
        NameType name_type;
        std::vector<opaque> name;
    };

    typedef std::vector<CompressionMethod> CompressionMethods;
    typedef std::vector<SignatureAndHashAlgorithm> SignatureAndHashAlgorithms;
    typedef std::vector<CipherSuite> CipherSuites;
    typedef std::vector<OpaqueVector> Certificates;
    typedef std::vector<ServerName> ServerNameList;

    struct Record
    {
        ContentType type;
        ProtocolVersion version;
        uint16 length;
        ByteVector::Ref fragment; // REF
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

    typedef std::vector<OpaqueVector> ResponderIDList;

    struct OCSPStatusRequest
    {
        ResponderIDList responder_id_list;
        std::vector<opaque> request_extensions;
    };

    struct CertificateStatusRequest
    {
        CertificateStatusType status_type;
        OCSPStatusRequest ocsp_status_request;
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

    typedef std::vector<NamedCurve> EllipticCurveList;

    enum ECPointFormat : uint8
    {
        uncompressed = 0,
        ansiX962_compressed_prime = 1,
        ansiX962_compressed_char2 = 2,
    };

    typedef std::vector<ECPointFormat> ECPointFormatList;

    struct HeartbeatExtension
    {
        HeartbeatMode mode;
    };

    struct ClientHello
    {
        ProtocolVersion client_version;
        Random random;
        std::vector<opaque> session_id;
        CipherSuites cipher_suites;
        CompressionMethods compression_methods;
        ServerNameList server_name_list;
        SignatureAndHashAlgorithms supported_signature_algorithms;
        std::vector<opaque> renegotiation_info;
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
        std::vector<opaque> session_id;
        CipherSuite cipher_suite;
        CompressionMethod compression_method;
        HeartbeatExtension heartbeat_extension;
        bool heartbeat_extension_initialized;
    };

    struct PreMasterSecret
    {
        ProtocolVersion client_version;
        opaque random[46];
    };

    static const uint8 ChangeCipherSpec = 1;

    struct Alert
    {
        AlertLevel level;
        AlertDescription description;
    };

    enum EventType
    {
        change_cipher_spec_event = 0x2000,
    };

    struct ChangeCipherSpecEvent : public Basic::IEvent
    {
        virtual uint32 get_type();
    };

    struct HeartbeatMessage
    {
        HeartbeatMessageType type;
        uint16 payload_length;
        std::vector<opaque> payload;
        std::vector<opaque> padding;
    };

    struct ServerDHParams
    {
        std::vector<opaque> dh_p;
        std::vector<opaque> dh_g;
        std::vector<opaque> dh_Ys;
    };
}
