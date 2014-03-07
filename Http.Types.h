// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.NameValueCollection.h"
#include "Basic.Uri.h"

namespace Http
{
	using namespace Basic;

	struct Request : public IRefCounted
	{
		typedef Basic::Ref<Request> Ref;

		UnicodeString::Ref method; // REF
		Uri::Ref resource; // REF
		UnicodeString::Ref protocol; // REF
		NameValueCollection::Ref headers; // REF
		Basic::Ref<ISerializable> client_body; // REF

		void Initialize();
		void Initialize(Request* request);
	};

	struct Response : public IRefCounted
	{
		typedef Basic::Ref<Response> Ref;

		UnicodeString::Ref protocol; // REF
		uint16 code;
		UnicodeString::Ref reason; // REF
		NameValueCollection::Ref headers; // REF

		Basic::Ref<ISerializable> server_body; // REF

		void Initialize();
	};

	struct Transaction
	{
		Request::Ref request; // REF
		Response::Ref response; // REF
	};

	typedef std::vector<Transaction> TransactionList;

	struct Cookie : public IRefCounted
	{
		typedef Basic::Ref<Cookie> Ref;

		UnicodeString::Ref name; // REF
		UnicodeString::Ref value; // REF
		// $ DateTime expire_time;
		Path domain;
		Path path;
		// $ DateTime creation_time;
		// $ DateTime last_access_time;
		bool persistent_flag;
		bool host_only_flag;
		bool secure_only_flag;
		bool http_only_flag;

		void Initialize();
		void Initialize(UnicodeString* value);

		bool Matches(Uri* url);
		bool equals(Cookie* value);
	};

	typedef std::vector<Cookie::Ref> CookieList; // REF

	enum EventType
	{
		response_headers_event = 0x1000,
		response_complete_event,
		accept_complete_event,
	};

	struct ResponseHeadersEvent : public IEvent
	{
		ByteString::Ref cookie; // REF

		virtual uint32 get_type();
	};

	struct ResponseCompleteEvent : public IEvent
	{
		ByteString::Ref cookie; // REF

		virtual uint32 get_type();
	};

	struct AcceptCompleteEvent : public IEvent
	{
		ByteString::Ref cookie; // REF

		virtual uint32 get_type();
	};
}