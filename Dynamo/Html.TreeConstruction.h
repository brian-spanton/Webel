#pragma once

#include "Html.Types.h"
#include "Html.ElementNode.h"
#include "Html.TagToken.h"
#include "Basic.IStream.h"
#include "Html.Document.h"
#include "Html.DocTypeToken.h"
#include "Html.CharacterToken.h"
#include "Html.EndTagToken.h"
#include "Html.FormattingElement.h"

namespace Html
{
	using namespace Basic;

	class Parser;
	class Tokenizer;

	class TreeConstruction : public IStream<TokenPointer>
	{
	private:
		enum InsertionMode
		{
			initial_insertion_mode,
			before_html_insertion_mode,
			before_head_insertion_mode,
			in_head_insertion_mode,
			in_head_noscript_insertion_mode,
			after_head_insertion_mode,
			in_body_insertion_mode,
			text_insertion_mode,
			in_table_insertion_mode,
			in_table_text_insertion_mode,
			in_caption_insertion_mode,
			in_column_group_insertion_mode,
			in_table_body_insertion_mode,
			in_row_insertion_mode,
			in_cell_insertion_mode,
			in_select_insertion_mode,
			in_select_in_table_insertion_mode,
			after_body_insertion_mode,
			in_frameset_insertion_mode,
			after_frameset_insertion_mode,
			after_after_body_insertion_mode,
			after_after_frameset_insertion_mode,
		};

		typedef std::vector<ElementNode::Ref> ElementList;

		Parser* parser;
		int script_nesting_level;
		bool parser_pause_flag;
		InsertionMode insertion_mode;
		InsertionMode original_insertion_mode;
		ElementList open_elements;
		ElementNode::Ref fragment_context; // $$$
		FormattingElementList active_formatting_elements;
		ElementNode::Ref head_element; // $$$
		ElementNode::Ref form_element; // $$$
		bool scripting_flag;
		bool frameset_ok;
		bool ignore_line_feed;
		std::vector<CharacterToken::Ref> pending_table_character_tokens; // $$$

		ElementNode* CurrentNode();
		ElementNode* AdjustedCurrentNode();

		bool InForeignContent(TokenPointer token);
		bool IsValidDocType(DocTypeToken* doctype_token);
		Document::Mode QuirksMode(DocTypeToken* doctype_token);

		void HandleError(const char* error);
		void ParseError(const char* error);
		void ParseError(TokenPointer token, const char* error);
		void ParseError(TokenPointer token);
		void HandleNyi(const char* algorithm, bool log);
		void HandleNyi(TokenPointer token, bool log);
		void GetInsertionModeString(InsertionMode insertion_mode, char* mode_string, int count);

		void reset_the_insertion_mode_appropriately();
		void switch_the_insertion_mode(InsertionMode insertion_mode);

		bool has_element_in_specific_scope(ElementName* target, ElementNameList* list);
		bool has_element_in_specific_scope(TagToken* target, ElementNameList* list);
		bool has_element_in_specific_anti_scope(ElementName* target, ElementNameList* list);
		bool has_element_in_specific_anti_scope(TagToken* target, ElementNameList* list);

		bool has_element_in_scope(ElementNode* target);
		bool has_element_in_scope(ElementName* target);
		bool has_element_in_scope(TagToken* target);
		bool has_element_in_list_item_scope(TagToken* target);
		bool has_element_in_button_scope(ElementName* target);
		bool has_element_in_button_scope(TagToken* target);
		bool has_element_in_table_scope(ElementName* target);
		bool has_element_in_table_scope(TagToken* target);
		bool has_element_in_select_scope(ElementName* target);
		bool has_element_in_select_scope(TagToken* target);

		void tree_construction_dispatcher(TokenPointer token, bool* ignored);
		void apply_the_rules_for(InsertionMode insertion_mode, TokenPointer token, bool* ignored, bool* reprocess);
		void apply_the_rules_for_parsing_tokens_in_foreign_content(TokenPointer token, bool* ignored, bool* reprocess);
		void remove_node_from_the_stack_of_open_elements(ElementNode* node);
		void remove_node_from_the_list_of_active_formatting_elements(ElementNode* node);
		void create_an_element_for_a_token(TagToken* token, UnicodeString* name_space, ElementNode::Ref* element);
		void insert_an_HTML_element(TagToken* token, ElementNode::Ref* element);
		void insert_an_HTML_element(ElementNode* place, TagToken* token, ElementNode::Ref* element);
		void insert_a_foreign_element(TagToken* token, UnicodeString* name_space);
		void adjust_MathML_attributes(TagToken* token);
		void adjust_SVG_attributes(TagToken* token);
		void adjust_foreign_attributes(TagToken* token);
		void generic_raw_text_element_parsing_algorithm(TagToken* token);
		void generic_RCDATA_element_parsing_algorithm(TagToken* token);
		void generic_element_parsing_algorithm(TagToken* token, TokenizerState state);
		void generate_implied_end_tags();
		void generate_implied_end_tags(TagToken* except_for);
		void generate_implied_end_tags(ElementName* except_for);
		void adoption_agency_algorithm(EndTagToken* tag, bool* ignore);
		void foster_parent(ElementNode* node);
		void reconstruct_the_active_formatting_elements();
		void push_onto_the_list_of_active_formatting_elements(ElementNode* element, TagToken* token);
		void insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();
		void clear_the_list_of_active_formatting_elements_up_to_the_last_marker();
		void clear_the_stack_back_to_a_table_context();
		void clear_the_stack_back_to_a_table_body_context();
		void clear_the_stack_back_to_a_table_row_context();
		void close_the_cell();
		void act_as_if_a_start_tag_token_had_been_seen(ElementName* name);
		void act_as_if_an_end_tag_token_had_been_seen(ElementName* name, bool* ignored);
		void stop_parsing();
		void abort();
		bool is_in_the_stack_of_open_elements(ElementNode* element);
		bool is_in_the_list_of_active_formatting_elements(ElementNode* element);
		void in_table_insertion_mode_anything_else(TokenPointer token);
		void perform_a_microtask_checkpoint();
		void provide_a_stable_state();
		void push_onto_the_stack_of_open_elements(ElementList::value_type& value);
		void pop_the_current_node_off_the_stack_of_open_elements();
		void any_other_end_tag_in_body(EndTagToken* tag, bool* ignore);
		bool is_special(ElementName* name);
		void close_a_p_element();
		std::string StackString() const;

	public:
		typedef Basic::Ref<TreeConstruction> Ref;

		Document::Ref document; // $$$

		void Initialize(Parser* parser, Uri::Ref url);

		virtual void IStream<TokenPointer>::Write(const TokenPointer* elements, uint32 count);
		virtual void IStream<TokenPointer>::WriteEOF();
	};
}