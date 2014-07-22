// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.FrameStream.h"
#include "Json.ByteStreamDecoder.h"
#include "Json.Tokenizer.h"
#include "Json.Text.h"

namespace Json
{
    using namespace Basic;

    class Parser : public UnitStream<byte>
    {
    private:
        friend class ByteStreamDecoder;
        friend class Tokenizer;
        friend class Text;

    public:
        std::shared_ptr<Text> text;

    private:
        std::shared_ptr<Tokenizer> tokenizer;
        std::shared_ptr<ByteStreamDecoder> decoder;

    public:
        Parser(std::shared_ptr<Html::Node> domain, UnicodeStringRef charset);

        virtual void IStream<byte>::write_element(byte element);
    };
}