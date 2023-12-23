// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"

namespace Html
{
    using namespace Basic;

    class ElementName
    {
    public:
        UnicodeStringRef name_space;
        UnicodeStringRef name;

        ElementName();

        void Initialize(ElementName* element_name);
        void Initialize(UnicodeStringRef name_space, UnicodeStringRef name);

        bool equals(ElementName* element_name);

        bool is_in_namespace(UnicodeString* name_space);
    };

    typedef std::vector<std::shared_ptr<ElementName> > ElementNameList;
}