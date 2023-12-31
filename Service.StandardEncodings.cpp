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
    StandardEncodings::StandardEncodings(std::shared_ptr<IProcess> completion, ByteStringRef cookie) :
        completion(completion),
        completion_cookie(cookie),
        client(std::make_shared<Web::Client>())
    {
        initialize_unicode(&Name_encodings, "encodings");
        initialize_unicode(&Name_heading, "heading");
        initialize_unicode(&heading_utf8, "The Encoding");
        initialize_unicode(&heading_legacy, "Legacy single-byte encodings");
        initialize_unicode(&Name_name, "name");
        initialize_unicode(&Name_labels, "labels");
    }

    void StandardEncodings::start()
    {
        std::shared_ptr<Uri> encodings_url = std::make_shared<Uri>();
        encodings_url->Initialize("https://encoding.spec.whatwg.org/encodings.json");

        // keep ourself alive until we decide to self-destruct
        this->self = this->shared_from_this();

        this->client->Get(encodings_url, 0, this->self, ByteStringRef());
    }

    void StandardEncodings::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            // so we don't leak ourself
            this->self.reset();
        }
    }

    EventResult StandardEncodings::consider_event(IEvent* event)
    {
        bool found_ascii = false;

        switch (get_state())
        {
        case State::single_byte_encodings_state:
            switch (event->get_type())
            {
            case Http::EventType::response_headers_event:
                {
                    if (this->client->transaction->response->code != 200)
                    {
                        std::shared_ptr<Uri> url;
                        this->client->get_url(&url);

                        url->write_to_stream(Service::globals->LogStream(), 0, 0);
                        Service::globals->DebugWriter()->WriteFormat<0x40>(" returned %d", this->client->transaction->response->code);
                        Service::globals->DebugWriter()->WriteLine();

                        switch_to_state(State::done_state);
                        return EventResult::event_result_continue;
                    }

                    UnicodeStringRef charset;
                    this->client->get_content_type_charset(&charset);

                    this->json_parser = std::make_shared<Json::Parser>(std::shared_ptr<Html::Node>(), charset);

                    this->client->set_decoded_content_stream(this->json_parser);

                    return EventResult::event_result_yield; // event consumed
                }
                break;

            case Http::EventType::response_complete_event:
                {
                    if (this->json_parser->text->value->type != Json::Value::Type::array_value)
                        throw FatalError("was Yield... expecting the json body to be an array type");

                    std::shared_ptr<Json::Array> root = std::static_pointer_cast<Json::Array>(this->json_parser->text->value);

                    std::shared_ptr<Uri> current_url;
                    this->client->get_url(&current_url);

                    for (Json::ValueList::iterator family_it = root->elements.begin(); family_it != root->elements.end(); family_it++)
                    {
                        if ((*family_it)->type != Json::Value::Type::object_value)
                            continue;

                        std::shared_ptr<Json::Object> family = std::static_pointer_cast<Json::Object>(*family_it);

                        Json::MemberList::iterator heading_it = family->members.find(Name_heading);
                        if (heading_it == family->members.end())
                            continue;

                        if (heading_it->second->type != Json::Value::string_value)
                            continue;

                        std::shared_ptr<Json::String> heading = std::static_pointer_cast<Json::String>(heading_it->second);

                        if (equals<UnicodeString, true>(heading->value.get(), heading_utf8.get()))
                        {
                            Json::MemberList::iterator encodings_it = family->members.find(Name_encodings);
                            if (encodings_it == family->members.end())
                                continue;

                            std::shared_ptr<Json::Array> encodings = std::static_pointer_cast<Json::Array>(encodings_it->second);

                            for (Json::ValueList::iterator encoding_it = encodings->elements.begin(); encoding_it != encodings->elements.end(); encoding_it++)
                            {
                                if ((*encoding_it)->type != Json::Value::Type::object_value)
                                    continue;

                                std::shared_ptr<Json::Object> encoding = std::static_pointer_cast<Json::Object>(*encoding_it);

                                Json::MemberList::iterator labels_it = encoding->members.find(Name_labels);
                                if (labels_it == encoding->members.end())
                                    continue;

                                if (labels_it->second->type != Json::Value::Type::array_value)
                                    continue;

                                std::shared_ptr<Json::Array> labels = std::static_pointer_cast<Json::Array>(labels_it->second);

                                std::shared_ptr<Utf8DecoderFactory> factory = std::make_shared<Utf8DecoderFactory>();

                                for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
                                {
                                    if ((*label_it)->type != Json::Value::Type::string_value)
                                        continue;

                                    std::shared_ptr<Json::String> label_string = std::static_pointer_cast<Json::String>(*label_it);
                                    Basic::globals->decoder_map.insert(DecoderMap::value_type(label_string->value, factory));
                                }
                            }
                        }
                        else if (equals<UnicodeString, true>(heading->value.get(), heading_legacy.get()))
                        {
                            Json::MemberList::iterator encodings_it = family->members.find(Name_encodings);
                            if (encodings_it == family->members.end())
                                continue;

                            std::shared_ptr<Json::Array> encodings = std::static_pointer_cast<Json::Array>(encodings_it->second);

                            for (Json::ValueList::iterator encoding_it = encodings->elements.begin(); encoding_it != encodings->elements.end(); encoding_it++)
                            {
                                if ((*encoding_it)->type != Json::Value::Type::object_value)
                                    continue;

                                std::shared_ptr<Json::Object> encoding = std::static_pointer_cast<Json::Object>(*encoding_it);

                                Json::MemberList::iterator name_it = encoding->members.find(Name_name);
                                if (name_it == encoding->members.end())
                                    continue;

                                if (name_it->second->type != Json::Value::Type::string_value)
                                    continue;

                                std::shared_ptr<Json::String> name_string = std::static_pointer_cast<Json::String>(name_it->second);

                                UnicodeStringRef file_name = std::make_shared<UnicodeString>();
                                initialize_unicode(&file_name, "index-.txt");
                                file_name->insert(file_name->begin() + 6, name_string->value->begin(), name_string->value->end());

                                std::shared_ptr<Uri> index_url = std::make_shared<Uri>();
                                index_url->Initialize();

                                bool success = index_url->Parse(file_name.get(), current_url.get());
                                if (!success)
                                    throw FatalError("url parse failed");

                                Json::MemberList::iterator labels_it = encoding->members.find(Name_labels);
                                if (labels_it == encoding->members.end())
                                    continue;

                                if (labels_it->second->type != Json::Value::Type::array_value)
                                    continue;

                                std::shared_ptr<Json::Array> labels = std::static_pointer_cast<Json::Array>(labels_it->second);

                                std::shared_ptr<SingleByteEncodingIndex> index;

                                for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
                                {
                                    if ((*label_it)->type != Json::Value::Type::string_value)
                                        continue;

                                    std::shared_ptr<Json::String> label_string = std::static_pointer_cast<Json::String>(*label_it);

                                    if (equals<UnicodeString, false>(label_string->value.get(), Basic::globals->us_ascii_label.get()))
                                    {
                                        found_ascii = true;
                                        index = Basic::globals->ascii_index;
                                    }
                                }

                                if (index.get() == 0)
                                {
                                    index = std::make_shared<SingleByteEncodingIndex>();
                                    index->Initialize();
                                }

                                std::shared_ptr<StandardSingleByteEncoding> standard_encoding = std::make_shared<StandardSingleByteEncoding>(index);
                                standard_encoding->start(index_url);

                                for (Json::ValueList::iterator label_it = labels->elements.begin(); label_it != labels->elements.end(); label_it++)
                                {
                                    if ((*label_it)->type != Json::Value::Type::string_value)
                                        continue;

                                    std::shared_ptr<Json::String> label_string = std::static_pointer_cast<Json::String>(*label_it);

                                    std::shared_ptr<SingleByteEncoderFactory> encoder_factory = std::make_shared<SingleByteEncoderFactory>();
                                    encoder_factory->Initialize(index);
                                    Basic::globals->encoder_map.insert(EncoderMap::value_type(label_string->value, encoder_factory));

                                    std::shared_ptr<SingleByteDecoderFactory> decoder_factory = std::make_shared<SingleByteDecoderFactory>();
                                    decoder_factory->Initialize(index);
                                    Basic::globals->decoder_map.insert(DecoderMap::value_type(label_string->value, decoder_factory));
                                }
                            }
                        }
                    }

                    if (found_ascii == false)
                        throw FatalError("didn't find us-ascii encoding");

                    Service::globals->DebugWriter()->WriteFormat<0x100>("Recognized %d encodings\n", Basic::globals->decoder_map.size());

                    std::shared_ptr<IProcess> completion = this->completion.lock();
                    if (completion.get() != 0)
                    {
                        EncodingsCompleteEvent event;
                        event.cookie = this->completion_cookie;
                        produce_event(completion.get(), &event);
                    }

                    switch_to_state(State::done_state);
                }
                break;

            default:
                throw FatalError("unexpected event");
            }
            break;

        default:
            throw FatalError("Globals::Complete unexpected state");
        }

        return EventResult::event_result_continue;
    }
}
