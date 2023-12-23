#pragma once

#include "Html.Node.h"
#include "Http.Types.h"

namespace Http
{
	using namespace Basic;

	class Globals
	{
	public:
		Globals();

		void Initialize();

		byte CR;
		byte LF;
		byte SP;
		byte HT;
		byte DQ;
		byte QM;
		byte FS;
		byte SC;
		byte EQ;

		byte CRLF[2];

		bool CHAR[0x100];
		bool UPALPHA[0x100];
		bool LOALPHA[0x100];
		bool ALPHA[0x100];
		bool DIGIT[0x100];
		bool CTL[0x100];
		bool TEXT[0x100];
		bool HEX[0x100];
		bool TOKEN[0x100];
		bool SEPARATOR[0x100];
		bool LWS[0x100];
		bool WSP[0x100];
		bool COOKIE_OCTET[0x100];

		UnicodeString::Ref header_content_length; // $$$
		UnicodeString::Ref header_connection; // $$$
		UnicodeString::Ref keep_alive; // $$$
		UnicodeString::Ref header_content_type; // $$$
		UnicodeString::Ref HTTP_1_1; // $$$
		UnicodeString::Ref header_te; // $$$
		UnicodeString::Ref trailers; // $$$
		UnicodeString::Ref header_host; // $$$
		UnicodeString::Ref reason_ok; // $$$
		UnicodeString::Ref reason_request_uri; // $$$
		UnicodeString::Ref reason_method; // $$$
		UnicodeString::Ref header_content_encoding; // $$$
		UnicodeString::Ref identity; // $$$
		UnicodeString::Ref header_transfer_encoding; // $$$
		UnicodeString::Ref header_transfer_length; // $$$
		UnicodeString::Ref chunked; // $$$
		UnicodeString::Ref header_location; // $$$
		UnicodeString::Ref header_set_cookie; // $$$
		UnicodeString::Ref expires_av_name; // $$$
		UnicodeString::Ref max_age_av_name; // $$$
		UnicodeString::Ref domain_av_name; // $$$
		UnicodeString::Ref path_av_name; // $$$
		UnicodeString::Ref secure_av_name; // $$$
		UnicodeString::Ref httponly_av_name; // $$$
		UnicodeString::Ref header_cookie; // $$$

		UnicodeString::Ref application_x_www_form_urlencoded_media_type; // $$$

		UnicodeString::Ref head_method; // $$$
		UnicodeString::Ref get_method; // $$$
		UnicodeString::Ref post_method; // $$$
	};

	extern Globals* globals;
}
