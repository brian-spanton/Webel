#pragma once

namespace Basic
{
	__interface IEvent;

	__interface IProcess : public IRefCounted
	{
		void Process(IEvent* event, bool* yield);
		void Process(IEvent* event);
		bool Pending();
		bool Succeeded();
		bool Failed();
	};
}