// Copyright © 2013 Brian Spanton

#pragma once

#include "Web.Client.h"
#include "Json.Parser.h"

namespace Service
{
    using namespace Basic;

    // $$$ should be a persistent object
    class HtmlNamedCharacterReferences : public StateMachine, public std::enable_shared_from_this<HtmlNamedCharacterReferences>
    {
    private:
        enum State
        {
            named_character_reference_state = Start_State,
            done_state = Succeeded_State,
        };

        std::shared_ptr<HtmlNamedCharacterReferences> self;
        std::shared_ptr<Web::Client> client;
        std::shared_ptr<Json::Parser> json_parser;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;

        UnicodeStringRef codepoints_member_name;

        void consider_event(void* event);
        void switch_to_state(State state);

    public:
        HtmlNamedCharacterReferences(std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        void start();
    };
}