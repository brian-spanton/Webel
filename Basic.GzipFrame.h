// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Basic
{
    class GzipFrame : public Frame
    {
    private:
        std::shared_ptr<IStream<byte> > uncompressed;

    public:
        GzipFrame(std::shared_ptr<IStream<byte> > uncompressed);

        virtual EventResult IProcess::consider_event(IEvent* event);
    };
}