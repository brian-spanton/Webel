// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.CharacterToken.h"
#include "Html.ElementName.h"
#include "Html.Node.h"
#include "Html.EndOfFileToken.h"

namespace Html
{
    using namespace Basic;

    class Globals
    {
    private:
        template <int Count>
        void InitializeElementName(std::shared_ptr<ElementName>* element, UnicodeStringRef name_space, const char (&name)[Count])
        {
            UnicodeStringRef name_string = std::make_shared<UnicodeString>();
            if (name[Count - 1] == 0)
                name_string->append(name, name + Count - 1);
            else
                name_string->append(name, name + Count);

            (*element) = std::make_shared<ElementName>();
            (*element)->Initialize(name_space, name_string);
        }

    public:
        Globals();

        void Initialize();

        ElementNameList Scope;
        ElementNameList ListItemScope;
        ElementNameList ButtonScope;
        ElementNameList TableScope;
        ElementNameList SelectScope;

        std::shared_ptr<StringMap> named_character_references_table;

        TranslationMap number_character_references_table;

        UnicodeStringRef Script;

        UnicodeStringRef markup_declaration_comment;
        UnicodeStringRef markup_declaration_doctype;
        UnicodeStringRef markup_declaration_cdata;
        UnicodeStringRef after_doctype_public_keyword;
        UnicodeStringRef after_doctype_system_keyword;
        UnicodeStringRef cdata_section_end;

        UnicodeStringRef Namespace_HTML;
        UnicodeStringRef Namespace_XML;
        UnicodeStringRef Namespace_XMLNS;
        UnicodeStringRef Namespace_MathML;
        UnicodeStringRef Namespace_SVG;
        UnicodeStringRef Namespace_XLink;

        UnicodeStringRef DOCTYPE_html_4_0_public_identifier;
        UnicodeStringRef DOCTYPE_html_4_0_system_identifier;
        UnicodeStringRef DOCTYPE_html_4_01_public_identifier;
        UnicodeStringRef DOCTYPE_html_4_01_system_identifier;
        UnicodeStringRef DOCTYPE_xhtml_1_0_public_identifier;
        UnicodeStringRef DOCTYPE_xhtml_1_0_system_identifier;
        UnicodeStringRef DOCTYPE_xhtml_1_1_public_identifier;
        UnicodeStringRef DOCTYPE_xhtml_1_1_system_identifier;
        UnicodeStringRef DOCTYPE_legacy_compat;

        UnicodeStringRef encoding_attribute_name;
        UnicodeStringRef href_attribute_name;
        UnicodeStringRef action_attribute_name;
        UnicodeStringRef id_attribute_name;
        UnicodeStringRef form_attribute_name;
        UnicodeStringRef value_attribute_name;
        UnicodeStringRef type_attribute_name;
        UnicodeStringRef dirname_attribute_name;
        UnicodeStringRef name_attribute_name;
        UnicodeStringRef disabled_attribute_name;
        UnicodeStringRef checked_attribute_name;
        UnicodeStringRef enctype_attribute_name;
        UnicodeStringRef method_attribute_name;
        UnicodeStringRef target_attribute_name;
        UnicodeStringRef accept_charset_attribute_name;
        UnicodeStringRef class_attribute_name;
        UnicodeStringRef src_attribute_name;

        UnicodeStringRef on_value;

        UnicodeStringRef checkbox_type;
        UnicodeStringRef radio_type;
        UnicodeStringRef image_type;
        UnicodeStringRef file_type;
        UnicodeStringRef object_type;
        UnicodeStringRef hidden_type;
        UnicodeStringRef text_type;

        UnicodeStringRef _charset__name;
        UnicodeStringRef isindex_name;

        UnicodeStringRef text_html_media_type; // $$ use a proper MediaType?
        UnicodeStringRef application_xhtml_xml_media_type;

