#pragma once

#include "Tls.Types.h"
#include "Tls.VectorFrame.h"

namespace Tls
{
	typedef VectorFrame<CompressionMethod, 1, 0xff, 1> CompressionMethodsFrame;
	typedef VectorFrame<opaque, 0, 0x20, 1> SessionIdFrame;
	typedef VectorFrame<SignatureAndHashAlgorithm, 2, 0xfffe, 2> SignatureAndHashAlgorithmsFrame;
	typedef VectorFrame<opaque, 1, 0xffffff, 3> CertificateFrame;
	typedef VectorFrame<opaque, 1, 0xffff, 2> HostNameFrame;
	typedef VectorFrame<opaque, 0, 0xff, 1> RenegotiationInfoFrame;
	typedef VectorFrame<opaque, 1, 0xffff, 2> ResponderIDFrame;
	typedef VectorFrame<opaque, 0, 0xffffff, 2> ExtensionsFrame;
	typedef VectorFrame<opaque, 0, 0xffff, 2> EncryptedPreMasterSecretFrame;
}