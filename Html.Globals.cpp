// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Globals.h"
#include "Html.Types.h"
#include "Html.CharacterToken.h"
#include "Html.ElementName.h"
#include "Html.Node.h"
#include "Html.TextNode.h"
#include "Html.ElementNode.h"

namespace Html
{
    Globals* globals = 0;

    Globals::Globals()
    {
    }

    void Globals::Initialize()
    {
        initialize_unicode(&Script, "script");

        initialize_unicode(&markup_declaration_comment, "--");
        initialize_unicode(&markup_declaration_doctype, "DOCTYPE");
        initialize_unicode(&markup_declaration_cdata, "[CDATA[");
        initialize_unicode(&after_doctype_public_keyword, "PUBLIC");
        initialize_unicode(&after_doctype_system_keyword, "SYSTEM");
        initialize_unicode(&cdata_section_end, "]]>");

        initialize_unicode(&Namespace_HTML, "http://www.w3.org/1999/xhtml");
        initialize_unicode(&Namespace_XML, "http://www.w3.org/XML/1998/namespace");
        initialize_unicode(&Namespace_XMLNS, "http://www.w3.org/2000/xmlns/");
        initialize_unicode(&Namespace_MathML, "http://www.w3.org/1998/Math/MathML");
        initialize_unicode(&Namespace_SVG, "http://www.w3.org/2000/svg");
        initialize_unicode(&Namespace_XLink, "http://www.w3.org/1999/xlink");

        initialize_unicode(&DOCTYPE_html_4_0_public_identifier, "-//W3C//DTD HTML 4.0//EN");
        initialize_unicode(&DOCTYPE_html_4_0_system_identifier, "http://www.w3.org/TR/REC-html40/strict.dtd");
        initialize_unicode(&DOCTYPE_html_4_01_public_identifier, "-//W3C//DTD HTML 4.01//EN");
        initialize_unicode(&DOCTYPE_html_4_01_system_identifier, "http://www.w3.org/TR/html4/strict.dtd");
        initialize_unicode(&DOCTYPE_xhtml_1_0_public_identifier, "-//W3C//DTD XHTML 1.0 Strict//EN");
        initialize_unicode(&DOCTYPE_xhtml_1_0_system_identifier, "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd");
        initialize_unicode(&DOCTYPE_xhtml_1_1_public_identifier, "-//W3C//DTD XHTML 1.1//EN");
        initialize_unicode(&DOCTYPE_xhtml_1_1_system_identifier, "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd");
        initialize_unicode(&DOCTYPE_legacy_compat, "about:legacy-compat");

        initialize_unicode(&encoding_attribute_name, "encoding");
        initialize_unicode(&href_attribute_name, "href");
        initialize_unicode(&action_attribute_name, "action");
        initialize_unicode(&id_attribute_name, "id");
        initialize_unicode(&form_attribute_name, "form");
        initialize_unicode(&value_attribute_name, "value");
        initialize_unicode(&type_attribute_name, "type");
        initialize_unicode(&dirname_attribute_name, "dirname");
        initialize_unicode(&name_attribute_name, "name");
        initialize_unicode(&disabled_attribute_name, "disabled");
        initialize_unicode(&checked_attribute_name, "checked");
        initialize_unicode(&enctype_attribute_name, "enctype");
        initialize_unicode(&method_attribute_name, "method");
        initialize_unicode(&target_attribute_name, "target");
        initialize_unicode(&accept_charset_attribute_name, "accept-charset");
        initialize_unicode(&class_attribute_name, "class");

        initialize_unicode(&on_value, "on");

        initialize_unicode(&checkbox_type, "checkbox");
        initialize_unicode(&radio_type, "radio");
        initialize_unicode(&image_type, "image");
        initialize_unicode(&file_type, "file");
        initialize_unicode(&object_type, "object");
        initialize_unicode(&hidden_type, "hidden");
        initialize_unicode(&text_type, "text");

        initialize_unicode(&_charset__name, "_charset_");
        initialize_unicode(&isindex_name, "isindex");

        initialize_unicode(&text_html_media_type, "text/html");
        initialize_unicode(&application_xhtml_xml_media_type, "application/xhtml+xml");

        InitializeElementName(&HTML_applet, Namespace_HTML, "applet");
        InitializeElementName(&HTML_body, Namespace_HTML, "body");
        InitializeElementName(&HTML_br, Namespace_HTML, "br");
        InitializeElementName(&button_element_name, Namespace_HTML, "button");
        InitializeElementName(&HTML_caption, Namespace_HTML, "caption");
        InitializeElementName(&HTML_colgroup, Namespace_HTML, "colgroup");
        InitializeElementName(&HTML_frameset, Namespace_HTML, "frameset");
        InitializeElementName(&HTML_head, Namespace_HTML, "head");
        InitializeElementName(&HTML_html, Namespace_HTML, "html");
        InitializeElementName(&HTML_marquee, Namespace_HTML, "marquee");
        InitializeElementName(&object_element_name, Namespace_HTML, "object");
        InitializeElementName(&HTML_ol, Namespace_HTML, "ol");
        InitializeElementName(&HTML_optgroup, Namespace_HTML, "optgroup");
        InitializeElementName(&HTML_option, Namespace_HTML, "option");
        InitializeElementName(&select_element_name, Namespace_HTML, "select");
        InitializeElementName(&HTML_table, Namespace_HTML, "table");
        InitializeElementName(&HTML_tbody, Namespace_HTML, "tbody");
        InitializeElementName(&HTML_td, Namespace_HTML, "td");
        InitializeElementName(&HTML_tfoot, Namespace_HTML, "tfoot");
        InitializeElementName(&HTML_th, Namespace_HTML, "th");
        InitializeElementName(&HTML_thead, Namespace_HTML, "thead");
        InitializeElementName(&HTML_tr, Namespace_HTML, "tr");
        InitializeElementName(&HTML_ul, Namespace_HTML, "ul");
        InitializeElementName(&HTML_a, Namespace_HTML, "a");
        InitializeElementName(&HTML_b, Namespace_HTML, "b");
        InitializeElementName(&HTML_big, Namespace_HTML, "big");
        InitializeElementName(&HTML_code, Namespace_HTML, "code");
        InitializeElementName(&HTML_em, Namespace_HTML, "em");
        InitializeElementName(&HTML_font, Namespace_HTML, "font");
        InitializeElementName(&HTML_i, Namespace_HTML, "i");
        InitializeElementName(&HTML_nobr, Namespace_HTML, "nobr");
        InitializeElementName(&HTML_s, Namespace_HTML, "s");
        InitializeElementName(&HTML_small, Namespace_HTML, "small");
        InitializeElementName(&HTML_strike, Namespace_HTML, "strike");
        InitializeElementName(&HTML_strong, Namespace_HTML, "strong");
        InitializeElementName(&HTML_tt, Namespace_HTML, "tt");
        InitializeElementName(&HTML_u, Namespace_HTML, "u");
        InitializeElementName(&HTML_fieldset, Namespace_HTML, "fieldset");
        InitializeElementName(&input_element_name, Namespace_HTML, "input");
        InitializeElementName(&HTML_keygen, Namespace_HTML, "keygen");
        InitializeElementName(&HTML_label, Namespace_HTML, "label");
        InitializeElementName(&HTML_output, Namespace_HTML, "output");
        InitializeElementName(&HTML_textarea, Namespace_HTML, "textarea");
        InitializeElementName(&MathML_annotation_xml, Namespace_MathML, "annotation-xml");
        InitializeElementName(&MathML_malignmark, Namespace_MathML, "malignmark");
        InitializeElementName(&MathML_mglyph, Namespace_MathML, "mglyph");
        InitializeElementName(&MathML_mi, Namespace_MathML, "mi");
        InitializeElementName(&MathML_mn, Namespace_MathML, "mn");
        InitializeElementName(&MathML_mo, Namespace_MathML, "mo");
        InitializeElementName(&MathML_ms, Namespace_MathML, "ms");
        InitializeElementName(&MathML_mtext, Namespace_MathML, "mtext");
        InitializeElementName(&MathML_math, Namespace_MathML, "math");
        InitializeElementName(&SVG_desc, Namespace_SVG, "desc");
        InitializeElementName(&SVG_foreignObject, Namespace_SVG, "foreignObject");
        InitializeElementName(&SVG_svg, Namespace_SVG, "svg");
        InitializeElementName(&SVG_title, Namespace_SVG, "title");
        InitializeElementName(&HTML_base, Namespace_HTML, "base");
        InitializeElementName(&HTML_basefont, Namespace_HTML, "basefont");
        InitializeElementName(&HTML_bgsound, Namespace_HTML, "bgsound");
        InitializeElementName(&HTML_link, Namespace_HTML, "link");
        InitializeElementName(&HTML_title, Namespace_HTML, "title");
        InitializeElementName(&HTML_meta, Namespace_HTML, "meta");
        InitializeElementName(&HTML_noscript, Namespace_HTML, "noscript");
        InitializeElementName(&HTML_noframes, Namespace_HTML, "noframes");
        InitializeElementName(&HTML_style, Namespace_HTML, "style");
        InitializeElementName(&HTML_script, Namespace_HTML, "script");
        InitializeElementName(&HTML_dd, Namespace_HTML, "dd");
        InitializeElementName(&HTML_dt, Namespace_HTML, "dt");
        InitializeElementName(&HTML_li, Namespace_HTML, "li");
        InitializeElementName(&HTML_p, Namespace_HTML, "p");
        InitializeElementName(&HTML_rp, Namespace_HTML, "rp");
        InitializeElementName(&HTML_rt, Namespace_HTML, "rt");
        InitializeElementName(&HTML_address, Namespace_HTML, "address");
        InitializeElementName(&HTML_article, Namespace_HTML, "article");
        InitializeElementName(&HTML_aside, Namespace_HTML, "aside");
        InitializeElementName(&HTML_blockquote, Namespace_HTML, "blockquote");
        InitializeElementName(&HTML_center, Namespace_HTML, "center");
        InitializeElementName(&HTML_details, Namespace_HTML, "details");
        InitializeElementName(&HTML_dialog, Namespace_HTML, "dialog");
        InitializeElementName(&HTML_dir, Namespace_HTML, "dir");
        InitializeElementName(&HTML_div, Namespace_HTML, "div");
        InitializeElementName(&HTML_dl, Namespace_HTML, "dl");
        InitializeElementName(&HTML_figcaption, Namespace_HTML, "figcaption");
        InitializeElementName(&HTML_figure, Namespace_HTML, "figure");
        InitializeElementName(&HTML_footer, Namespace_HTML, "footer");
        InitializeElementName(&HTML_header, Namespace_HTML, "header");
        InitializeElementName(&HTML_hgroup, Namespace_HTML, "hgroup");
        InitializeElementName(&HTML_main, Namespace_HTML, "main");
        InitializeElementName(&HTML_menu, Namespace_HTML, "menu");
        InitializeElementName(&HTML_nav, Namespace_HTML, "nav");
        InitializeElementName(&HTML_section, Namespace_HTML, "section");
        InitializeElementName(&HTML_summary, Namespace_HTML, "summary");
        InitializeElementName(&HTML_h1, Namespace_HTML, "h1");
        InitializeElementName(&HTML_h2, Namespace_HTML, "h2");
        InitializeElementName(&HTML_h3, Namespace_HTML, "h3");
        InitializeElementName(&HTML_h4, Namespace_HTML, "h4");
        InitializeElementName(&HTML_h5, Namespace_HTML, "h5");
        InitializeElementName(&HTML_h6, Namespace_HTML, "h6");
        InitializeElementName(&HTML_pre, Namespace_HTML, "pre");
        InitializeElementName(&HTML_listing, Namespace_HTML, "listing");
        InitializeElementName(&HTML_form, Namespace_HTML, "form");
        InitializeElementName(&HTML_plaintext, Namespace_HTML, "plaintext");
        InitializeElementName(&HTML_area, Namespace_HTML, "area");
        InitializeElementName(&HTML_embed, Namespace_HTML, "embed");
        InitializeElementName(&HTML_img, Namespace_HTML, "img");
        InitializeElementName(&HTML_wbr, Namespace_HTML, "wbr");
        InitializeElementName(&HTML_menuitem, Namespace_HTML, "menuitem");
        InitializeElementName(&HTML_param, Namespace_HTML, "param");
        InitializeElementName(&HTML_source, Namespace_HTML, "source");
        InitializeElementName(&HTML_track, Namespace_HTML, "track");
        InitializeElementName(&HTML_hr, Namespace_HTML, "hr");
        InitializeElementName(&HTML_image, Namespace_HTML, "image");
        InitializeElementName(&HTML_isindex, Namespace_HTML, "isindex");
        InitializeElementName(&HTML_xmp, Namespace_HTML, "xmp");
        InitializeElementName(&HTML_iframe, Namespace_HTML, "iframe");
        InitializeElementName(&HTML_noembed, Namespace_HTML, "noembed");
        InitializeElementName(&HTML_ruby, Namespace_HTML, "ruby");
        InitializeElementName(&HTML_col, Namespace_HTML, "col");
        InitializeElementName(&HTML_frame, Namespace_HTML, "frame");
        InitializeElementName(&datalist_element_name, Namespace_HTML, "datalist");

        named_character_references_table = std::make_shared<StringMap>();

        // from http://www.whatwg.org/specs/web-apps/current-work/multipage/tokenization.html#consume-a-character-reference
        number_character_references_table.insert(TranslationMap::value_type(0x00, 0xFFFD)); // REPLACEMENT CHARACTER  
        number_character_references_table.insert(TranslationMap::value_type(0x0D, 0x000D)); // CARRIAGE RETURN (TranslationMap::value_type(CR)
        number_character_references_table.insert(TranslationMap::value_type(0x80, 0x20AC)); // EURO SIGN (TranslationMap::value_type(€)
        number_character_references_table.insert(TranslationMap::value_type(0x81, 0x0081)); // <control>
        number_character_references_table.insert(TranslationMap::value_type(0x82, 0x201A)); // SINGLE LOW-9 QUOTATION MARK (TranslationMap::value_type(‚)
        number_character_references_table.insert(TranslationMap::value_type(0x83, 0x0192)); // LATIN SMALL LETTER F WITH HOOK (TranslationMap::value_type(ƒ)
        number_character_references_table.insert(TranslationMap::value_type(0x84, 0x201E)); // DOUBLE LOW-9 QUOTATION MARK (TranslationMap::value_type(„)
        number_character_references_table.insert(TranslationMap::value_type(0x85, 0x2026)); // HORIZONTAL ELLIPSIS (TranslationMap::value_type(…)
        number_character_references_table.insert(TranslationMap::value_type(0x86, 0x2020)); // DAGGER (TranslationMap::value_type(†)
        number_character_references_table.insert(TranslationMap::value_type(0x87, 0x2021)); // DOUBLE DAGGER (TranslationMap::value_type(‡)
        number_character_references_table.insert(TranslationMap::value_type(0x88, 0x02C6)); // MODIFIER LETTER CIRCUMFLEX ACCENT (TranslationMap::value_type(ˆ)
        number_character_references_table.insert(TranslationMap::value_type(0x89, 0x2030)); // PER MILLE SIGN (TranslationMap::value_type(‰)
        number_character_references_table.insert(TranslationMap::value_type(0x8A, 0x0160)); // LATIN CAPITAL LETTER S WITH CARON (TranslationMap::value_type(Š)
        number_character_references_table.insert(TranslationMap::value_type(0x8B, 0x2039)); // SINGLE LEFT-POINTING ANGLE QUOTATION MARK (TranslationMap::value_type(‹)
        number_character_references_table.insert(TranslationMap::value_type(0x8C, 0x0152)); // LATIN CAPITAL LIGATURE OE (TranslationMap::value_type(Œ)
        number_character_references_table.insert(TranslationMap::value_type(0x8D, 0x008D)); // <control>
        number_character_references_table.insert(TranslationMap::value_type(0x8E, 0x017D)); // LATIN CAPITAL LETTER Z WITH CARON (TranslationMap::value_type(Ž)
        number_character_references_table.insert(TranslationMap::value_type(0x8F, 0x008F)); // <control>
        number_character_references_table.insert(TranslationMap::value_type(0x90, 0x0090)); // <control>
        number_character_references_table.insert(TranslationMap::value_type(0x91, 0x2018)); // LEFT SINGLE QUOTATION MARK (TranslationMap::value_type(‘)
        number_character_references_table.insert(TranslationMap::value_type(0x92, 0x2019)); // RIGHT SINGLE QUOTATION MARK (TranslationMap::value_type(’)
        number_character_references_table.insert(TranslationMap::value_type(0x93, 0x201C)); // LEFT DOUBLE QUOTATION MARK (TranslationMap::value_type(“)
        number_character_references_table.insert(TranslationMap::value_type(0x94, 0x201D)); // RIGHT DOUBLE QUOTATION MARK (TranslationMap::value_type(”)
        number_character_references_table.insert(TranslationMap::value_type(0x95, 0x2022)); // BULLET (TranslationMap::value_type(•)
        number_character_references_table.insert(TranslationMap::value_type(0x96, 0x2013)); // EN DASH (TranslationMap::value_type(–)
        number_character_references_table.insert(TranslationMap::value_type(0x97, 0x2014)); // EM DASH (TranslationMap::value_type(—)
        number_character_references_table.insert(TranslationMap::value_type(0x98, 0x02DC)); // SMALL TILDE (TranslationMap::value_type(˜)
        number_character_references_table.insert(TranslationMap::value_type(0x99, 0x2122)); // TRADE MARK SIGN (TranslationMap::value_type(™)
        number_character_references_table.insert(TranslationMap::value_type(0x9A, 0x0161)); // LATIN SMALL LETTER S WITH CARON (TranslationMap::value_type(š)
        number_character_references_table.insert(TranslationMap::value_type(0x9B, 0x203A)); // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK (TranslationMap::value_type(›)
        number_character_references_table.insert(TranslationMap::value_type(0x9C, 0x0153)); // LATIN SMALL LIGATURE OE (TranslationMap::value_type(œ)
        number_character_references_table.insert(TranslationMap::value_type(0x9D, 0x009D)); // <control>
        number_character_references_table.insert(TranslationMap::value_type(0x9E, 0x017E)); // LATIN SMALL LETTER Z WITH CARON (TranslationMap::value_type(ž)
        number_character_references_table.insert(TranslationMap::value_type(0x9F, 0x0178)); // LATIN CAPITAL LETTER Y WITH DIAERESIS (TranslationMap::value_type(Ÿ) 

        Scope.push_back(HTML_applet);
        Scope.push_back(HTML_caption);
        Scope.push_back(HTML_html);
        Scope.push_back(HTML_table);
        Scope.push_back(HTML_td);
        Scope.push_back(HTML_th);
        Scope.push_back(HTML_marquee);
        Scope.push_back(object_element_name);
        Scope.push_back(MathML_mi);
        Scope.push_back(MathML_mo);
        Scope.push_back(MathML_mn);
        Scope.push_back(MathML_ms);
        Scope.push_back(MathML_mtext);
        Scope.push_back(MathML_annotation_xml);
        Scope.push_back(SVG_foreignObject);
        Scope.push_back(SVG_desc);
        Scope.push_back(SVG_title);

        ListItemScope.insert(ListItemScope.end(), Scope.begin(), Scope.end());
        ListItemScope.push_back(HTML_ol);
        ListItemScope.push_back(HTML_ul);

        ButtonScope.insert(ButtonScope.end(), Scope.begin(), Scope.end());
        ButtonScope.push_back(button_element_name);

        TableScope.push_back(HTML_html);
        TableScope.push_back(HTML_table);

        SelectScope.push_back(HTML_optgroup);
        SelectScope.push_back(HTML_option);
    }
}