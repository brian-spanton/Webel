// Copyright © 2013 Brian Spanton

#pragma once

#include "Web.Client.h"
#include "Basic.Frame.h"
#include "Json.Parser.h"

namespace Service
{
    using namespace Basic;

    class HtmlNamedCharacterReferences : public Frame
    {
    private:
        enum State
        {
            named_character_reference_state = Start_State,
            done_state = Succeeded_State,
        };

        Web::Client::Ref client; // REF
        Json::Parser::Ref json_parser; // REF
        Basic::Ref<IProcess> characters_completion; // REF
        ByteString::Ref characters_cookie; // REF

        UnicodeString::Ref codepoints_member_name; // REF

    public:
        typedef Basic::Ref<HtmlNamedCharacterReferences> Ref;

        HtmlNamedCharacterReferences();

        void Initialize(Basic::Ref<IProcess> completion, ByteString::Ref cookie);

        virtual void IProcess::Process(IEvent* event, bool* yield);
    };
}