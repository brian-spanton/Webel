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

        ByteStreamDecoder::Ref decoder; // REF
        Tokenizer::Ref tokenizer; // REF

        bool ParseError(const char* error);

    public:
        typedef Basic::Ref<Parser, IStream<byte> > Ref;

        Text::Ref text; // REF

        void Initialize(Html::Node::Ref domain, UnicodeString::Ref charset);
    };
}