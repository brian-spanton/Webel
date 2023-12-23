// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.LengthBodyFrame.h"

namespace Http
{
    using namespace Basic;

    LengthBodyFrame::LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream) :
        body_stream(body_stream),
        bytes_expected(0),
        bytes_received(0)
    {
    }

    LengthBodyFrame::LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream, uint32 bytes_expected) :
        body_stream(body_stream),
        bytes_expected(bytes_expected),
        bytes_received(0)
    {
    }

    void LengthBodyFrame::reset(uint32 bytes_expected)
    {
        this->bytes_expected = bytes_expected;
        this->bytes_received = 0;
    }

    bool LengthBodyFrame::write_elements(IElementSource<byte>* element_source)
    {
        const byte* elements;
        uint32 count;

        element_source->Read(this->bytes_expected - this->bytes_received, &elements, &count);

        this->body_stream->write_elements(elements, count);

        this->bytes_received += count;

        if (this->bytes_received != this->bytes_expected)
            return false;

        return true;
    }
}