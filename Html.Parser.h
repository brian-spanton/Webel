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

        std::shared_ptr<ByteStreamDecoder> decoder;
        std::shared_ptr<InputStreamPreprocessor> preprocessor;
        std::shared_ptr<Tokenizer> tokenizer;

        bool ParseError(const char* error);

    public:
        std::shared_ptr<TreeConstruction> tree;

        void Initialize(std::shared_ptr<Uri> url, UnicodeStringRef charset);
    };
}