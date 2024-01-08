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

        // due to rules about constructor initializors, the order of all member declarations is important for this class.
        // see definition of Parser(std::shared_ptr<Uri> url, UnicodeStringRef charset)

    public:
        std::shared_ptr<TreeConstruction> tree; // init 1st

    private:
        std::shared_ptr<Tokenizer> tokenizer; // init 2nd
        std::shared_ptr<InputStreamPreprocessor> preprocessor; // init 3rd
        std::shared_ptr<ByteStreamDecoder> decoder; // init 4th

        void ParseError(const char* error);

    public:
        Parser(std::shared_ptr<Uri> url, UnicodeStringRef charset);

        virtual void IStream<byte>::write_element(byte element);
    };
}