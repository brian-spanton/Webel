#pragma once

#include "Basic.ListenSocket.h"
#include "Http.RequestFrame.h"

namespace Http
{
	class Server : public Frame
	{
	private:
		enum State
		{
			pending_connection_state = Start_State,
			new_request_state,
			request_frame_pending_state,
			response_done_state,
			done_state = Succeeded_State,
			request_frame_failed,
		};

		Request::Ref request; // $$$
		Response::Ref response; // $$$
		Basic::Ref<IBufferedStream<byte> > peer; // $$$
		Inline<RequestFrame> request_frame;
		Basic::Ref<IProcess> accept_completion; // $$$
		ByteString::Ref accept_cookie; // $$$

		static void AdminRequest(Request* request, Response* response);
		static bool EchoRequest(Request* request, Response* response);
		static bool QuestionRequest(Request* request, Response* response);

		void switch_to_state(State state);

	public:
		typedef Basic::Ref<Server, IProcess> Ref;

		void Initialize(ListenSocket* listen_socket, bool secure, Basic::Ref<IProcess> completion, ByteString::Ref cookie);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}