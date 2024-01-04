// Copyright © 2013 Brian Spanton

#pragma once

#include "Scrape.Types.h"

namespace Scrape
{
    using namespace Basic;

    class Globals
    {
    public:
		UnicodeStringRef streams_namespace;

		Basic::UnicodeStringRef command_amazon;
		std::shared_ptr<Uri> amazon_url;
		UnicodeStringRef amazon_title_class;
		UnicodeStringRef amazon_result_id_prefix;
		UnicodeStringRef amazon_source_name;
		UnicodeStringRef amazon_sign_in_link;
		UnicodeStringRef amazon_sign_in_form;
		UnicodeStringRef amazon_email_control;
		UnicodeStringRef amazon_password_control;
		UnicodeStringRef amazon_prime_link;
		UnicodeStringRef amazon_browse_link;
		UnicodeStringRef amazon_movies_link;
		UnicodeStringRef amazon_next_page_link;

		Basic::UnicodeStringRef command_netflix;
		std::shared_ptr<Uri> netflix_url;
		std::list<UnicodeStringRef> netflix_search_space;
		UnicodeStringRef netflix_sign_in_link;
		UnicodeStringRef netflix_movieid_param;
		UnicodeStringRef netflix_movie_url;
		UnicodeStringRef netflix_search_form;
		UnicodeStringRef netflix_logon_form;
		UnicodeStringRef netflix_email_control;
		UnicodeStringRef netflix_password_control;
		UnicodeStringRef netflix_query1_control;
		UnicodeStringRef netflix_query2_control;
		UnicodeStringRef netflix_search_path;
		UnicodeStringRef netflix_row_param;

        UnicodeStringRef title_property;
        UnicodeStringRef as_of_property;
        UnicodeStringRef source_property;

        std::shared_ptr<Index> video_index;

        Globals();

        void Initialize();
        void Store(UnicodeStringRef source, std::shared_ptr<Json::Value> value);
        void Search(UnicodeStringRef query, std::shared_ptr<Json::Array>* results);
    };

    extern Globals* globals;
}