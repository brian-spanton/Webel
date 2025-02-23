// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.NameValueCollection.h"
#include "Basic.IDecoderFactory.h"
#include "Basic.IEncoderFactory.h"
#include "Basic.IEvent.h"
#include "Basic.IElementSource.h"

namespace Basic
{
    struct FatalError
    {
        FatalError();
        FatalError(const char* ns, const char* cl, const char* func, const char* message, uint32 code = 0);
    };

    struct Path : std::vector<UnicodeStringRef>
    {
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

        void GetString(uint8 separator, UnicodeStringRef* value)
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

        void GetReverseString(uint8 separator, UnicodeStringRef* value)
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
    };

    typedef StringMapCaseInsensitive<std::shared_ptr<IEncoderFactory> > EncoderMap;
    typedef StringMapCaseInsensitive<std::shared_ptr<IDecoderFactory> > DecoderMap;

    struct ContextualizedEvent : public Basic::IEvent
    {
        ContextualizedEvent();
        ContextualizedEvent(std::shared_ptr<void> context);

        std::shared_ptr<void> context;

        virtual uint32 Basic::IEvent::get_type() = 0;
    };

    struct IoCompletionEvent : public ContextualizedEvent
    {
        IoCompletionEvent(std::shared_ptr<void> context, uint32 count, uint32 error);

        uint32 count;
        uint32 error;

        virtual uint32 IEvent::get_type();
    };

    struct ReceivedBytesEvent : public IEvent
    {
    public:
        IElementSource<byte>* element_source = 0; // this was holding a ref, but it should be ok because Events only ever live on the stack

        virtual uint32 get_type();

        void Initialize(IElementSource<byte>* element_source);
    };

    struct CanSendBytesEvent : public IEvent
    {
    public:
        IElementSource<byte>* element_source = 0; // this was holding a ref, but it should be ok because Events only ever live on the stack

        virtual uint32 get_type();

        void Initialize(IElementSource<byte>* element_source);
    };

    struct ReceivedCodepointsEvent : public IEvent
    {
    public:
        IElementSource<Codepoint>* element_source = 0; // this was holding a ref, but it should be ok because Events only ever live on the stack

        virtual uint32 get_type();

        void Initialize(IElementSource<Codepoint>* element_source);
    };

    struct CanSendCodepointsEvent : public IEvent
    {
    public:
        IElementSource<Codepoint>* element_source = 0; // this was holding a ref, but it should be ok because Events only ever live on the stack

        virtual uint32 get_type();

        void Initialize(IElementSource<Codepoint>* element_source);
    };

    struct ElementStreamEndingEvent : public IEvent
    {
        virtual uint32 get_type();
    };

    struct EncodingsCompleteEvent : public ContextualizedEvent
    {
        virtual uint32 get_type();
    };
}
