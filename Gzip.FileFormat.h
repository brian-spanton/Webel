// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.NumberFrame.h"
#include "Basic.IStream.h"
#include "Gzip.MemberFrame.h"

namespace Gzip
{
    using namespace Basic;

    class FileFormat : public Frame
    {
    private:
        enum State
        {
            new_member_state = Start_State,
            member_frame_state,
            next_member_state,

            done_state = Succeeded_State,
            member_frame_failed,
            next_member_failed,
        };

        std::vector<std::shared_ptr<MemberHeader> > members;
        std::shared_ptr<IStream<byte> > first_output;
        std::shared_ptr<MemberHeader> member;
        std::shared_ptr<MemberFrame> member_frame;

    public:
        FileFormat(std::shared_ptr<IStream<byte> > first_output);

        virtual EventResult IProcess::consider_event(IEvent* event);
    };
}