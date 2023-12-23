// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.Frame.h"

namespace Html
{
    using namespace Basic;

    typedef std::unordered_map<Codepoint, Codepoint> TranslationMap;

    enum Confidence
    {
        Confidence_Tentative,
        Confidence_Certain,
        Confidence_Irrelevant,
    };

    enum TokenizerState
    {
        data_state = Frame::Start_State,
        character_reference_in_data_state,
        RCDATA_state,
        character_reference_in_RCDATA_state,
        RAWTEXT_state,
        script_data_state,
        PLAINTEXT_state,
        tag_open_state,
        end_tag_open_state,
        tag_name_state,
        RCDATA_less_than_sign_state,
        RCDATA_end_tag_open_state,
        RCDATA_end_tag_name_state,
        RAWTEXT_less_than_sign_state,
        RAWTEXT_end_tag_open_state,
        RAWTEXT_end_tag_name_state,
        script_data_less_than_sign_state,
        script_data_end_tag_open_state,
        script_data_end_tag_name_state,
        script_data_escape_start_state,
        script_data_escape_start_dash_state,
        script_data_escaped_state,
        script_data_escaped_dash_state,
        script_data_escaped_dash_dash_state,
        script_data_escaped_less_than_sign_state,
        script_data_escaped_end_tag_open_state,
        script_data_escaped_end_tag_name_state,
        script_data_double_escape_start_state,
        script_data_double_escaped_state,
        script_data_double_escaped_dash_state,
        script_data_double_escaped_dash_dash_state,
        script_data_double_escaped_less_than_sign_state,
        script_data_double_escape_end_state,
        before_attribute_name_state,
        attribute_name_state,
        after_attribute_name_state,
        before_attribute_value_state,
        attribute_value_double_quoted_state,
        attribute_value_single_quoted_state,
        attribute_value_unquoted_state,
        character_reference_in_attribute_value_state,
        after_attribute_value_quoted_state,
        self_closing_start_tag_state,
        bogus_comment_state,
        markup_declaration_open_state,
        markup_declaration_open_2_state,
        comment_start_state_state,
        comment_start_dash_state,
        comment_state,
        comment_end_dash_state,
        comment_end_state,
        comment_end_bang_state,
        doctype_state,
        before_doctype_name_state,
        doctype_name_state,
        after_doctype_name_state,
        after_doctype_name_2_state,
        after_doctype_public_keyword_state,
        after_doctype_system_keyword_state,
        bogus_doctype_state,
        before_doctype_public_identifier_state,
        doctype_public_identifier_double_quoted_state,
        doctype_public_identifier_single_quoted_state,
        after_doctype_public_identifier_state,
        between_doctype_public_and_system_identifiers_state,
        before_doctype_system_identifier_state,
        doctype_system_identifier_double_quoted_state,
        doctype_system_identifier_single_quoted_state,
        after_doctype_system_identifier_state,
        cdata_section_state,
        cdata_section_2_state,
        cdata_section_3_state,
    };
}
