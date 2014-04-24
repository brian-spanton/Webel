// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.ISerializable.h"
#include "Basic.Event.h"
#include "Basic.Globals.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    template <class Type, int encoded_length = sizeof(Type)>
    class NumberFrame : public Frame, public ISerializable
    {
    private:
        enum State
        {
            start_state = Start_State,
            receiving_state,
            done_state = Succeeded_State,
        };

        uint32 received;
        Type* value;

    public:
        typedef Basic::Ref<NumberFrame<Type, encoded_length>, IProcess> Ref;

        void Initialize(Type* value)
        {
            __super::Initialize();
            this->received = 0;
            this->value = value;
        }

        virtual void IProcess::Process(IEvent* event, bool* yield)
        {
            switch (frame_state())
            {
            case State::start_state:
                ZeroMemory(this->value, sizeof(Type));
                switch_to_state(State::receiving_state);
                break;

            case State::receiving_state:
                {
                    byte b;
                    if (!Event::ReadNext(event, &b, yield))
                        return;

                    byte* value_bytes = reinterpret_cast<byte*>(this->value);
                    int index = encoded_length - this->received - 1;
                    value_bytes[index] = b;

                    this->received++;

                    if (this->received == encoded_length)
                        switch_to_state(State::done_state);
                }
                break;

            default:
                throw new Exception("Tls::NumberFrame::Process unexpected state");
            }
        }

        virtual void ISerializable::SerializeTo(IStream<byte>* stream)
        {
            const byte zero = 0;
            byte* value_bytes = reinterpret_cast<byte*>(this->value);
            int most_significant = sizeof(Type) - 1;
            int overflow = 0;

            if (encoded_length < sizeof(Type))
            {
                overflow = sizeof(Type) - encoded_length;

                for (int i = 0; i < overflow; i++)
                {
                    if (value_bytes[most_significant - i] != 0)
                        throw new Exception("Tls::NumberFrame::SerializeTo overflow");
                }
            }
            else
            {
                int padding = encoded_length - sizeof(Type);

                for (int i = 0; i < padding; i++)
                {
                    stream->Write(&zero, 1);
                }
            }

            for (int i = overflow; i < sizeof(Type); i++)
            {
                stream->Write(value_bytes + most_significant - i, 1);
            }
        }
    };
}
