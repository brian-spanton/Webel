// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.HtmlNamedCharacterReferences.h"
#include "Html.Globals.h"
#include "Service.Types.h"

namespace Service
{
    HtmlNamedCharacterReferences::HtmlNamedCharacterReferences(std::shared_ptr<IProcess> completion, ByteStringRef cookie) :
        completion(completion),
        completion_cookie(cookie),
        client(std::make_shared<Web::Client>())
    {
        initialize_unicode(&this->codepoints_member_name, "codepoints");

        // http://www.whatwg.org/specs/web-apps/current-work/multipage/named-character-references.html#named-character-references
    }

    void HtmlNamedCharacterReferences::start()
    {
        std::shared_ptr<Uri> url = std::make_shared<Uri>();
        url->Initialize("http://www.whatwg.org/specs/web-apps/current-work/multipage/entities.json");

        // keep ourself alive until we decide to self-destruct
        this->self = this->shared_from_this();

        this->client->Get(url, 0, this->self, ByteStringRef());
    }

    void HtmlNamedCharacterReferences::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            // so we don't leak ourself
            this->self.reset();
        }
    }

    void HtmlNamedCharacterReferences::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::named_character_reference_state:
            switch(event->get_type())
            {
            case Http::EventType::response_headers_event:
                {
                    std::shared_ptr<Http::Response> response = this->client->history.back().response;
                    if (response->code != 200)
                    {
                        std::shared_ptr<Uri> url;
                        this->client->get_url(&url);

                        url->write_to_stream(Basic::globals->LogStream(), 0, 0);
                        Basic::globals->DebugWriter()->WriteLine(" did not return 200");

                        switch_to_state(State::done_state);
                        return;
                    }

                    UnicodeStringRef charset;
                    bool success = this->client->get_content_type_charset(&charset);

                    this->json_parser = std::make_shared<Json::Parser>();
                    this->json_parser->Initialize(std::shared_ptr<Html::Node>(), charset);

                    this->client->set_body_stream(this->json_parser);

                    throw Yield("event consumed");
                }
                break;

            case Http::EventType::response_complete_event:
                {
                    if (this->client->history.size() == 0)
                        throw FatalError("was Yield... expecting to have this completed request in our history...");

                    std::shared_ptr<Http::Response> response = this->client->history.back().response;

                    if (this->json_parser->text->value->type != Json::Value::Type::object_value)
                        throw FatalError("was Yield... expecting the json body to be an object type");

                    std::shared_ptr<Json::Object> root = std::static_pointer_cast<Json::Object>(this->json_parser->text->value);

                    for (Json::MemberList::iterator mapping = root->members.begin(); mapping != root->members.end(); mapping++)
                    {
                        if (mapping->first->size() < 2)
                            continue;

                        if (mapping->first->at(0) != '&')
                            continue;

                        if (mapping->second->type != Json::Value::Type::object_value)
                            continue;

                        std::shared_ptr<Json::Object> line = std::static_pointer_cast<Json::Object>(mapping->second);

                        Json::MemberList::iterator member = line->members.find(codepoints_member_name);
                        if (member == line->members.end())
                            continue;

                        if (member->second->type != Json::Value::Type::array_value)
                            continue;

                        std::shared_ptr<Json::Array> codepoints = std::static_pointer_cast<Json::Array>(member->second);

                        UnicodeStringRef unicode_name = std::make_shared<UnicodeString>();
                        unicode_name->append(mapping->first->cbegin() + 1, mapping->first->cend());

                        UnicodeStringRef list = std::make_shared<UnicodeString>();

                        for (Json::ValueList::iterator codepoint = codepoints->elements.begin(); codepoint != codepoints->elements.end(); codepoint++)
                        {
                            if ((*codepoint)->type != Json::Value::Type::number_value)
                                continue;

                            std::shared_ptr<Json::Number> number = std::static_pointer_cast<Json::Number>(*codepoint);
                            list->push_back((Codepoint)number->value);
                        }

                        if (list->size() == 0)
                            continue;

                        StringMap::value_type value(unicode_name, list);
                        Html::globals->named_character_references_table->emplace(std::move(value));
                    }

                    Basic::globals->DebugWriter()->WriteFormat<0x100>("Recognized %d HTML named character references\n", Html::globals->named_character_references_table->size());

                    std::shared_ptr<IProcess> completion = this->completion.lock();
                    if (completion.get() != 0)
                    {
                        CharactersCompleteEvent event;
                        event.cookie = this->completion_cookie;
                        produce_event(completion.get(), &event);
                    }

                    switch_to_state(State::done_state);
                }
                break;

            default:
                throw FatalError("Html::Globals::Complete unexpected event");
            }
            break;

        default:
            throw FatalError("Html::Globals::Complete unexpected state");
        }
    }
}