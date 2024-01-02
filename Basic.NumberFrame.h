// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Event.h"
#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Basic.CountStream.h"
#include "Basic.Frame.h"

namespace Basic
{
    ///////////////////////////////////////////////////////////////////////////
    // serialization meta template
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) serialize
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
            static_assert(false, "No Basic::serialize defined for this type");
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // deserialization meta template
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) make_deserializer
    {
        void operator()(value_type* value, std::shared_ptr<IProcess>* deserializer) const
        {
            static_assert(false, "No Tls::make_deserializer defined for this type");
        }
    };

    template <typename value_type, typename frame_type>
    struct __declspec(novtable) make_frame_deserializer
    {
        void operator()(value_type* value, std::shared_ptr<IProcess>* deserializer) const
        {
            std::shared_ptr<frame_type> frame = std::make_shared<frame_type>(value);
            (*deserializer) = frame;
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    // number serialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type, int encoded_length = sizeof(value_type)>
    struct __declspec(novtable) serialize_number
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
            byte* value_bytes = (byte*)value;
            int most_significant = sizeof(value_type) - 1;
            int overflow = 0;

            if (encoded_length < sizeof(value_type))
            {
                overflow = sizeof(value_type) - encoded_length;

                for (int i = 0; i < overflow; i++)
                {
                    if (value_bytes[most_significant - i] != 0)
                        throw FatalError("Tls::NumberFrame::write_to_stream overflow");
                }
            }
            else
            {
                int padding = encoded_length - sizeof(value_type);

                for (int i = 0; i < padding; i++)
                {
                    stream->write_element(0);
                }
            }

            for (int i = overflow; i < sizeof(value_type); i++)
            {
                stream->write_element(value_bytes[most_significant - i]);
            }
        }
    };

    template <> struct __declspec(novtable) serialize<uint16> : public serialize_number<uint16> {};
    template <> struct __declspec(novtable) serialize<uint32> : public serialize_number<uint32> {};
    template <> struct __declspec(novtable) serialize<uint64> : public serialize_number<uint64> {};

    ///////////////////////////////////////////////////////////////////////////
    // number deserialization
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type, int encoded_length = sizeof(value_type)>
    class NumberFrame : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            receiving_state,
            done_state = Succeeded_State,
        };

        uint32 received;
        value_type* value;

    public:
        NumberFrame(value_type* value)
        {
            this->received = 0;
            this->value = value;
        }

        void reset()
        {
            __super::reset();
            this->received = 0;
        }

        virtual ProcessResult IProcess::process_event(IEvent* event)
        {
            switch (get_state())
            {
            case State::start_state:
                ZeroMemory(this->value, sizeof(value_type));
                switch_to_state(State::receiving_state);
                break;

            case State::receiving_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                byte* value_bytes = reinterpret_cast<byte*>(this->value);
                int index = encoded_length - this->received - 1;
                value_bytes[index] = b;

                this->received++;

                if (this->received == encoded_length)
                    switch_to_state(State::done_state);
            }
            break;

            default:
                throw FatalError("Tls::NumberFrame::handle_event unexpected state");
            }

            return ProcessResult::process_result_ready;
        }
    };

    template <typename value_type, int encoded_length = sizeof(value_type)>
    struct __declspec(novtable) make_number_deserializer : public make_frame_deserializer<value_type, NumberFrame<value_type, encoded_length> > {};

    template <> struct __declspec(novtable) make_deserializer<uint16> : public make_number_deserializer<uint16> {};
    template <> struct __declspec(novtable) make_deserializer<uint32> : public make_number_deserializer<uint32> {};
    template <> struct __declspec(novtable) make_deserializer<uint64> : public make_number_deserializer<uint64> {};

    ///////////////////////////////////////////////////////////////////////////
    // serializer object
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    class Serializer : public IStreamWriter<byte>
    {
    private:
        value_type* value;

    public:
        Serializer(value_type* value)
        {
            this->value = value;
        }

        virtual void IStreamWriter<byte>::write_to_stream(IStream<byte>* stream) const
        {
            serialize<value_type>()(value, stream);
        }
    };
}