// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Http
{
    using namespace Basic;

    class UrlDecoder : public UnitStream<byte> // $$ this class is unused at the moment
    {
    private:
        enum State
        {
            normal_state,
            hex1_state,
            hex2_state,
            hex1_error,
            hex2_error,
        };

        std::shared_ptr<IStream<byte> > destination;
        byte hex;

    public:
        State state;

        void Initialize(std::shared_ptr<IStream<byte> > destination);

        virtual void IStream<byte>::write_element(byte element);
        virtual void IStream<byte>::write_eof();
    };
}