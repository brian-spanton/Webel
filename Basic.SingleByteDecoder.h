// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IDecoder.h"
#include "Basic.IDecoderFactory.h"
#include "Basic.ISingleByteEncodingIndex.h"

namespace Basic
{
    class SingleByteDecoder : public IDecoder
    {
    private:
        IStream<Codepoint>* destination;
        std::shared_ptr<ISingleByteEncodingIndex> index;

        void Emit(Codepoint codepoint);
        void EmitDecoderError(byte b);
        void EmitDecoderError(const char* message);

    public:
        SingleByteDecoder(std::shared_ptr<ISingleByteEncodingIndex> index);
        SingleByteDecoder(std::shared_ptr<ISingleByteEncodingIndex> index, IStream<Codepoint>* destination);

        void IDecoder::set_destination(IStream<Codepoint>* destination);
        void IDecoder::write_elements(const byte* elements, uint32 count);
        void IDecoder::write_element(byte element);
        void IDecoder::write_eof();
    };

    class SingleByteDecoderFactory : public IDecoderFactory
    {
    private:
        std::shared_ptr<ISingleByteEncodingIndex> index;

    public:
        void Initialize(std::shared_ptr<ISingleByteEncodingIndex> index);

        virtual void IDecoderFactory::CreateDecoder(std::shared_ptr<IDecoder>* decoder);
    };
}