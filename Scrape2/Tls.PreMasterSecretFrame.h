// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.MemoryRange.h"
#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class PreMasterSecretFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
            version_frame_pending_state = Start_State,
            random_frame_pending_state,
            done_state = Succeeded_State,
            version_frame_failed,
            random_frame_failed,
        };

        PreMasterSecret* pre_master_secret;
        NumberFrame<uint16> version_frame;
        MemoryRange random_frame;

    public:
        PreMasterSecretFrame(PreMasterSecret* pre_master_secret);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};

    template <>
    struct __declspec(novtable) serialize<PreMasterSecret>
    {
        void operator()(const PreMasterSecret* value, IStream<byte>* stream) const
        {
            serialize<uint16>()(&value->client_version, stream);
            serialize<const byte[46]>()(&value->random, stream);
        }
    };
}