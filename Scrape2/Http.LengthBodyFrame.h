// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.StateMachine.h"
#include "Basic.IStream.h"
#include "Basic.ElementSource.h"

namespace Http
{
    using namespace Basic;

    class LengthBodyFrame
    {
    private:
        std::shared_ptr<IStream<byte> > body_stream;
        uint32 bytes_expected;
        uint32 bytes_received;

    public:
        LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream);
        LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream, uint32 bytes_expected);

        void reset(uint32 bytes_expected);

        bool write_elements(IElementSource<byte>* element_source);
    };
}