// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.RefCounted.h"
#include "Basic.NameValueCollection.h"
#include "Basic.IDecoderFactory.h"
#include "Basic.IEncoderFactory.h"
#include "Basic.IEvent.h"
#include "Basic.IElementSource.h"

namespace Basic
{
    struct Exception
    {
        Exception(const char* context);
        Exception(const char* context, uint32 error);
    };

    struct Path : std::vector<UnicodeString::Ref> // REF
    {
        template <bool case_sensitive>
        bool equals(const Path& rvalue) const
        {
            if (this->size() != rvalue.size())
                return false;

            for (uint16 i = 0; i < this->size(); i++)
            {
                if (!this->at(i).equals<case_sensitive>(rvalue.at(i)))
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
                if (!this->at(i).equals<case_sensitive>(rvalue.at(i)))
                    return false;
            }

            return true;
        }

        void GetString(uint8 separator, UnicodeString::Ref* value)
        {
            UnicodeString::Ref result = New<UnicodeString>();

            for (Path::iterator it = this->begin(); it != this->end(); it++)
            {
                if (it != this->begin())
                    result->push_back(separator);

                result->append((*it)->begin(), (*it)->end());
            }

            (*value) = result;
        }

        void GetReverseString(uint8 separator, UnicodeString::Ref* value)
        {
            UnicodeString::Ref result = New<UnicodeString>();

            for (Path::reverse_iterator it = this->rbegin(); it != this->rend(); it++)
            {
                if (it != this->rbegin())
                    result->push_back(separator);

                result->append((*it)->begin(), (*it)->end());
            }

            (*value) = result;
        }
    };

    bool HandleError(const char* context);

    typedef StringMapCaseInsensitive<Ref<IEncoderFactory> > EncoderMap;
    typedef StringMapCaseInsensitive<Ref<IDecoderFactory> > DecoderMap;

    enum EventType
    {
        process_event,
        ready_for_read_bytes_event,
        ready_for_write_bytes_event,
        ready_for_read_codepoints_event,
        ready_for_write_codepoints_event,
        element_stream_ending_event,
        request_headers_event,
        request_complete_event,
        encodings_complete_event,
    };

    struct ProcessEvent : public IEvent
    {
        virtual uint32 get_type();
    };

    struct ReadyForReadBytesEvent : public IEvent
    {
    public:
        Basic::Ref<IElementSource<byte> > element_source; // REF

        virtual uint32 get_type();

        void Initialize(IElementSource<byte>* element_source);
    };

    struct ReadyForWriteBytesEvent : public IEvent
    {
    public:
        Basic::Ref<IElementSource<byte> > element_source; // REF

        virtual uint32 get_type();

        void Initialize(IElementSource<byte>* element_source);
    };

    struct ReadyForReadCodepointsEvent : public IEvent
    {
    public:
        Basic::Ref<IElementSource<Codepoint> > element_source; // REF

        virtual uint32 get_type();

        void Initialize(IElementSource<Codepoint>* element_source);
    };

    struct ReadyForWriteCodepointsEvent : public IEvent
    {
    public:
        Basic::Ref<IElementSource<Codepoint> > element_source; // REF

        virtual uint32 get_type();

        void Initialize(IElementSource<Codepoint>* element_source);
    };

    struct ElementStreamEndingEvent : public IEvent
    {
        virtual uint32 get_type();
    };

    struct EncodingsCompleteEvent : public IEvent
    {
        ByteString::Ref cookie; // REF

        virtual uint32 get_type();
    };
}
