// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Web.Client.h"
#include "Json.Parser.h"

namespace Service
{
    using namespace Basic;

    class StandardEncodings : public Frame, public std::enable_shared_from_this<StandardEncodings>
    {
    private:
        enum State
        {
            single_byte_encodings_state = Start_State,
            done_state = Succeeded_State,
        };

        std::shared_ptr<StandardEncodings> self;
        std::shared_ptr<Web::Client> client;
        std::shared_ptr<Json::Parser> json_parser;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;

        UnicodeStringRef Name_encodings;
        UnicodeStringRef Name_heading;
        UnicodeStringRef heading_utf8;
        UnicodeStringRef heading_legacy;
        UnicodeStringRef Name_name;
        UnicodeStringRef Name_labels;

        virtual ProcessResult IProcess::process_event(IEvent* event);
        void switch_to_state(State state);

    public:
        StandardEncodings(std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        void start();
    };
}
