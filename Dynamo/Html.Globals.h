#pragma once

#include "Html.Types.h"
#include "Html.CharacterToken.h"
#include "Html.ElementName.h"
#include "Html.Node.h"
#include "Http.Client.h"
#include "Basic.Frame.h"
#include "Json.Parser.h"

namespace Http
{
	class Client;
}

namespace Html
{
	class Globals : public Frame
	{
	private:
		enum State
		{
			named_character_reference_state = Start_State,
			done_state = Succeeded_State,
		};

		Http::Client::Ref client; // $$$
		Json::Parser::Ref json_parser; // $$$
		Basic::Ref<IProcess> characters_completion; // $$$
		ByteString::Ref characters_cookie; // $$$

		template <int Count>
		void AddNamedCharacterReference(const char (&name)[Count], Codepoint c)
		{
			UnicodeString::Ref unicode_name = New<UnicodeString>();

			if (name[Count - 1] == 0)
				unicode_name->append(name, name + Count - 1);
			else
				unicode_name->append(name, name + Count);

			UnicodeString::Ref list = New<UnicodeString>();
			list->push_back(c);

			StringMap::value_type value(unicode_name, list);
			named_character_references_table->emplace(std::move(value));
		}

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
		void InitializeNamedCharacterReferences(Basic::Ref<IProcess> completion, ByteString::Ref cookie);

		virtual void IProcess::Process(IEvent* event, bool* yield);

		ElementNameList::Ref Scope; // $$$
		ElementNameList::Ref ListItemScope; // $$$
		ElementNameList::Ref ButtonScope; // $$$
		ElementNameList::Ref TableScope; // $$$
		ElementNameList::Ref SelectScope; // $$$

		StringMap::Ref named_character_references_table; // $$$

		TranslationMap::Ref number_character_references_table; // $$$

		UnicodeString::Ref Script; // $$$

		UnicodeString::Ref markup_declaration_comment; // $$$
		UnicodeString::Ref markup_declaration_doctype; // $$$
		UnicodeString::Ref markup_declaration_cdata; // $$$
		UnicodeString::Ref after_doctype_public_keyword; // $$$
		UnicodeString::Ref after_doctype_system_keyword; // $$$
		UnicodeString::Ref cdata_section_end; // $$$

		UnicodeString::Ref Namespace_HTML; // $$$
		UnicodeString::Ref Namespace_XML; // $$$
		UnicodeString::Ref Namespace_XMLNS; // $$$
		UnicodeString::Ref Namespace_MathML; // $$$
		UnicodeString::Ref Namespace_SVG; // $$$
		UnicodeString::Ref Namespace_XLink; // $$$

		UnicodeString::Ref DOCTYPE_html_4_0_public_identifier; // $$$
		UnicodeString::Ref DOCTYPE_html_4_0_system_identifier; // $$$
		UnicodeString::Ref DOCTYPE_html_4_01_public_identifier; // $$$
		UnicodeString::Ref DOCTYPE_html_4_01_system_identifier; // $$$
		UnicodeString::Ref DOCTYPE_xhtml_1_0_public_identifier; // $$$
		UnicodeString::Ref DOCTYPE_xhtml_1_0_system_identifier; // $$$
		UnicodeString::Ref DOCTYPE_xhtml_1_1_public_identifier; // $$$
		UnicodeString::Ref DOCTYPE_xhtml_1_1_system_identifier; // $$$
		UnicodeString::Ref DOCTYPE_legacy_compat; // $$$

