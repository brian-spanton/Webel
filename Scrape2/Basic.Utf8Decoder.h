// Copyright � 2013 Brian Spanton

#pragma once

#include "Basic.IDecoder.h"

namespace Basic
{
    class Utf8Decoder : public IDecoder
    {
    private:
        IStream<Codepoint>* destination;
        int needed;
        int seen;
        Codepoint codepoint;
        Codepoint lower_bound;
        Codepoint upper_bound;

        void Emit(Codepoint codepoint);
        void EmitDecoderError(byte b);
        void EmitDecoderError(const char* error);

    public:
        void IDecoder::set_destination(IStream<Codepoint>* destination);
        void IDecoder::write_elements(const byte* elements, uint32 count);
        void IDecoder::write_element(byte element);
        void IDecoder::write_eof();
    };

    class Utf8DecoderFactory : public IDecoderFactory
    {
    public:
        virtual void IDecoderFactory::CreateDecoder(std::shared_ptr<IDecoder>* decoder);
    };
}