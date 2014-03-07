// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IErrorHandler.h"
#include "Basic.ICompletionQueue.h"

namespace Basic
{
	class SingleByteEncodingIndex;

	class Globals : public IErrorHandler, public ICompletionQueue
	{
	public:
		typedef Basic::Ref<Globals> Ref;

		Globals();

		void Initialize(Basic::Ref<IErrorHandler> error_handler, Basic::Ref<ICompletionQueue> completion_queue);
		bool InitializeSocketApi();

		byte utf_16_big_endian_bom[2];
		byte utf_16_little_endian_bom[2];
		byte utf_8_bom[3];

		UnicodeString::Ref ftp_scheme; // REF
		UnicodeString::Ref file_scheme; // REF
		UnicodeString::Ref gopher_scheme; // REF
		UnicodeString::Ref http_scheme; // REF
		UnicodeString::Ref https_scheme; // REF
		UnicodeString::Ref ws_scheme; // REF
		UnicodeString::Ref wss_scheme; // REF

		UnicodeString::Ref utf_32_big_endian_label; // REF
		UnicodeString::Ref utf_32_little_endian_label; // REF
		UnicodeString::Ref utf_16_big_endian_label; // REF
		UnicodeString::Ref utf_16_little_endian_label; // REF
		UnicodeString::Ref utf_8_label; // REF
		UnicodeString::Ref us_ascii_label; // REF

		UnicodeString::Ref CRLF; // REF
		UnicodeString::Ref percent_forty; // REF
		UnicodeString::Ref percent_two_e; // REF
		UnicodeString::Ref dot_percent_two_e; // REF
		UnicodeString::Ref percent_two_e_dot; // REF
		UnicodeString::Ref dot_dot; // REF
		UnicodeString::Ref dot; // REF

		UnicodeString::Ref text_plain_media_type; // REF
		UnicodeString::Ref text_html_media_type; // REF
		UnicodeString::Ref application_json_media_type; // REF
		UnicodeString::Ref charset_parameter_name; // REF

		EncoderMap encoder_map;
		DecoderMap decoder_map;

		Basic::Ref<SingleByteEncodingIndex, ISingleByteEncodingIndex> ascii_index; // REF

		bool simple_encode_anti_set[0x100];
		bool default_encode_anti_set[0x100];
		bool password_encode_anti_set[0x100];
		bool username_encode_anti_set[0x100];

		typedef StringMapCaseSensitive<UnicodeString::Ref> PortMap; // REF
		PortMap scheme_to_port_map; // REF

		bool sanitizer_white_space[0x100];

		Basic::Ref<IErrorHandler> error_handler;
		Basic::Ref<ICompletionQueue> completion_queue;

		LPFN_CONNECTEX ConnectEx;

		void GetEncoder(UnicodeString* encoding, Basic::Ref<IEncoder>* encoder);
		void GetDecoder(UnicodeString* encoding, Basic::Ref<IDecoder>* decoder);

		virtual bool IErrorHandler::HandleError(const char* context, uint32 error);
		virtual Basic::IStream<Codepoint>* DebugStream();
		virtual Basic::TextWriter* DebugWriter();

		virtual void ICompletionQueue::BindToCompletionQueue(Socket::Ref socket);
		virtual void ICompletionQueue::BindToCompletionQueue(LogFile::Ref socket);
		virtual void ICompletionQueue::PostCompletion(Basic::ICompletion* completion, LPOVERLAPPED overlapped);
		virtual void ICompletionQueue::QueueProcess(Basic::Ref<IProcess> process, ByteString::Ref cookie);
	};

	extern Inline<Globals>* globals;
}
