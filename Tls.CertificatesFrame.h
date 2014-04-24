// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.VectorFrames.h"

namespace Tls
{
    class CertificatesFrame : public VectorFrame<OpaqueVector, 0, 0xffffff, 3>
    {
    private:
        virtual void GetItemFrame(Item* item, Basic::Ref<IProcess>* value)
        {
            CertificateFrame::Ref frame = New<CertificateFrame>();
            frame->Initialize(item);
            (*value) = frame;
        }

        virtual void GetItemSerializer(Item* item, Basic::Ref<ISerializable>* value)
        {
            CertificateFrame::Ref frame = New<CertificateFrame>();
            frame->Initialize(item);
            (*value) = frame;
        }
    };
}