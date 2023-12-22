// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEncoder.h"
#include "Basic.IEncoderFactory.h"
#include "Basic.ISingleByteEncodingIndex.h"

namespace Basic
{
    class SingleByteEncoder : public IEncoder
    {
    private:
        IStream<byte>* destination = 0;
        std::shared_ptr<ISingleByteEncodingIndex> index;
        byte error_replacement_byte = 0x7F; // $ is this a good default error_replacement_byte char?

        void Emit(byte b);
        void EncoderError(Codepoint codepoint);

    public:
        SingleByteEncoder();

        void Initialize(std::shared_ptr<ISingleByteEncodingIndex> index);
        void Initialize(std::shared_ptr<ISingleByteEncodingIndex> index, IStream<byte>* destination);

        void IEncoder::set_destination(IStream<byte>* destination);
        void IEncoder::set_error_replacement_byte(byte b);
        void IEncoder::write_elements(const Codepoint* elements, uint32 count);
        void IEncoder::write_element(Codepoint element);
        void IEncoder::write_eof();
    };

    class SingleByteEncoderFactory : public IEncoderFactory
    {
    private:
        std::shared_ptr<ISingleByteEncodingIndex> index;

    public:
        void Initialize(std::shared_ptr<ISingleByteEncodingIndex> index);

        virtual void IEncoderFactory::CreateEncoder(std::shared_ptr<IEncoder>* encoder);
    };
}