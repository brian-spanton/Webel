// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.NumberFrame.h"
#include "Basic.IStream.h"
#include "Gzip.Types.h"

namespace Gzip
{
    using namespace Basic;

    class Deflate : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            done_state = Succeeded_State,
        };

        std::shared_ptr<IStream<byte> > uncompressed;

    public:
        Deflate(std::shared_ptr<IStream<byte> > uncompressed);

        virtual EventResult IProcess::consider_event(IEvent* event);
    };
}