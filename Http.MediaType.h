// Copyright © 2013 Brian Spanton

#pragma once

namespace Http
{
    using namespace Basic;

    struct MediaType
    {
        UnicodeStringRef type;
        UnicodeStringRef subtype;
        std::shared_ptr<NameValueCollection> parameters;

        void Initialize();
        void Initialize(UnicodeString* value);
        bool equals(MediaType* value);
    };
}
