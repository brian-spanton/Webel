// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.Frame.h"
#include "Http.Types.h"

namespace Http
{
    using namespace Basic;

    class BodyFrame : public Frame
    {
    protected:
        std::shared_ptr<IStream<byte> > decoded_content_stream;

    public:
        BodyFrame(std::shared_ptr<IStream<byte> > decoded_content_stream);

        static void make_body_frame(std::shared_ptr<IStream<byte> > decoded_content_stream, NameValueCollection* headers, std::shared_ptr<BodyFrame>* output);
    };
}