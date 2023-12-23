// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.ElementName.h"

namespace Html
{
    using namespace Basic;

    ElementName::ElementName()
    {
    }

    void ElementName::Initialize(ElementName* element_name)
    {
        this->name_space = element_name->name_space;
        this->name = element_name->name;
    }

    void ElementName::Initialize(UnicodeStringRef name_space, UnicodeStringRef name)
    {
        this->name_space = name_space;
        this->name = name;
    }

    bool ElementName::equals(ElementName* element_name)
    {
        if (element_name->name != this->name)
            return false;

        if (element_name->name_space != this->name_space)
            return false;

        return true;
    }

    bool ElementName::is_in_namespace(UnicodeString* name_space)
    {
        return Basic::equals<UnicodeString, true>(this->name_space.get(), name_space);
    }
}