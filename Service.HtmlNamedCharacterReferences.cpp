// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.HtmlNamedCharacterReferences.h"
#include "Html.Globals.h"
#include "Service.Types.h"

namespace Service
{
	HtmlNamedCharacterReferences::HtmlNamedCharacterReferences()
	{
	}

	void HtmlNamedCharacterReferences::Initialize(Basic::Ref<IProcess> completion, ByteString::Ref cookie)
	{
		__super::Initialize();

		this->codepoints_member_name.Initialize("codepoints");

		this->characters_completion = completion;
		this->characters_cookie = cookie;

		this->client = New<Web::Client>();
		this->client->Initialize();

		// http://www.whatwg.org/specs/web-apps/current-work/multipage/named-character-references.html#named-character-references

		Http::Uri::Ref url = New<Http::Uri>();
		url->Initialize("http://www.whatwg.org/specs/web-apps/current-work/multipage/entities.json");

		this->client->Get(url, this, (ByteString*)0);
	}

	void HtmlNamedCharacterReferences::Process(IEvent* event, bool* yield)
	{
		(*yield) = true;

		switch (frame_state())
		{
		case State::named_character_reference_state:
			switch(event->get_type())
			{
			case Http::EventType::response_headers_event:
				{
					Http::Response::Ref response = this->client->history.back().response;
					if (response->code != 200)
					{
						Uri::Ref url;
						this->client->get_url(&url);

						url->SerializeTo(Basic::globals->DebugStream(), 0, 0);
						Basic::globals->DebugWriter()->WriteLine(" did not return 200");

						switch_to_state(State::done_state);
						break;
					}

					UnicodeString::Ref charset;
					bool success = this->client->get_content_type_charset(&charset);

					this->json_parser = New<Json::Parser>();
					this->json_parser->Initialize((Html::Node*)0, charset);

					this->client->set_body_stream(this->json_parser);
				}
				break;

			case Http::EventType::response_complete_event:
				{
					if (this->client->history.size() == 0)
						break;

					Http::Response::Ref response = this->client->history.back().response;

					if (this->json_parser->text->value->type != Json::Value::Type::object_value)
						break;

					Json::Object::Ref root = (Json::Object*)this->json_parser->text->value.item();

					for (Json::MemberList::iterator mapping = root->members.begin(); mapping != root->members.end(); mapping++)
					{
						if (mapping->first->size() < 2)
							continue;

						if (mapping->first->at(0) != '&')
							continue;

						if (mapping->second->type != Json::Value::Type::object_value)
							continue;

						Json::Object::Ref line = (Json::Object*)mapping->second.item();

						Json::MemberList::iterator member = line->members.find(codepoints_member_name);
						if (member == line->members.end())
							continue;

						if (member->second->type != Json::Value::Type::array_value)
							continue;

						Json::Array::Ref codepoints = (Json::Array*)member->second.item();

						UnicodeString::Ref unicode_name = New<UnicodeString>();
						unicode_name->append(mapping->first->cbegin() + 1, mapping->first->cend());

						UnicodeString::Ref list = New<UnicodeString>();

						for (Json::ValueList::iterator codepoint = codepoints->elements.begin(); codepoint != codepoints->elements.end(); codepoint++)
						{
							if ((*codepoint)->type != Json::Value::Type::number_value)
								continue;

							Json::Number::Ref number = (Json::Number*)codepoint->item();
							list->push_back((Codepoint)number->value);
						}

						if (list->size() == 0)
							continue;

						StringMap::value_type value(unicode_name, list);
						Html::globals->named_character_references_table->emplace(std::move(value));
					}

					Basic::globals->DebugWriter()->WriteFormat<0x100>("Recognized %d HTML named character references\n", Html::globals->named_character_references_table->size());

					Basic::Ref<IProcess> completion = this->characters_completion;
					this->characters_completion = 0;

					CharactersCompleteEvent event;
					event.cookie = this->characters_cookie;
					this->characters_cookie = 0;

					if (completion.item() != 0)
						completion->Process(&event);

					switch_to_state(State::done_state);
				}
				break;

			default:
				throw new Exception("Html::Globals::Complete unexpected event");
			}
			break;

		default:
			throw new Exception("Html::Globals::Complete unexpected state");
		}
	}
}