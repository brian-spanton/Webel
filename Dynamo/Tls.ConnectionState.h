#pragma once

#include "Basic.IRefCounted.h"
#include "Tls.Types.h"
#include "Tls.SecurityParameters.h"
#include "Basic.Cng.h"

namespace Tls
{
	class ConnectionState : public IRefCounted
	{
	public:
		typedef Basic::Ref<ConnectionState> Ref;

		// compression_state
		Basic::BCRYPT_KEY_HANDLE key_handle;

		std::vector<opaque> MAC_key;
		std::vector<opaque> encryption_key;
		ByteVector::Ref IV; // $$$

		uint64 sequence_number;
		SecurityParameters::Ref security_parameters; // $$$

		ConnectionState();
		virtual ~ConnectionState();

		bool Initialize(SecurityParameters* params);
		void MAC(Record* compressed, opaque* output, uint8 output_max);
	};

}