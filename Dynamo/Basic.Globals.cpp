#include "stdafx.h"
#include "Dynamo.Globals.h"
#include "Basic.ClientSocket.h"
#include "Tls.CertificatesFrame.h"
#include "Http.Client.h"
#include "Basic.IDecoder.h"
#include "Basic.Utf32LittleEndianDecoder.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.SingleByteDecoder.h"
#include "Json.Parser.h"

namespace Basic
{
	Inline<Globals>* globals = 0;

	Globals::Globals()
	{
	}

	bool Globals::HandleError(const char* context, uint32 error)
	{
		Dynamo::globals->DebugWriter()->Write("ERROR: ");
		Dynamo::globals->DebugWriter()->Write(context);
		Dynamo::globals->DebugWriter()->Write(" code=");
		Dynamo::globals->DebugWriter()->WriteError(error);
		Dynamo::globals->DebugWriter()->WriteLine();
		return false;
	}

	void Globals::GetEncoder(UnicodeString* encoding, Basic::Ref<IEncoder>* encoder)
	{
		EncoderMap::iterator it = this->encoder_map.find(encoding);
		if (it == this->encoder_map.end())
		{
			(*encoder) = 0;
			return;
		}

		it->second->CreateEncoder(encoder);
	}

	void Globals::GetDecoder(UnicodeString* encoding, Basic::Ref<IDecoder>* decoder)
	{
		DecoderMap::iterator it = this->decoder_map.find(encoding);
		if (it == this->decoder_map.end())
		{
			(*decoder) = 0;
			return;
		}

		it->second->CreateDecoder(decoder);
	}

	void Globals::Initialize()
	{
		__super::Initialize();

		this->ascii_index = New<SingleByteEncodingIndex>();

		utf_16_big_endian_bom[0] = 0xFE;
		utf_16_big_endian_bom[1] = 0xFF;

		utf_16_little_endian_bom[0] = 0xFF;
		utf_16_little_endian_bom[1] = 0xFE;

		utf_8_bom[0] = 0xEF;
		utf_8_bom[1] = 0xBB;
		utf_8_bom[2] = 0xBF;

		ftp_scheme.Initialize("ftp");
		file_scheme.Initialize("file");
		gopher_scheme.Initialize("gopher");
		http_scheme.Initialize("http");
		https_scheme.Initialize("https");
		ws_scheme.Initialize("ws");
		wss_scheme.Initialize("wss");

		utf_32_big_endian_label.Initialize("utf-32be");
		utf_32_little_endian_label.Initialize("utf-32le");
		utf_16_big_endian_label.Initialize("utf-16be");
		utf_16_little_endian_label.Initialize("utf-16le");
		utf_8_label.Initialize("utf-8");
		us_ascii_label.Initialize("us-ascii");

		Name_encodings.Initialize("encodings");
		Name_heading.Initialize("heading");
		heading_utf8.Initialize("The Encoding");
		heading_legacy.Initialize("Legacy single-byte encodings");
		Name_name.Initialize("name");
		Name_labels.Initialize("labels");
		CRLF.Initialize("\r\n");
		percent_forty.Initialize("%40");
		percent_two_e.Initialize("%2e");
		dot_percent_two_e.Initialize(".%2e");
		percent_two_e_dot.Initialize("%2e.");
		dot_dot.Initialize("..");
		dot.Initialize(".");

		text_plain_media_type.Initialize("text/plain");
		text_html_media_type.Initialize("text/html");
		application_json_media_type.Initialize("application/json");
		charset_parameter_name.Initialize("charset");

		ZeroMemory(this->simple_encode_anti_set, sizeof(this->simple_encode_anti_set));

		for (Codepoint c = 0x0020; c <= 0x007E; c++)
			this->simple_encode_anti_set[c] = true;

		CopyMemory(this->default_encode_anti_set, this->simple_encode_anti_set, sizeof(this->default_encode_anti_set));

		this->default_encode_anti_set[0x0020] = false;
		this->default_encode_anti_set['\"'] = false;
		this->default_encode_anti_set['#'] = false;
		this->default_encode_anti_set['<'] = false;
		this->default_encode_anti_set['>'] = false;
		this->default_encode_anti_set['?'] = false;
		this->default_encode_anti_set['`'] = false;

		CopyMemory(this->password_encode_anti_set, this->default_encode_anti_set, sizeof(this->password_encode_anti_set));

		this->password_encode_anti_set['/'] = false;
		this->password_encode_anti_set['@'] = false;
		this->password_encode_anti_set['\\'] = false;

		CopyMemory(this->username_encode_anti_set, this->password_encode_anti_set, sizeof(this->username_encode_anti_set));

		this->username_encode_anti_set[':'] = false;

		UnicodeString::Ref port;

		port.Initialize("21");
		this->scheme_to_port_map.insert(PortMap::value_type(this->ftp_scheme, port));

		port.Initialize("");
		this->scheme_to_port_map.insert(PortMap::value_type(this->file_scheme, port));

		port.Initialize("70");
		this->scheme_to_port_map.insert(PortMap::value_type(this->gopher_scheme, port));

		port.Initialize("80");
		this->scheme_to_port_map.insert(PortMap::value_type(this->http_scheme, port));
		this->scheme_to_port_map.insert(PortMap::value_type(this->ws_scheme, port));

		port.Initialize("443");
		this->scheme_to_port_map.insert(PortMap::value_type(this->https_scheme, port));
		this->scheme_to_port_map.insert(PortMap::value_type(this->wss_scheme, port));

		Utf32LittleEndianDecoderFactory::Ref utf_32_little_endian_decoder_factory = New<Utf32LittleEndianDecoderFactory>();
		this->decoder_map.insert(DecoderMap::value_type(utf_32_little_endian_label, utf_32_little_endian_decoder_factory.item()));

		Utf8DecoderFactory::Ref utf_8_decoder_factory = New<Utf8DecoderFactory>();
		this->decoder_map.insert(DecoderMap::value_type(utf_8_label, utf_8_decoder_factory.item()));

		Utf8EncoderFactory::Ref utf_8_encoder_factory = New<Utf8EncoderFactory>();
		this->encoder_map.insert(EncoderMap::value_type(utf_8_label, utf_8_encoder_factory.item()));

		encodings_url = New<Http::Uri>();
		encodings_url->Initialize("http://encoding.spec.whatwg.org/encodings.json");
	}

