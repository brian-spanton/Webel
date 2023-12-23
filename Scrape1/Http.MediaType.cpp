// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.MediaType.h"
#include "Http.MediaTypeStream.h"

namespace Http
{
    void MediaType::Initialize()
    {
        this->type = std::make_shared<UnicodeString>();
        this->subtype = std::make_shared<UnicodeString>();
        this->parameters = std::make_shared<NameValueCollection>();
    }

    void MediaType::Initialize(UnicodeString* value)
    {
        Initialize();

        MediaTypeStream frame;
        frame.Initialize(this);

        frame.write_elements(value->address(), value->size());
        frame.write_eof();
    }

    bool MediaType::equals(MediaType* value)
    {
        if (!Basic::equals<UnicodeString, false>(value->type.get(), this->type.get()))
            return false;

        if (!Basic::equals<UnicodeString, false>(value->subtype.get(), this->subtype.get()))
            return false;

        return true;
    }
}