		UnicodeString::Ref encoding_attribute_name; // $$$
		UnicodeString::Ref href_attribute_name; // $$$
		UnicodeString::Ref action_attribute_name; // $$$
		UnicodeString::Ref id_attribute_name; // $$$
		UnicodeString::Ref form_attribute_name; // $$$
		UnicodeString::Ref value_attribute_name; // $$$
		UnicodeString::Ref type_attribute_name; // $$$
		UnicodeString::Ref dirname_attribute_name; // $$$
		UnicodeString::Ref name_attribute_name; // $$$
		UnicodeString::Ref disabled_attribute_name; // $$$
		UnicodeString::Ref checked_attribute_name; // $$$
		UnicodeString::Ref enctype_attribute_name; // $$$
		UnicodeString::Ref method_attribute_name; // $$$
		UnicodeString::Ref target_attribute_name; // $$$
		UnicodeString::Ref accept_charset_attribute_name; // $$$
		UnicodeString::Ref class_attribute_name; // $$$

		UnicodeString::Ref on_value; // $$$

		UnicodeString::Ref checkbox_type; // $$$
		UnicodeString::Ref radio_type; // $$$
		UnicodeString::Ref image_type; // $$$
		UnicodeString::Ref file_type; // $$$
		UnicodeString::Ref object_type; // $$$
		UnicodeString::Ref hidden_type; // $$$
		UnicodeString::Ref text_type; // $$$

		UnicodeString::Ref _charset__name; // $$$
		UnicodeString::Ref isindex_name; // $$$

		UnicodeString::Ref text_html_media_type; // $$$
		UnicodeString::Ref application_xhtml_xml_media_type; // $$$

		UnicodeString::Ref codepoints_member_name; // $$$

