// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"
#include "Basic.MemoryRange.h"

namespace Tls
{
    using namespace Basic;

    class DigitallySignedFrame : public Frame
    {
    private:
        enum State
        {
            algorithm_frame_pending_state = Start_State,
            signature_frame_pending_state,
            done_state = Succeeded_State,
            algorithm_frame_failed,
            signature_frame_failed,
        };

        DigitallySigned* digitally_signed;
        SignatureAndHashAlgorithmFrame algorithm_frame;
        VectorFrame<Signature> signature_frame;

        virtual ProcessResult IProcess::process_event(IEvent* event);

    public:
        DigitallySignedFrame(DigitallySigned* digitally_signed);

        void reset();
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::DigitallySigned>
    {
        void operator()(const Tls::DigitallySigned* value, IStream<byte>* stream) const
        {
            serialize<Tls::SignatureAndHashAlgorithm>()(&value->algorithm, stream);
            serialize<Tls::Signature>()(&value->signature, stream);
        }
    };
}
