#pragma once

#include "Basic.Event.h"
#include "Basic.Uri.h"
#include "Basic.SuffixArray.h"
#include "Json.Types.h"

namespace Dynamo
{
	enum EventType
	{
		task_complete_event = 0x4000,
	};

	struct TaskCompleteEvent : public Basic::IEvent
	{
		Basic::ByteString::Ref cookie; // $$$

		virtual uint32 get_type();
	};

	struct Offer : public Basic::IRefCounted
	{
		typedef Basic::Ref<Offer> Ref;
	};

	typedef Basic::SuffixArray<Json::Object::Ref> VideoMap; // $$$
}