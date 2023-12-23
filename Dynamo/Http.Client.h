#pragma once

#include "Http.ResponseHeadersFrame.h"
#include "Http.BodyFrame.h"
#include "Basic.Frame.h"
#include "Basic.Lock.h"
#include "Basic.ICompletion.h"
#include "Basic.IBufferedStream.h"

namespace Http
{
	using namespace Basic;

	class Client : public Frame
	{
	public:
		enum State
		{
			inactive_state = Start_State,
			get_pending_state,
			resolve_address_state,
			connection_pending_state,
			headers_pending_state,
			body_pending_state,
			response_complete_state,
		};

	private:
		Basic::Ref<IBufferedStream<byte> > peer; // $$$
		Basic::Ref<IProcess> client_completion; // $$$
		ByteString::Ref client_cookie; // $$$
		Inline<ResponseHeadersFrame> response_headers_frame;
		Inline<BodyFrame> response_body_frame;
		MediaType::Ref media_type; // $$$
		uint8 retries;
		uint8 redirects;
		Request::Ref planned_request; // $$$
		Lock lock;

		void switch_to_state(State state);
		void Error(const char* error);
		void Redirect(Uri* url);
		void Retry(Request* request);
		void QueuePlanned();

	public:
		typedef Basic::Ref<Client, IProcess> Ref;

		TransactionList history;
		CookieList http_cookies;

		void Initialize();

		void Get(Request* request, Basic::Ref<IProcess> completion, ByteString::Ref cookie);
		void Get(Uri* url, Basic::Ref<IProcess> completion, ByteString::Ref cookie);

		virtual void IProcess::Process(IEvent* event);
		virtual void IProcess::Process(IEvent* event, bool* yield);

		bool get_content_type(MediaType::Ref* media_type);
		bool get_content_type_charset(UnicodeString::Ref* media_type);
		void set_body_stream(IStream<byte>* body_stream);
		void get_url(Uri::Ref* url);
	};
}