// Copyright � 2013 Brian Spanton

#pragma once

#include "Html.TagToken.h"

namespace Html
{
    using namespace Basic;

    class EndTagToken : public TagToken
    {
    public:
        EndTagToken();

        virtual void GetDebugString(char* debug_string, int count);
    };
}