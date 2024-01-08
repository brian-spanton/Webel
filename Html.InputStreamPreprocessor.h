// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.Node.h"
#include "Html.Token.h"

namespace Html
{
    using namespace Basic;

    class Parser;

    class InputStreamPreprocessor : public UnitStream<Codepoint>
    {
    private:
        enum State
        {
            bom_state,
            normal_state,
            ignore_lf_state,
        };

        State state;
        std::shared_ptr<IStream<Codepoint> > output;
        Parser* parser;

        bool IsValid(Codepoint codepoint);

    public:
        InputStreamPreprocessor(Parser* parser, std::shared_ptr<IStream<Codepoint> > output);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
    };
}