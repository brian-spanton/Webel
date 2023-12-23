#pragma once

#include "Basic.NameValueCollection.h"
#include "Html.Document.h"
#include "Json.Types.h"

namespace Http
{
	using namespace Basic;

	struct Request : public IRefCounted
	{
		typedef Basic::Ref<Request> Ref;

		UnicodeString::Ref method; // $$$
		Uri::Ref resource; // $$$
		UnicodeString::Ref protocol; // $$$
		NameValueCollection::Ref headers; // $$$
		Basic::Ref<ISerializable> client_body; // $$$

		void Initialize();
		void Initialize(Request* request);
	};

	struct Response : public IRefCounted
	{
		typedef Basic::Ref<Response> Ref;

		UnicodeString::Ref protocol; // $$$
		uint16 code;
		UnicodeString::Ref reason; // $$$
		NameValueCollection::Ref headers; // $$$

		Basic::Ref<ISerializable> server_body; // $$$

		void Initialize();
	};

	struct Transaction
	{
		Request::Ref request; // $$$
		Response::Ref response; // $$$
	};

	typedef std::vector<Transaction> TransactionList;

	struct Cookie : public IRefCounted
	{
		typedef Basic::Ref<Cookie> Ref;

		UnicodeString::Ref name; // $$$
		UnicodeString::Ref value; // $$$
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

	typedef std::vector<Cookie::Ref> CookieList; // $$$

	enum EventType
	{
		response_headers_event = 0x1000,
		response_complete_event,
		accept_complete_event,
	};

	struct ResponseHeadersEvent : public IEvent
	{
		ByteString::Ref cookie; // $$$

		virtual uint32 get_type();
	};

	struct ResponseCompleteEvent : public IEvent
	{
		ByteString::Ref cookie; // $$$

		virtual uint32 get_type();
	};

	struct AcceptCompleteEvent : public IEvent
	{
		ByteString::Ref cookie; // $$$

		virtual uint32 get_type();
	};
}