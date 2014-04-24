// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.StandardEncodings.h"
#include "Service.Globals.h"
#include "Basic.Utf8Decoder.h"
#include "Service.StandardSingleByteEncoding.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.SingleByteEncoder.h"

namespace Service
{
    StandardEncodings::StandardEncodings()
    {
    }

    void StandardEncodings::Initialize(Basic::Ref<IProcess> completion, ByteString::Ref cookie)
    {
        __super::Initialize();

        Name_encodings.Initialize("encodings");
        Name_heading.Initialize("heading");
        heading_utf8.Initialize("The Encoding");
        heading_legacy.Initialize("Legacy single-byte encodings");
        Name_name.Initialize("name");
        Name_labels.Initialize("labels");

        encodings_url = New<Uri>();
        encodings_url->Initialize("http://encoding.spec.whatwg.org/encodings.json");

        this->encodings_completion = completion;
        this->encodings_cookie = cookie;

        this->client = New<Web::Client>();
        this->client->Initialize();
        this->client->Get(encodings_url, this, (ByteString*)0);
    }

    void StandardEncodings::Process(IEvent* event, bool* yield)
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

                        url->SerializeTo(Service::globals->DebugStream(), 0, 0);
                        Service::globals->DebugWriter()->WriteLine(" did not return 200");

                        switch_to_state(State::done_state);
                        break;
                    }

                    UnicodeString::Ref charset;
                    this->client->get_content_type_charset(&charset);

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
                                    Basic::globals->decoder_map.insert(DecoderMap::value_type(label_string->value, factory.item()));
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

                                    if (label_string->value->equals<false>(Basic::globals->us_ascii_label))
                                    {
                                        found_ascii = true;
                                        index = Basic::globals->ascii_index;
                                    }
                                }

                                if (index.item() == 0)
                                {
                                    index = New<SingleByteEncodingIndex>();
                                    index->Initialize();
                                }

                                StandardSingleByteEncoding::Ref standard_encoding = New<StandardSingleByteEncoding>();
                                standard_encoding->Initialize(index_url, index);

                                for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
                                {
                                    if ((*label_it)->type != Json::Value::Type::string_value)
                                        continue;

                                    Json::String::Ref label_string = (Json::String*)label_it->item();

                                    SingleByteEncoderFactory::Ref encoder_factory = New<SingleByteEncoderFactory>();
                                    encoder_factory->Initialize(index);
                                    Basic::globals->encoder_map.insert(EncoderMap::value_type(label_string->value, encoder_factory.item()));

                                    SingleByteDecoderFactory::Ref decoder_factory = New<SingleByteDecoderFactory>();
                                    decoder_factory->Initialize(index);
                                    Basic::globals->decoder_map.insert(DecoderMap::value_type(label_string->value, decoder_factory.item()));
                                }
                            }
                        }
                    }

                    if (found_ascii == false)
                        throw new Exception("didn't find us-ascii encoding");

                    Service::globals->DebugWriter()->WriteFormat<0x100>("Recognized %d encodings\n", Basic::globals->decoder_map.size());

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
