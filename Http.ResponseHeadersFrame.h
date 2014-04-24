// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.ISerializable.h"
#include "Basic.DecNumberStream.h"
#include "Http.Types.h"
#include "Http.HeadersFrame.h"

namespace Http
{
    using namespace Basic;

    class ResponseHeadersFrame : public Frame, public ISerializable
    {
    private:
        enum State
        {
            receiving_protocol_state = Start_State,
            receiving_code_state,
            receiving_reason_state,
            expecting_LF_after_reason_state,
            headers_frame_pending_state,
            done_state = Succeeded_State,
            receiving_code_error,
            write_to_number_stream_failed,
            receiving_reason_error,
            expecting_LF_after_reason_error,
            headers_frame_failed,
        };

        Response* response;
        UnicodeString::Ref method; // REF
        Inline<DecNumberStream<byte, uint16> > number_stream;
        Inline<HeadersFrame> headers_frame;

    public:
        typedef Basic::Ref<ResponseHeadersFrame, IProcess> Ref;

        void Initialize(UnicodeString* method, Response* response);
        void WriteResponseLineTo(IStream<byte>* stream);
        virtual void IProcess::Process(IEvent* event, bool* yield);
        virtual void ISerializable::SerializeTo(IStream<byte>* stream);
    };
}