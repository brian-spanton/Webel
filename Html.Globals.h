// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.CharacterToken.h"
#include "Html.ElementName.h"
#include "Html.Node.h"

namespace Html
{
    using namespace Basic;

    class Globals : public IRefCounted
    {
    private:
        template <int Count>
        void InitializeElementName(ElementName::Ref* element, UnicodeString* name_space, const char (&name)[Count])
        {
            UnicodeString::Ref name_string = New<UnicodeString>();
            if (name[Count - 1] == 0)
                name_string->append(name, name + Count - 1);
            else
                name_string->append(name, name + Count);

            (*element) = New<ElementName>();
            (*element)->Initialize(name_space, name_string);
        }

    public:
        typedef Basic::Ref<Globals> Ref;

        Globals();

        void Initialize();

        ElementNameList::Ref Scope; // REF
        ElementNameList::Ref ListItemScope; // REF
        ElementNameList::Ref ButtonScope; // REF
        ElementNameList::Ref TableScope; // REF
        ElementNameList::Ref SelectScope; // REF

        StringMap::Ref named_character_references_table; // REF

        TranslationMap::Ref number_character_references_table; // REF

        UnicodeString::Ref Script; // REF

        UnicodeString::Ref markup_declaration_comment; // REF
        UnicodeString::Ref markup_declaration_doctype; // REF
        UnicodeString::Ref markup_declaration_cdata; // REF
        UnicodeString::Ref after_doctype_public_keyword; // REF
        UnicodeString::Ref after_doctype_system_keyword; // REF
        UnicodeString::Ref cdata_section_end; // REF

        UnicodeString::Ref Namespace_HTML; // REF
        UnicodeString::Ref Namespace_XML; // REF
        UnicodeString::Ref Namespace_XMLNS; // REF
        UnicodeString::Ref Namespace_MathML; // REF
        UnicodeString::Ref Namespace_SVG; // REF
        UnicodeString::Ref Namespace_XLink; // REF

        UnicodeString::Ref DOCTYPE_html_4_0_public_identifier; // REF
        UnicodeString::Ref DOCTYPE_html_4_0_system_identifier; // REF
        UnicodeString::Ref DOCTYPE_html_4_01_public_identifier; // REF
        UnicodeString::Ref DOCTYPE_html_4_01_system_identifier; // REF
        UnicodeString::Ref DOCTYPE_xhtml_1_0_public_identifier; // REF
        UnicodeString::Ref DOCTYPE_xhtml_1_0_system_identifier; // REF
        UnicodeString::Ref DOCTYPE_xhtml_1_1_public_identifier; // REF
        UnicodeString::Ref DOCTYPE_xhtml_1_1_system_identifier; // REF
        UnicodeString::Ref DOCTYPE_legacy_compat; // REF

        UnicodeString::Ref encoding_attribute_name; // REF
        UnicodeString::Ref href_attribute_name; // REF
        UnicodeString::Ref action_attribute_name; // REF
        UnicodeString::Ref id_attribute_name; // REF
        UnicodeString::Ref form_attribute_name; // REF
        UnicodeString::Ref value_attribute_name; // REF
        UnicodeString::Ref type_attribute_name; // REF
        UnicodeString::Ref dirname_attribute_name; // REF
        UnicodeString::Ref name_attribute_name; // REF
        UnicodeString::Ref disabled_attribute_name; // REF
        UnicodeString::Ref checked_attribute_name; // REF
        UnicodeString::Ref enctype_attribute_name; // REF
        UnicodeString::Ref method_attribute_name; // REF
        UnicodeString::Ref target_attribute_name; // REF
        UnicodeString::Ref accept_charset_attribute_name; // REF
        UnicodeString::Ref class_attribute_name; // REF

        UnicodeString::Ref on_value; // REF

        UnicodeString::Ref checkbox_type; // REF
        UnicodeString::Ref radio_type; // REF
        UnicodeString::Ref image_type; // REF
        UnicodeString::Ref file_type; // REF
        UnicodeString::Ref object_type; // REF
        UnicodeString::Ref hidden_type; // REF
        UnicodeString::Ref text_type; // REF

        UnicodeString::Ref _charset__name; // REF
        UnicodeString::Ref isindex_name; // REF

        UnicodeString::Ref text_html_media_type; // REF
        UnicodeString::Ref application_xhtml_xml_media_type; // REF

