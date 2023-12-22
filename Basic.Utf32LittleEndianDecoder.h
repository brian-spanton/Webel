// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IDecoder.h"

namespace Basic
{
    class Utf32LittleEndianDecoder : public IDecoder
    {
    private:
        IStream<Codepoint>* destination = 0;
        uint8 received = 0;
        Codepoint codepoint = 0;

        void Emit(Codepoint codepoint);

    public:
        void IDecoder::set_destination(IStream<Codepoint>* destination);
        void IDecoder::write_elements(const byte* elements, uint32 count);
        void IDecoder::write_element(byte element);
        void IDecoder::write_eof();
    };

    class Utf32LittleEndianDecoderFactory : public IDecoderFactory
    {
    public:
        virtual void IDecoderFactory::CreateDecoder(std::shared_ptr<IDecoder>* decoder);
    };
}