        std::shared_ptr<ElementName> HTML_applet;
        std::shared_ptr<ElementName> HTML_body;
        std::shared_ptr<ElementName> HTML_br;
        std::shared_ptr<ElementName> button_element_name;
        std::shared_ptr<ElementName> HTML_caption;
        std::shared_ptr<ElementName> HTML_colgroup;
        std::shared_ptr<ElementName> HTML_frameset;
        std::shared_ptr<ElementName> HTML_head;
        std::shared_ptr<ElementName> HTML_html;
        std::shared_ptr<ElementName> HTML_marquee;
        std::shared_ptr<ElementName> object_element_name;
        std::shared_ptr<ElementName> HTML_ol;
        std::shared_ptr<ElementName> HTML_optgroup;
        std::shared_ptr<ElementName> HTML_option;
        std::shared_ptr<ElementName> select_element_name;
        std::shared_ptr<ElementName> HTML_table;
        std::shared_ptr<ElementName> HTML_tbody;
        std::shared_ptr<ElementName> HTML_td;
        std::shared_ptr<ElementName> HTML_tfoot;
        std::shared_ptr<ElementName> HTML_th;
        std::shared_ptr<ElementName> HTML_thead;
        std::shared_ptr<ElementName> HTML_tr;
        std::shared_ptr<ElementName> HTML_ul;
        std::shared_ptr<ElementName> HTML_a;
        std::shared_ptr<ElementName> HTML_b;
        std::shared_ptr<ElementName> HTML_big;
        std::shared_ptr<ElementName> HTML_code;
        std::shared_ptr<ElementName> HTML_em;
        std::shared_ptr<ElementName> HTML_font;
        std::shared_ptr<ElementName> HTML_i;
        std::shared_ptr<ElementName> HTML_nobr;
        std::shared_ptr<ElementName> HTML_s;
        std::shared_ptr<ElementName> HTML_small;
        std::shared_ptr<ElementName> HTML_strike;
        std::shared_ptr<ElementName> HTML_strong;
        std::shared_ptr<ElementName> HTML_tt;
        std::shared_ptr<ElementName> HTML_u;
        std::shared_ptr<ElementName> HTML_fieldset;
        std::shared_ptr<ElementName> input_element_name;
        std::shared_ptr<ElementName> HTML_keygen;
        std::shared_ptr<ElementName> HTML_label;
        std::shared_ptr<ElementName> HTML_output;
        std::shared_ptr<ElementName> HTML_textarea;
        std::shared_ptr<ElementName> HTML_base;
        std::shared_ptr<ElementName> HTML_basefont;
        std::shared_ptr<ElementName> HTML_bgsound;
        std::shared_ptr<ElementName> HTML_link;
        std::shared_ptr<ElementName> HTML_title;
        std::shared_ptr<ElementName> HTML_meta;
        std::shared_ptr<ElementName> HTML_noscript;
        std::shared_ptr<ElementName> HTML_noframes;
        std::shared_ptr<ElementName> HTML_style;
        std::shared_ptr<ElementName> HTML_script;
        std::shared_ptr<ElementName> HTML_dd;
        std::shared_ptr<ElementName> HTML_dt;
        std::shared_ptr<ElementName> HTML_li;
        std::shared_ptr<ElementName> HTML_p;
        std::shared_ptr<ElementName> HTML_rp;
        std::shared_ptr<ElementName> HTML_rt;
        std::shared_ptr<ElementName> HTML_address;
        std::shared_ptr<ElementName> HTML_article;
        std::shared_ptr<ElementName> HTML_aside;
        std::shared_ptr<ElementName> HTML_blockquote;
        std::shared_ptr<ElementName> HTML_center;
        std::shared_ptr<ElementName> HTML_details;
        std::shared_ptr<ElementName> HTML_dialog;
        std::shared_ptr<ElementName> HTML_dir;
        std::shared_ptr<ElementName> HTML_div;
        std::shared_ptr<ElementName> HTML_dl;
        std::shared_ptr<ElementName> HTML_figcaption;
        std::shared_ptr<ElementName> HTML_figure;
        std::shared_ptr<ElementName> HTML_footer;
        std::shared_ptr<ElementName> HTML_header;
        std::shared_ptr<ElementName> HTML_hgroup;
        std::shared_ptr<ElementName> HTML_main;
        std::shared_ptr<ElementName> HTML_menu;
        std::shared_ptr<ElementName> HTML_nav;
        std::shared_ptr<ElementName> HTML_section;
        std::shared_ptr<ElementName> HTML_summary;
        std::shared_ptr<ElementName> HTML_h1;
        std::shared_ptr<ElementName> HTML_h2;
        std::shared_ptr<ElementName> HTML_h3;
        std::shared_ptr<ElementName> HTML_h4;
        std::shared_ptr<ElementName> HTML_h5;
        std::shared_ptr<ElementName> HTML_h6;
        std::shared_ptr<ElementName> HTML_pre;
        std::shared_ptr<ElementName> HTML_listing;
        std::shared_ptr<ElementName> HTML_form;
        std::shared_ptr<ElementName> HTML_plaintext;
        std::shared_ptr<ElementName> HTML_area;
        std::shared_ptr<ElementName> HTML_embed;
        std::shared_ptr<ElementName> HTML_img;
        std::shared_ptr<ElementName> HTML_wbr;
        std::shared_ptr<ElementName> HTML_menuitem;
        std::shared_ptr<ElementName> HTML_param;
        std::shared_ptr<ElementName> HTML_source;
        std::shared_ptr<ElementName> HTML_track;
        std::shared_ptr<ElementName> HTML_hr;
        std::shared_ptr<ElementName> HTML_image;
        std::shared_ptr<ElementName> HTML_isindex;
        std::shared_ptr<ElementName> HTML_xmp;
        std::shared_ptr<ElementName> HTML_iframe;
        std::shared_ptr<ElementName> HTML_noembed;
        std::shared_ptr<ElementName> HTML_ruby;
        std::shared_ptr<ElementName> HTML_col;
        std::shared_ptr<ElementName> HTML_frame;
        std::shared_ptr<ElementName> datalist_element_name;

        std::shared_ptr<ElementName> MathML_annotation_xml;
        std::shared_ptr<ElementName> MathML_malignmark;
        std::shared_ptr<ElementName> MathML_mglyph;
        std::shared_ptr<ElementName> MathML_mi;
        std::shared_ptr<ElementName> MathML_mn;
        std::shared_ptr<ElementName> MathML_mo;
        std::shared_ptr<ElementName> MathML_ms;
        std::shared_ptr<ElementName> MathML_mtext;
        std::shared_ptr<ElementName> MathML_math;

        std::shared_ptr<ElementName> SVG_desc;
        std::shared_ptr<ElementName> SVG_foreignObject;
        std::shared_ptr<ElementName> SVG_svg;
        std::shared_ptr<ElementName> SVG_title;

        std::shared_ptr<EndOfFileToken> eof_token;
    };

    extern Globals* globals;
}