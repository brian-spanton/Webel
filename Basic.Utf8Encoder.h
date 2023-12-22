// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEncoder.h"

namespace Basic
{
    class Utf8Encoder : public IEncoder
    {
    private:
        IStream<byte>* destination = 0;
        byte error_replacement_byte = 0;

        void Emit(byte b);
        void EncoderError(Codepoint codepoint);

    public:
        Utf8Encoder();

        void IEncoder::set_destination(IStream<byte>* destination);
        void IEncoder::set_error_replacement_byte(byte b);
        void IEncoder::write_elements(const Codepoint* elements, uint32 count);
        void IEncoder::write_element(Codepoint element);
        void IEncoder::write_eof();
    };

    class Utf8EncoderFactory : public IEncoderFactory
    {
    public:
        virtual void IEncoderFactory::CreateEncoder(std::shared_ptr<IEncoder>* encoder);
    };
}