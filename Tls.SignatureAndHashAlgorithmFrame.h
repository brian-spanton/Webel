// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class SignatureAndHashAlgorithmFrame : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            signature_algorithm_state,
            name_state,
            done_state = Succeeded_State,
            signature_algorithm_frame_failed,
            hash_algorithm_frame_failed,
        };

        SignatureAndHashAlgorithm* signature_and_hash_algorithm;
        MemoryRange signature_algorithm_frame;
        MemoryRange hash_algorithm_frame;

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        SignatureAndHashAlgorithmFrame(SignatureAndHashAlgorithm* signature_and_hash_algorithm);
    };

    template <>
    struct __declspec(novtable) serialize<SignatureAndHashAlgorithm>
    {
        void operator()(const SignatureAndHashAlgorithm* value, IStream<byte>* stream) const
        {
            serialize<SignatureAlgorithm>()(&value->signature, stream);
            serialize<HashAlgorithm>()(&value->hash, stream);
        }
    };

    template <>
    struct __declspec(novtable) make_deserializer<SignatureAndHashAlgorithm> : public make_frame_deserializer<SignatureAndHashAlgorithm, SignatureAndHashAlgorithmFrame> {};
}