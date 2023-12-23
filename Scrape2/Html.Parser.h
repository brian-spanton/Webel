// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.ByteStreamDecoder.h"
#include "Html.InputStreamPreprocessor.h"
#include "Html.Tokenizer.h"
#include "Html.TreeConstruction.h"

namespace Html
{
    using namespace Basic;

    class Parser : public UnitStream<byte>
    {
    private:
        friend class ByteStreamDecoder;
        friend class InputStreamPreprocessor;
        friend class Tokenizer;
        friend class TreeConstruction;
        friend class CharacterReferenceFrame;

    public:
        std::shared_ptr<TreeConstruction> tree;

    private:
        std::shared_ptr<Tokenizer> tokenizer;
        std::shared_ptr<InputStreamPreprocessor> preprocessor;
        std::shared_ptr<ByteStreamDecoder> decoder;

        bool ParseError(const char* error);

    public:
        Parser(std::shared_ptr<Uri> url, UnicodeStringRef charset);

        virtual void IStream<byte>::write_element(byte element);
    };
}