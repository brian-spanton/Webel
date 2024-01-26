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

    void HandshakeProtocol::CalculateVerifyData(ByteString* label, byte* output, uint16 output_max)
    {
        if (output_max < this->security_parameters->verify_data_length)
            throw FatalError("Tls", "HandshakeProtocol", "CalculateVerifyData", "output_max < this->security_parameters->verify_data_length", this->security_parameters->verify_data_length);

        IStreamWriter<byte>* seed[] = { this->handshake_messages.get(), };

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

        IStreamWriter<byte>* prf_seed[] = { md5.get(), sha1.get(), };

        Tls::globals->PRF(
            this->security_parameters->prf_algorithm,
            &this->security_parameters->master_secret,
            label,
            prf_seed,
            _countof(prf_seed),
            output,
            this->security_parameters->verify_data_length);
    }

    bool HandshakeProtocol::WriteFinished(ByteString* label)
    {
        // http://www.ietf.org/mail-archive/web/tls/current/msg09221.html

        std::shared_ptr<ByteString> finished_data = std::make_shared<ByteString>();
        finished_data->resize(this->security_parameters->verify_data_length);

        CalculateVerifyData(label, finished_data->address(), (uint16)finished_data->size());

        // send the handshake buffer so far, before sending change cipher spec

        if (this->send_buffer->size() > 0)
        {
            ByteStringRef send_buffer = std::make_shared<ByteString>();
            send_buffer.swap(this->send_buffer);

            this->session->write_record_elements(ContentType::handshake, send_buffer->address(), send_buffer->size());
        }

        this->session->WriteChangeCipherSpec();

        bool success = WriteMessage(HandshakeType::finished, finished_data.get());
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

        // generate keys per TLS section 6.3

        uint16 key_material_length = 2 * (this->security_parameters->mac_key_length + this->security_parameters->enc_key_length + this->security_parameters->block_length);
        ByteString key_material;
        key_material.resize(key_material_length);

        IStreamWriter<byte>* keyExpansionSeed[] = { &server_random_serializer, &client_random_serializer, };

        Tls::globals->PRF(
            this->security_parameters->prf_algorithm,
            &this->security_parameters->master_secret,
            &Tls::globals->key_expansion_label,
            keyExpansionSeed,
            _countof(keyExpansionSeed),
            key_material.address(),
            key_material.size());

        this->session->pending_read_state->IV = std::make_shared<ByteString>();
        this->session->pending_write_state->IV = std::make_shared<ByteString>();

        PartitionKeyMaterial(&key_material);

        this->session->pending_read_state->Initialize(this->security_parameters);
        this->session->pending_write_state->Initialize(this->security_parameters);

        ZeroMemory(pre_master_secret->address(), pre_master_secret->size());
    }
}