        ElementName::Ref HTML_applet; // REF
        ElementName::Ref HTML_body; // REF
        ElementName::Ref HTML_br; // REF
        ElementName::Ref button_element_name; // REF
        ElementName::Ref HTML_caption; // REF
        ElementName::Ref HTML_colgroup; // REF
        ElementName::Ref HTML_frameset; // REF
        ElementName::Ref HTML_head; // REF
        ElementName::Ref HTML_html; // REF
        ElementName::Ref HTML_marquee; // REF
        ElementName::Ref object_element_name; // REF
        ElementName::Ref HTML_ol; // REF
        ElementName::Ref HTML_optgroup; // REF
        ElementName::Ref HTML_option; // REF
        ElementName::Ref select_element_name; // REF
        ElementName::Ref HTML_table; // REF
        ElementName::Ref HTML_tbody; // REF
        ElementName::Ref HTML_td; // REF
        ElementName::Ref HTML_tfoot; // REF
        ElementName::Ref HTML_th; // REF
        ElementName::Ref HTML_thead; // REF
        ElementName::Ref HTML_tr; // REF
        ElementName::Ref HTML_ul; // REF
        ElementName::Ref HTML_a; // REF
        ElementName::Ref HTML_b; // REF
        ElementName::Ref HTML_big; // REF
        ElementName::Ref HTML_code; // REF
        ElementName::Ref HTML_em; // REF
        ElementName::Ref HTML_font; // REF
        ElementName::Ref HTML_i; // REF
        ElementName::Ref HTML_nobr; // REF
        ElementName::Ref HTML_s; // REF
        ElementName::Ref HTML_small; // REF
        ElementName::Ref HTML_strike; // REF
        ElementName::Ref HTML_strong; // REF
        ElementName::Ref HTML_tt; // REF
        ElementName::Ref HTML_u; // REF
        ElementName::Ref HTML_fieldset; // REF
        ElementName::Ref input_element_name; // REF
        ElementName::Ref HTML_keygen; // REF
        ElementName::Ref HTML_label; // REF
        ElementName::Ref HTML_output; // REF
        ElementName::Ref HTML_textarea; // REF
        ElementName::Ref HTML_base; // REF
        ElementName::Ref HTML_basefont; // REF
        ElementName::Ref HTML_bgsound; // REF
        ElementName::Ref HTML_link; // REF
        ElementName::Ref HTML_title; // REF
        ElementName::Ref HTML_meta; // REF
        ElementName::Ref HTML_noscript; // REF
        ElementName::Ref HTML_noframes; // REF
        ElementName::Ref HTML_style; // REF
        ElementName::Ref HTML_script; // REF
        ElementName::Ref HTML_dd; // REF
        ElementName::Ref HTML_dt; // REF
        ElementName::Ref HTML_li; // REF
        ElementName::Ref HTML_p; // REF
        ElementName::Ref HTML_rp; // REF
        ElementName::Ref HTML_rt; // REF
        ElementName::Ref HTML_address; // REF
        ElementName::Ref HTML_article; // REF
        ElementName::Ref HTML_aside; // REF
        ElementName::Ref HTML_blockquote; // REF
        ElementName::Ref HTML_center; // REF
        ElementName::Ref HTML_details; // REF
        ElementName::Ref HTML_dialog; // REF
        ElementName::Ref HTML_dir; // REF
        ElementName::Ref HTML_div; // REF
        ElementName::Ref HTML_dl; // REF
        ElementName::Ref HTML_figcaption; // REF
        ElementName::Ref HTML_figure; // REF
        ElementName::Ref HTML_footer; // REF
        ElementName::Ref HTML_header; // REF
        ElementName::Ref HTML_hgroup; // REF
        ElementName::Ref HTML_main; // REF
        ElementName::Ref HTML_menu; // REF
        ElementName::Ref HTML_nav; // REF
        ElementName::Ref HTML_section; // REF
        ElementName::Ref HTML_summary; // REF
        ElementName::Ref HTML_h1; // REF
        ElementName::Ref HTML_h2; // REF
        ElementName::Ref HTML_h3; // REF
        ElementName::Ref HTML_h4; // REF
        ElementName::Ref HTML_h5; // REF
        ElementName::Ref HTML_h6; // REF
        ElementName::Ref HTML_pre; // REF
        ElementName::Ref HTML_listing; // REF
        ElementName::Ref HTML_form; // REF
        ElementName::Ref HTML_plaintext; // REF
        ElementName::Ref HTML_area; // REF
        ElementName::Ref HTML_embed; // REF
        ElementName::Ref HTML_img; // REF
        ElementName::Ref HTML_wbr; // REF
        ElementName::Ref HTML_menuitem; // REF
        ElementName::Ref HTML_param; // REF
        ElementName::Ref HTML_source; // REF
        ElementName::Ref HTML_track; // REF
        ElementName::Ref HTML_hr; // REF
        ElementName::Ref HTML_image; // REF
        ElementName::Ref HTML_isindex; // REF
        ElementName::Ref HTML_xmp; // REF
        ElementName::Ref HTML_iframe; // REF
        ElementName::Ref HTML_noembed; // REF
        ElementName::Ref HTML_ruby; // REF
        ElementName::Ref HTML_col; // REF
        ElementName::Ref HTML_frame; // REF
        ElementName::Ref datalist_element_name; // REF

        ElementName::Ref MathML_annotation_xml; // REF
        ElementName::Ref MathML_malignmark; // REF
        ElementName::Ref MathML_mglyph; // REF
        ElementName::Ref MathML_mi; // REF
        ElementName::Ref MathML_mn; // REF
        ElementName::Ref MathML_mo; // REF
        ElementName::Ref MathML_ms; // REF
        ElementName::Ref MathML_mtext; // REF
        ElementName::Ref MathML_math; // REF

        ElementName::Ref SVG_desc; // REF
        ElementName::Ref SVG_foreignObject; // REF
        ElementName::Ref SVG_svg; // REF
        ElementName::Ref SVG_title; // REF
    };

    extern Inline<Globals>* globals;
}