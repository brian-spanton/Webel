#pragma once

#include "Basic.Types.h"
#include "Basic.IProcess.h"
#include "Basic.IStream.h"
#include "Basic.IDecoder.h"
#include "Basic.ICompletion.h"
#include "Basic.Frame.h"
#include "Json.Types.h"
#include "Basic.Uri.h"

namespace Http
{
	class Client;
}

namespace Json
{
	class Parser;
}

namespace Basic
{
	class SingleByteEncodingIndex;

	class Globals : public Frame
	{
	private:
		enum State
		{
			single_byte_encodings_state = Start_State,
			done_state = Succeeded_State,
		};

		Basic::Ref<Http::Client, IProcess> client; // $$$
		Basic::Ref<Json::Parser, IStream<byte> > json_parser; // $$$
		Basic::Ref<IProcess> encodings_completion; // $$$
		ByteString::Ref encodings_cookie; // $$$

	public:
		typedef Basic::Ref<Globals> Ref;

		Globals();

		void Initialize();
		void InitializeEncodings(Basic::Ref<IProcess> completion, ByteString::Ref cookie);

		virtual void IProcess::Process(IEvent* event, bool* yield);

		byte utf_16_big_endian_bom[2];
		byte utf_16_little_endian_bom[2];
		byte utf_8_bom[3];

		UnicodeString::Ref ftp_scheme; // $$$
		UnicodeString::Ref file_scheme; // $$$
		UnicodeString::Ref gopher_scheme; // $$$
		UnicodeString::Ref http_scheme; // $$$
		UnicodeString::Ref https_scheme; // $$$
		UnicodeString::Ref ws_scheme; // $$$
		UnicodeString::Ref wss_scheme; // $$$

		UnicodeString::Ref utf_32_big_endian_label; // $$$
		UnicodeString::Ref utf_32_little_endian_label; // $$$
		UnicodeString::Ref utf_16_big_endian_label; // $$$
		UnicodeString::Ref utf_16_little_endian_label; // $$$
		UnicodeString::Ref utf_8_label; // $$$
		UnicodeString::Ref us_ascii_label; // $$$

		UnicodeString::Ref Name_encodings; // $$$
		UnicodeString::Ref Name_heading; // $$$
		UnicodeString::Ref heading_utf8; // $$$
		UnicodeString::Ref heading_legacy; // $$$
		UnicodeString::Ref Name_name; // $$$
		UnicodeString::Ref Name_labels; // $$$
		UnicodeString::Ref CRLF; // $$$
		UnicodeString::Ref percent_forty; // $$$
		UnicodeString::Ref percent_two_e; // $$$
		UnicodeString::Ref dot_percent_two_e; // $$$
		UnicodeString::Ref percent_two_e_dot; // $$$
		UnicodeString::Ref dot_dot; // $$$
		UnicodeString::Ref dot; // $$$

		UnicodeString::Ref text_plain_media_type; // $$$
		UnicodeString::Ref text_html_media_type; // $$$
		UnicodeString::Ref application_json_media_type; // $$$
		UnicodeString::Ref charset_parameter_name; // $$$

		EncoderMap encoder_map;
		DecoderMap decoder_map;

		Basic::Ref<SingleByteEncodingIndex, ISingleByteEncodingIndex> ascii_index; // $$$

		bool simple_encode_anti_set[0x100];
		bool default_encode_anti_set[0x100];
		bool password_encode_anti_set[0x100];
		bool username_encode_anti_set[0x100];

		typedef StringMapCaseSensitive<UnicodeString::Ref> PortMap; // $$$
		PortMap scheme_to_port_map; // $$$

		Uri::Ref encodings_url; // $$$

		bool HandleError(const char* context, uint32 error);
		void GetEncoder(UnicodeString* encoding, Basic::Ref<IEncoder>* encoder);
		void GetDecoder(UnicodeString* encoding, Basic::Ref<IDecoder>* decoder);
	};

	extern Inline<Globals>* globals;
}
