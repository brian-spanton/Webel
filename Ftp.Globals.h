// Copyright © 2013 Brian Spanton

#pragma once

namespace Ftp
{
    using namespace Basic;

    class Globals
    {
    public:
        Globals();

        void Initialize();

        ByteStringRef greeting;
    };

    extern Globals* globals;
}