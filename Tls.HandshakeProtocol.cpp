// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HandshakeProtocol.h"
#include "Basic.CountStream.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Basic.Cng.h"
#include "Tls.RecordLayer.h"
#include "Tls.HandshakeFrame.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.RandomFrame.h"
#include "Tls.Globals.h"
#include "Tls.RecordLayer.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

namespace Tls
{
    using namespace Basic;

    HandshakeProtocol::HandshakeProtocol(RecordLayer* session) :
        security_parameters(std::make_shared<SecurityParameters>()),
        handshake_messages(std::make_shared<ByteString>()),
        session(session),
        handshake_frame(&this->handshake), // initialization is in order of declaration in class def
        send_buffer(std::make_shared<ByteString>())
    {
        // $$ find a way to optimize this - directly write into the hash stream instead?
        this->handshake_messages->reserve(0x1000);
    }

    void HandshakeProtocol::switch_to_state(uint32 state)
    {
        __super::switch_to_state(state);

        if (succeeded())
        {
            this->session->ConnectApplication();
        }
    }

    void HandshakeProtocol::CalculateVerifyData(ByteString* label, ByteString* output)
    {
        IStreamWriter<byte>* seed[] = { this->handshake_messages.get(), };
        std::vector<std::shared_ptr<ByteString> > prf_seed;

        switch (this->session->version)
        {
        case 0x0301:
        case 0x0302:
            {
                std::shared_ptr<Basic::HashAlgorithm> hashAlgorithm = std::make_shared<Basic::HashAlgorithm>();
                hashAlgorithm->Initialize(BCRYPT_MD5_ALGORITHM, false);

                std::shared_ptr<ByteString> md5 = std::make_shared<ByteString>();
                md5->resize(hashAlgorithm->hash_output_length);
                Tls::globals->Hash(hashAlgorithm.get(), 0, 0, seed, _countof(seed), md5->address(), md5->size());

                hashAlgorithm = std::make_shared<Basic::HashAlgorithm>();
                hashAlgorithm->Initialize(BCRYPT_SHA1_ALGORITHM, false);

                std::shared_ptr<ByteString> sha1 = std::make_shared<ByteString>();
                sha1->resize(hashAlgorithm->hash_output_length);
                Tls::globals->Hash(hashAlgorithm.get(), 0, 0, seed, _countof(seed), sha1->address(), sha1->size());

                prf_seed.push_back(md5);
                prf_seed.push_back(sha1);
            }
            break;

        case 0x0303:
            {
                std::shared_ptr<Basic::HashAlgorithm> hashAlgorithm = std::make_shared<Basic::HashAlgorithm>();
                hashAlgorithm->Initialize(BCRYPT_SHA256_ALGORITHM, false);

                std::shared_ptr<ByteString> sha256 = std::make_shared<ByteString>();
                sha256->resize(hashAlgorithm->hash_output_length);
                Tls::globals->Hash(hashAlgorithm.get(), 0, 0, seed, _countof(seed), sha256->address(), sha256->size());

                prf_seed.push_back(sha256);
            }
            break;

        default:
            throw FatalError("Tls", "HandshakeProtocol", "CalculateVerifyData", "unsupported version", this->session->version);
        }

        output->resize(this->security_parameters->verify_data_length);

        std::vector<IStreamWriter<byte>*> prf_seed_ptrs;
        for (auto ptr : prf_seed)
            prf_seed_ptrs.push_back(ptr.get());

        Tls::globals->PRF(
            this->security_parameters->prf_algorithm,
            &this->security_parameters->master_secret,
            label,
            &*prf_seed_ptrs.begin(),
            prf_seed_ptrs.size(),
            output->address(),
            output->size());
    }

    bool HandshakeProtocol::WriteFinished(ByteString* label)
    {
        // http://www.ietf.org/mail-archive/web/tls/current/msg09221.html

        std::shared_ptr<ByteString> verify_data = std::make_shared<ByteString>();
        verify_data->resize(this->security_parameters->verify_data_length);

        CalculateVerifyData(label, verify_data.get());

        // send the handshake buffer so far, before sending change cipher spec

        if (this->send_buffer->size() > 0)
        {
            ByteStringRef send_buffer = std::make_shared<ByteString>();
            send_buffer.swap(this->send_buffer);

            this->session->write_record_elements(ContentType::handshake, send_buffer->address(), send_buffer->size());
        }

        this->session->WriteChangeCipherSpec();

        bool success = WriteMessage(HandshakeType::finished, verify_data.get());
        if (!success)
            return false;

        return true;
    }

    bool HandshakeProtocol::WriteMessage(HandshakeType msg_type, IStreamWriter<byte>* frame)
    {
        Handshake handshake;
        handshake.msg_type = msg_type;
        handshake.length = 0;

        if (frame != 0)
        {
            std::shared_ptr<CountStream<byte> > count_stream = std::make_shared<CountStream<byte> >();
            frame->write_to_stream(count_stream.get());
            handshake.length = count_stream->count;
        }

        serialize<Handshake>()(&handshake, this->handshake_messages.get());
        serialize<Handshake>()(&handshake, this->send_buffer.get());

        if (frame != 0)
        {
            frame->write_to_stream(this->handshake_messages.get());
            frame->write_to_stream(this->send_buffer.get());
        }

        return true;
    }

    void HandshakeProtocol::CalculateKeys(IVector<byte>* pre_master_secret)
    {
        Serializer<Random> client_random_serializer(&this->security_parameters->client_random);
        Serializer<Random> server_random_serializer(&this->security_parameters->server_random);

        IStreamWriter<byte>* masterSecretSeed[] = { &client_random_serializer, &server_random_serializer, };

        this->security_parameters->master_secret.resize(Tls::globals->master_secret_length);

        Tls::globals->PRF(
            this->security_parameters->prf_algorithm,
            pre_master_secret,
            &Tls::globals->master_secret_label,
            masterSecretSeed,
            _countof(masterSecretSeed),
            this->security_parameters->master_secret.address(),
            this->security_parameters->master_secret.size());

        IStreamWriter<byte>* keyExpansionSeed[] = { &server_random_serializer, &client_random_serializer, };

        PartitionKeyMaterial(keyExpansionSeed, _countof(keyExpansionSeed));

        this->session->pending_read_state->Initialize(this->security_parameters);
        this->session->pending_write_state->Initialize(this->security_parameters);

        ZeroMemory(pre_master_secret->address(), pre_master_secret->size());
    }

    void HandshakeProtocol::GenerateKeyMaterial(IStreamWriter<byte>** keyExpansionSeed, uint32 keyExpansionSeedCount, byte* output, uint32 output_length)
    {
        Tls::globals->PRF(
            this->security_parameters->prf_algorithm,
            &this->security_parameters->master_secret,
            &Tls::globals->key_expansion_label,
            keyExpansionSeed,
            keyExpansionSeedCount,
            output,
            output_length);
    }
}
