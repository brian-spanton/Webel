// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"

namespace Html
{
    using namespace Basic;

    class EndOfFileToken : public Token
    {
    private:

    public:
        typedef Basic::Ref<EndOfFileToken> Ref;

        EndOfFileToken();
    };
}