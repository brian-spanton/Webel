// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    class Path : public std::vector<UnicodeStringRef>
    {
    public:
        template <bool case_sensitive>
        bool equals(const Path& rvalue) const
        {
            if (this->size() != rvalue.size())
                return false;

            for (uint16 i = 0; i < this->size(); i++)
            {
                if (!Basic::equals<UnicodeString, case_sensitive>(this->at(i).get(), rvalue.at(i).get()))
                    return false;
            }

            return true;
        }

        template <bool case_sensitive>
        bool BelongsTo(const Path& rvalue) const
        {
            if (this->size() < rvalue.size())
                return false;

            for (uint16 i = 0; i != rvalue.size(); i++)
            {
                if (!Basic::equals<UnicodeString, case_sensitive>(this->at(i).get(), rvalue.at(i).get()))
                    return false;
            }

            return true;
        }

        void GetString(uint8 separator, UnicodeStringRef* value);
        void GetReverseString(uint8 separator, UnicodeStringRef* value);
    };
}