	void Globals::InitializeEncodings(Basic::Ref<IProcess> completion, ByteString::Ref cookie)
	{
		this->encodings_completion = completion;
		this->encodings_cookie = cookie;

		this->client = New<Http::Client>();
		this->client->Initialize();
		this->client->Get(encodings_url, this, (ByteString*)0);
	}

	void Globals::Process(IEvent* event, bool* yield)
	{
		(*yield) = true;

		bool found_ascii = false;

		switch (frame_state())
		{
		case State::single_byte_encodings_state:
			switch (event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					Http::Response::Ref response = this->client->history.back().response;
					if (response->code != 200)
					{
						Uri::Ref url;
						this->client->get_url(&url);

						url->SerializeTo(Dynamo::globals->DebugStream(), 0, 0);
						Dynamo::globals->DebugWriter()->WriteLine(" did not return 200");

						switch_to_state(State::done_state);
						break;
					}

					this->json_parser = New<Json::Parser>();
					this->json_parser->Initialize((Html::Node*)0, this->client);

					this->client->set_body_stream(this->json_parser);
				}
				break;

			case Http::EventType::response_complete_event:
				{
					if (this->client->history.size() == 0)
						break;

					Http::Response::Ref response = this->client->history.back().response;

					Uri::Ref current_url;
					this->client->get_url(&current_url);

					if (this->json_parser->text->value->type != Json::Value::Type::array_value)
						break;

					Json::Array::Ref root = (Json::Array*)this->json_parser->text->value.item();

					for (Json::ValueList::iterator family_it = root->elements.begin(); family_it != root->elements.end(); family_it++)
					{
						if ((*family_it)->type != Json::Value::Type::object_value)
							continue;

						Json::Object::Ref family = (Json::Object*)family_it->item();

						Json::MemberList::iterator heading_it = family->members.find(Name_heading);
						if (heading_it == family->members.end())
							continue;

						if (heading_it->second->type != Json::Value::string_value)
							continue;

						Json::String::Ref heading = (Json::String*)heading_it->second.item();

						if (heading->value.equals<true>(heading_utf8))
						{
							Json::MemberList::iterator encodings_it = family->members.find(Name_encodings);
							if (encodings_it == family->members.end())
								continue;

							Json::Array::Ref encodings = (Json::Array*)encodings_it->second.item();

							for (Json::ValueList::iterator encoding_it = encodings->elements.begin(); encoding_it != encodings->elements.end(); encoding_it++)
							{
								if ((*encoding_it)->type != Json::Value::Type::object_value)
									continue;

								Json::Object::Ref encoding = (Json::Object*)encoding_it->item();

								Json::MemberList::iterator labels_it = encoding->members.find(Name_labels);
								if (labels_it == encoding->members.end())
									continue;

								if (labels_it->second->type != Json::Value::Type::array_value)
									continue;

								Json::Array::Ref labels = (Json::Array*)labels_it->second.item();

								Utf8DecoderFactory::Ref factory = New<Utf8DecoderFactory>();

								for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
								{
									if ((*label_it)->type != Json::Value::Type::string_value)
										continue;

									Json::String::Ref label_string = (Json::String*)label_it->item();
									this->decoder_map.insert(DecoderMap::value_type(label_string->value, factory.item()));
								}
							}
						}
						else if (heading->value.equals<true>(heading_legacy))
						{
							Json::MemberList::iterator encodings_it = family->members.find(Name_encodings);
							if (encodings_it == family->members.end())
								continue;

							Json::Array::Ref encodings = (Json::Array*)encodings_it->second.item();

							for (Json::ValueList::iterator encoding_it = encodings->elements.begin(); encoding_it != encodings->elements.end(); encoding_it++)
							{
								if ((*encoding_it)->type != Json::Value::Type::object_value)
									continue;

								Json::Object::Ref encoding = (Json::Object*)encoding_it->item();

								Json::MemberList::iterator name_it = encoding->members.find(Name_name);
								if (name_it == encoding->members.end())
									continue;

								if (name_it->second->type != Json::Value::Type::string_value)
									continue;

								Json::String::Ref name_string = (Json::String*)name_it->second.item();

								UnicodeString::Ref file_name = New<UnicodeString>();
								file_name.Initialize("index-.txt");
								file_name->insert(file_name->begin() + 6, name_string->value->begin(), name_string->value->end());

								Http::Uri::Ref index_url = New<Http::Uri>();
								index_url->Initialize();

								bool success = index_url->Parse(file_name, current_url);
								if (!success)
									throw new Exception("url parse failed");

								Json::MemberList::iterator labels_it = encoding->members.find(Name_labels);
								if (labels_it == encoding->members.end())
									continue;

								if (labels_it->second->type != Json::Value::Type::array_value)
									continue;

								Json::Array::Ref labels = (Json::Array*)labels_it->second.item();

								SingleByteEncodingIndex::Ref index;

								for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
								{
									if ((*label_it)->type != Json::Value::Type::string_value)
										continue;

									Json::String::Ref label_string = (Json::String*)label_it->item();

									if (label_string->value->equals<false>(this->us_ascii_label))
									{
										found_ascii = true;
										index = Basic::globals->ascii_index;
									}
								}

								if (index.item() == 0)
								{
									index = New<SingleByteEncodingIndex>();
								}

								index->Initialize(index_url);

								for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
								{
									if ((*label_it)->type != Json::Value::Type::string_value)
										continue;

									Json::String::Ref label_string = (Json::String*)label_it->item();

									SingleByteEncoderFactory::Ref encoder_factory = New<SingleByteEncoderFactory>();
									encoder_factory->Initialize(index);
									this->encoder_map.insert(EncoderMap::value_type(label_string->value, encoder_factory.item()));

									SingleByteDecoderFactory::Ref decoder_factory = New<SingleByteDecoderFactory>();
									decoder_factory->Initialize(index);
									this->decoder_map.insert(DecoderMap::value_type(label_string->value, decoder_factory.item()));
								}
							}
						}
					}

					if (found_ascii == false)
						throw new Exception("didn't find us-ascii encoding");

					Dynamo::globals->DebugWriter()->WriteFormat<0x100>("Recognized %d encodings\n", this->decoder_map.size());

					Basic::Ref<IProcess> completion = this->encodings_completion;
					this->encodings_completion = 0;

					EncodingsCompleteEvent event;
					event.cookie = this->encodings_cookie;
					this->encodings_cookie = 0;

					if (completion.item() != 0)
						completion->Process(&event);

					switch_to_state(State::done_state);
				}
				break;

			default:
				throw new Exception("unexpected event");
			}
			break;

		default:
			throw new Exception("Globals::Complete unexpected state");
		}
	}
}