		ElementName::Ref HTML_applet; // $$$
		ElementName::Ref HTML_body; // $$$
		ElementName::Ref HTML_br; // $$$
		ElementName::Ref button_element_name; // $$$
		ElementName::Ref HTML_caption; // $$$
		ElementName::Ref HTML_colgroup; // $$$
		ElementName::Ref HTML_frameset; // $$$
		ElementName::Ref HTML_head; // $$$
		ElementName::Ref HTML_html; // $$$
		ElementName::Ref HTML_marquee; // $$$
		ElementName::Ref object_element_name; // $$$
		ElementName::Ref HTML_ol; // $$$
		ElementName::Ref HTML_optgroup; // $$$
		ElementName::Ref HTML_option; // $$$
		ElementName::Ref select_element_name; // $$$
		ElementName::Ref HTML_table; // $$$
		ElementName::Ref HTML_tbody; // $$$
		ElementName::Ref HTML_td; // $$$
		ElementName::Ref HTML_tfoot; // $$$
		ElementName::Ref HTML_th; // $$$
		ElementName::Ref HTML_thead; // $$$
		ElementName::Ref HTML_tr; // $$$
		ElementName::Ref HTML_ul; // $$$
		ElementName::Ref HTML_a; // $$$
		ElementName::Ref HTML_b; // $$$
		ElementName::Ref HTML_big; // $$$
		ElementName::Ref HTML_code; // $$$
		ElementName::Ref HTML_em; // $$$
		ElementName::Ref HTML_font; // $$$
		ElementName::Ref HTML_i; // $$$
		ElementName::Ref HTML_nobr; // $$$
		ElementName::Ref HTML_s; // $$$
		ElementName::Ref HTML_small; // $$$
		ElementName::Ref HTML_strike; // $$$
		ElementName::Ref HTML_strong; // $$$
		ElementName::Ref HTML_tt; // $$$
		ElementName::Ref HTML_u; // $$$
		ElementName::Ref HTML_fieldset; // $$$
		ElementName::Ref input_element_name; // $$$
		ElementName::Ref HTML_keygen; // $$$
		ElementName::Ref HTML_label; // $$$
		ElementName::Ref HTML_output; // $$$
		ElementName::Ref HTML_textarea; // $$$
		ElementName::Ref HTML_base; // $$$
		ElementName::Ref HTML_basefont; // $$$
		ElementName::Ref HTML_bgsound; // $$$
		ElementName::Ref HTML_link; // $$$
		ElementName::Ref HTML_title; // $$$
		ElementName::Ref HTML_meta; // $$$
		ElementName::Ref HTML_noscript; // $$$
		ElementName::Ref HTML_noframes; // $$$
		ElementName::Ref HTML_style; // $$$
		ElementName::Ref HTML_script; // $$$
		ElementName::Ref HTML_dd; // $$$
		ElementName::Ref HTML_dt; // $$$
		ElementName::Ref HTML_li; // $$$
		ElementName::Ref HTML_p; // $$$
		ElementName::Ref HTML_rp; // $$$
		ElementName::Ref HTML_rt; // $$$
		ElementName::Ref HTML_address; // $$$
		ElementName::Ref HTML_article; // $$$
		ElementName::Ref HTML_aside; // $$$
		ElementName::Ref HTML_blockquote; // $$$
		ElementName::Ref HTML_center; // $$$
		ElementName::Ref HTML_details; // $$$
		ElementName::Ref HTML_dialog; // $$$
		ElementName::Ref HTML_dir; // $$$
		ElementName::Ref HTML_div; // $$$
		ElementName::Ref HTML_dl; // $$$
		ElementName::Ref HTML_figcaption; // $$$
		ElementName::Ref HTML_figure; // $$$
		ElementName::Ref HTML_footer; // $$$
		ElementName::Ref HTML_header; // $$$
		ElementName::Ref HTML_hgroup; // $$$
		ElementName::Ref HTML_main; // $$$
		ElementName::Ref HTML_menu; // $$$
		ElementName::Ref HTML_nav; // $$$
		ElementName::Ref HTML_section; // $$$
		ElementName::Ref HTML_summary; // $$$
		ElementName::Ref HTML_h1; // $$$
		ElementName::Ref HTML_h2; // $$$
		ElementName::Ref HTML_h3; // $$$
		ElementName::Ref HTML_h4; // $$$
		ElementName::Ref HTML_h5; // $$$
		ElementName::Ref HTML_h6; // $$$
		ElementName::Ref HTML_pre; // $$$
		ElementName::Ref HTML_listing; // $$$
		ElementName::Ref HTML_form; // $$$
		ElementName::Ref HTML_plaintext; // $$$
		ElementName::Ref HTML_area; // $$$
		ElementName::Ref HTML_embed; // $$$
		ElementName::Ref HTML_img; // $$$
		ElementName::Ref HTML_wbr; // $$$
		ElementName::Ref HTML_menuitem; // $$$
		ElementName::Ref HTML_param; // $$$
		ElementName::Ref HTML_source; // $$$
		ElementName::Ref HTML_track; // $$$
		ElementName::Ref HTML_hr; // $$$
		ElementName::Ref HTML_image; // $$$
		ElementName::Ref HTML_isindex; // $$$
		ElementName::Ref HTML_xmp; // $$$
		ElementName::Ref HTML_iframe; // $$$
		ElementName::Ref HTML_noembed; // $$$
		ElementName::Ref HTML_ruby; // $$$
		ElementName::Ref HTML_col; // $$$
		ElementName::Ref HTML_frame; // $$$
		ElementName::Ref datalist_element_name; // $$$

		ElementName::Ref MathML_annotation_xml; // $$$
		ElementName::Ref MathML_malignmark; // $$$
		ElementName::Ref MathML_mglyph; // $$$
		ElementName::Ref MathML_mi; // $$$
		ElementName::Ref MathML_mn; // $$$
		ElementName::Ref MathML_mo; // $$$
		ElementName::Ref MathML_ms; // $$$
		ElementName::Ref MathML_mtext; // $$$
		ElementName::Ref MathML_math; // $$$

		ElementName::Ref SVG_desc; // $$$
		ElementName::Ref SVG_foreignObject; // $$$
		ElementName::Ref SVG_svg; // $$$
		ElementName::Ref SVG_title; // $$$

		bool WSP(Codepoint c);
	};

	extern Inline<Globals>* globals;
}