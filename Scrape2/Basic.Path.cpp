// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Path.h"

namespace Basic
{
    void Path::GetString(uint8 separator, UnicodeStringRef* value)
    {
        UnicodeStringRef result = std::make_shared<UnicodeString>();
        result->reserve(0x100);

        for (Path::iterator it = this->begin(); it != this->end(); it++)
        {
            if (it != this->begin())
                result->push_back(separator);

            result->append((*it)->begin(), (*it)->end());
        }

        (*value) = result;
    }

    void Path::GetReverseString(uint8 separator, UnicodeStringRef* value)
    {
        UnicodeStringRef result = std::make_shared<UnicodeString>();
        result->reserve(0x100);

        for (Path::reverse_iterator it = this->rbegin(); it != this->rend(); it++)
        {
            if (it != this->rbegin())
                result->push_back(separator);

            result->append((*it)->begin(), (*it)->end());
        }

        (*value) = result;
    }
}
