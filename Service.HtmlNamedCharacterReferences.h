// Copyright © 2013 Brian Spanton

#pragma once

#include "Web.Client.h"
#include "Basic.Frame.h"
#include "Json.Parser.h"

namespace Service
{
    using namespace Basic;

    class HtmlNamedCharacterReferences : public Frame, public std::enable_shared_from_this<HtmlNamedCharacterReferences>
    {
    private:
        enum State
        {
            named_character_reference_state = Start_State,
            done_state = Succeeded_State,
            unexpected_json_error,
            unexpected_event_error,
        };

        std::shared_ptr<HtmlNamedCharacterReferences> self;
        std::shared_ptr<Web::Client> client;
        std::shared_ptr<Json::Parser> json_parser;
        std::weak_ptr<IProcess> completion;
        std::shared_ptr<void> context;

        UnicodeStringRef codepoints_member_name;

        virtual ProcessResult IProcess::process_event(IEvent* event);
        void switch_to_state(State state);

    public:
        HtmlNamedCharacterReferences(std::shared_ptr<IProcess> completion, std::shared_ptr<void> context);

        void start();
    };
}