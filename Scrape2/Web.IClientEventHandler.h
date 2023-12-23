// Copyright © 2013 Brian Spanton

#pragma once

namespace Web
{
    __interface IClientEventHandler
    {
        void response_completed(ByteStringRef cookie);
        void headers_completed(ByteStringRef cookie);
    };
}