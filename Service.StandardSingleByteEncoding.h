// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.IStream.h"
#include "Basic.HexNumberStream.h"
#include "Web.Client.h"
#include "Json.Types.h"

namespace Service
{
    using namespace Basic;

    class StandardSingleByteEncoding : public Frame, public UnitStream<byte>, public std::enable_shared_from_this<StandardSingleByteEncoding>
    {
    private:
        enum State
        {
            headers_pending_state = Start_State,
            line_start_state,
            ignore_line_state,
            before_index_state,
            index_pending_state,
            before_codepoint_state,
            codepoint_pending_state,
            done_state = Succeeded_State,
            before_index_error,
            before_codepoint_error,
            line_start_error,
            index_pending_error,
            codepoint_pending_error,
            connection_lost_error,
            malformed_content_error,
        };

        std::shared_ptr<StandardSingleByteEncoding> self;
        std::shared_ptr<Web::Client> client;
        std::shared_ptr<Json::Value> json_value;
        byte pointer;
        DecNumberStream<byte, byte> pointer_stream;
        Codepoint codepoint;
        HexNumberStream<byte, Codepoint> codepoint_stream;
        std::shared_ptr<SingleByteEncodingIndex> index;

        virtual ProcessResult IProcess::consider_event(IEvent* event);
        void switch_to_state(State state);

    public:
        StandardSingleByteEncoding(std::shared_ptr<SingleByteEncodingIndex> index);

        void start(std::shared_ptr<Uri> index_url);
        virtual void IStream<byte>::write_element(byte element);
        virtual void IStream<byte>::write_eof();
    };
}