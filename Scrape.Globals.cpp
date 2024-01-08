// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Scrape.Globals.h"

namespace Scrape
{
    Globals* globals = 0;

    Globals::Globals()
    {
        this->video_index = std::make_shared<Index>();

		initialize_unicode(&streams_namespace, "dynamo.streams");

		initialize_unicode(&amazon_title_class, "ilt2");
		initialize_unicode(&amazon_result_id_prefix, "result_");
		initialize_unicode(&amazon_source_name, "Amazon");
		initialize_unicode(&amazon_sign_in_link, "Hello. Sign in Your Account");
		initialize_unicode(&amazon_sign_in_form, "ap_signin_form");
		initialize_unicode(&amazon_email_control, "email");
		initialize_unicode(&amazon_password_control, "password");
		initialize_unicode(&amazon_prime_link, "Your Prime");
		initialize_unicode(&amazon_browse_link, "Browse Prime Instant Video");
		initialize_unicode(&amazon_movies_link, "movies");
		initialize_unicode(&amazon_next_page_link, "Next Page");

		initialize_unicode(&netflix_sign_in_link, "Member Sign In");
		initialize_unicode(&netflix_movieid_param, "movieid");
		initialize_unicode(&netflix_movie_url, "http://dvd.netflix.com/Movie");
		initialize_unicode(&netflix_search_form, "global-search");
		initialize_unicode(&netflix_logon_form, "login-form");
		initialize_unicode(&netflix_email_control, "email");
		initialize_unicode(&netflix_password_control, "password");
		initialize_unicode(&netflix_query1_control, "raw_query");
		initialize_unicode(&netflix_query2_control, "v1");
		initialize_unicode(&netflix_search_path, "Search");
		initialize_unicode(&netflix_row_param, "row");

		initialize_unicode(&command_amazon, "amazon");
		initialize_unicode(&command_netflix, "netflix");

		for (Codepoint codepoint = '0'; codepoint <= '9'; codepoint++)
		{
			UnicodeStringRef term = std::make_shared<UnicodeString>();
			term->push_back(codepoint);
			netflix_search_space.push_back(term);
		}

		for (Codepoint codepoint = 'a'; codepoint <= 'z'; codepoint++)
		{
			UnicodeStringRef term = std::make_shared<UnicodeString>();
			term->push_back(codepoint);
			netflix_search_space.push_back(term);
		}

        initialize_unicode(&title_property, "title");
        initialize_unicode(&as_of_property, "as of");
        initialize_unicode(&source_property, "source");

		this->netflix_url = std::make_shared<Basic::Uri>();
		this->netflix_url->Initialize("https://signup.netflix.com/Login");

		this->amazon_url = std::make_shared<Basic::Uri>();
		this->amazon_url->Initialize("http://www.amazon.com/s/ref=sr_il_to_instant-video?rh=n%3A2858778011&ie=UTF8");
    }

    void Globals::Initialize()
    {
    }

    void Globals::Store(UnicodeStringRef source, std::shared_ptr<Json::Value> value)
    {
        if (value->type == Json::Value::Type::array_value)
        {
            Json::Array* array = (Json::Array*)value.get();

            for (Json::ValueList::iterator it = array->elements.begin(); it != array->elements.end(); it++)
            {
                Store(source, (*it));
            }
        }
        else if (value->type == Json::Value::Type::object_value)
        {
            std::shared_ptr<Json::Object> object = std::static_pointer_cast<Json::Object>(value);

            Json::MemberList::iterator title_it = object->members.find(title_property);

            if (title_it != object->members.end() &&
                title_it->second->type == Json::Value::Type::string_value)
            {
                std::shared_ptr<Json::String> as_of = std::make_shared<Json::String>();
                as_of->value = std::make_shared<UnicodeString>();

                TextWriter writer(as_of->value.get());
                writer.WriteTimestamp();

                auto as_of_result = object->members.insert(Json::MemberList::value_type(as_of_property, as_of));
                if (as_of_result.second == false)
                    as_of_result.first->second = as_of;

                Json::String* title_string = (Json::String*)title_it->second.get();
                this->video_index->Add(title_string->value, object);
            }
        }
    }

    void Globals::Search(UnicodeStringRef query, std::shared_ptr<Json::Array>* results)
    {
        std::shared_ptr<Json::Array> list = std::make_shared<Json::Array>();

        uint32 begin;
        uint32 end;

        this->video_index->Search(query, &begin, &end);

        for (uint32 i = begin; i != end; i++)
        {
            list->elements.push_back(this->video_index->results[i].value);
        }

        (*results) = list;
    }
}
