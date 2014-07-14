// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.FrameStream.h"
#include "Json.ByteStreamDecoder.h"
#include "Json.Tokenizer.h"
#include "Json.Text.h"

namespace Json
{
    using namespace Basic;

    class Parser : public FrameStream<byte>
    {
    private:
        friend class ByteStreamDecoder;
        friend class Tokenizer;
        friend class Text;

        std::shared_ptr<ByteStreamDecoder> decoder;
        std::shared_ptr<Tokenizer> tokenizer;

        bool ParseError(const char* error);

    public:
        std::shared_ptr<Text> text;

        void Initialize(std::shared_ptr<Html::Node> domain, UnicodeStringRef charset);
    };
}