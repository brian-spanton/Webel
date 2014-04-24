// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.ByteStreamDecoder.h"
#include "Html.InputStreamPreprocessor.h"
#include "Html.Tokenizer.h"
#include "Html.TreeConstruction.h"
#include "Basic.FrameStream.h"

namespace Html
{
    using namespace Basic;

    class Parser : public FrameStream<byte>
    {
    private:
        friend class ByteStreamDecoder;
        friend class InputStreamPreprocessor;
        friend class Tokenizer;
        friend class TreeConstruction;
        friend class CharacterReferenceFrame;

        ByteStreamDecoder::Ref decoder; // REF
        InputStreamPreprocessor::Ref preprocessor; // REF
        Tokenizer::Ref tokenizer; // REF

        bool ParseError(const char* error);

    public:
        typedef Basic::Ref<Parser, IStream<byte> > Ref;

        TreeConstruction::Ref tree; // REF

        void Initialize(Uri::Ref url, UnicodeString::Ref charset);
    };
}