// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class PreMasterSecretFrame : public Frame
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

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        PreMasterSecretFrame(PreMasterSecret* pre_master_secret);
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::PreMasterSecret>
    {
        void operator()(const Tls::PreMasterSecret* value, IStream<byte>* stream) const
        {
            serialize<uint16>()(&value->client_version, stream);
            serialize<const byte[46]>()(&value->random, stream);
        }
    };
}