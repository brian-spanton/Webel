// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class SignatureAndHashAlgorithmFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
			signature_algorithm_state = Start_State,
            hash_algorithm_state,
            done_state = Succeeded_State,
            signature_algorithm_frame_failed,
            hash_algorithm_frame_failed,
        };

        SignatureAndHashAlgorithm* signature_and_hash_algorithm;
        MemoryRange signature_algorithm_frame;
        MemoryRange hash_algorithm_frame;

    public:
        SignatureAndHashAlgorithmFrame(SignatureAndHashAlgorithm* signature_and_hash_algorithm);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
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