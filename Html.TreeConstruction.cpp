// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.TreeConstruction.h"
#include "Html.Globals.h"
#include "Html.Types.h"
#include "Html.StartTagToken.h"
#include "Html.EndTagToken.h"
#include "Html.CharacterToken.h"
#include "Html.EndOfFileToken.h"
#include "Html.Tokenizer.h"
#include "Html.ElementNode.h"
#include "Html.Parser.h"
#include "Html.Document.h"
#include "Html.CommentNode.h"
#include "Html.FormattingElement.h"

namespace Html
{
    using namespace Basic;

    void TreeConstruction::Initialize(Parser* parser, Uri::Ref url)
    {
        this->parser = parser;
        this->script_nesting_level = 0;
        this->parser_pause_flag = false;
        this->insertion_mode = InsertionMode::initial_insertion_mode;
        this->original_insertion_mode = InsertionMode::initial_insertion_mode;
        this->scripting_flag = false;
        this->frameset_ok = true;
        this->document = New<Document>();
        this->document->Initialize(url);
        this->ignore_line_feed = false;
    }

    ElementNode* TreeConstruction::CurrentNode()
    {
        if (this->open_elements.size() == 0)
            return 0;

        return this->open_elements.back();
    }

    ElementNode* TreeConstruction::AdjustedCurrentNode()
    {
        if (this->fragment_context.item() != 0 && this->open_elements.size() == 1)
            return this->fragment_context;

        return CurrentNode();
    }

    bool TreeConstruction::InForeignContent(TokenPointer token)
    {
        if (this->AdjustedCurrentNode() == 0)
            return false;

        if (this->AdjustedCurrentNode()->is_in_namespace(Html::globals->Namespace_HTML))
            return false;

        if (this->AdjustedCurrentNode()->IsMathMLTextIntegrationPoint() && token->type == Token::Type::start_tag_token)
        {
            StartTagToken* tag = (StartTagToken*)token;
            if (tag->HasNameOf(Html::globals->MathML_mglyph) && tag->HasNameOf(Html::globals->MathML_malignmark))
                return false;
        }

        if (this->AdjustedCurrentNode()->IsMathMLTextIntegrationPoint() && token->type == Token::Type::character_token)
            return false;

        if (this->AdjustedCurrentNode()->has_element_name(Html::globals->MathML_annotation_xml) && token->type == Token::Type::start_tag_token)
        {
            StartTagToken* tag = (StartTagToken*)token;
            if (tag->HasNameOf(Html::globals->SVG_svg))
                return false;
        }

        if (this->AdjustedCurrentNode()->IsHTMLIntegrationPoint() && token->type == Token::Type::start_tag_token)
            return false;

        if (this->AdjustedCurrentNode()->IsHTMLIntegrationPoint() && token->type == Token::Type::character_token)
            return false;

        if (token->type == Token::Type::end_of_file_token)
            return false;

        return true;
    }

    void TreeConstruction::adjust_MathML_attributes(TagToken* token)
    {
        HandleNyi("TreeConstruction::adjust_MathML_attributes", true); // $ NYI
    }

    void TreeConstruction::adjust_SVG_attributes(TagToken* token)
    {
        HandleNyi("TreeConstruction::adjust_SVG_attributes", true); // $ NYI
    }

    void TreeConstruction::adjust_foreign_attributes(TagToken* token)
    {
        HandleNyi("TreeConstruction::adjust_foreign_attributes", true); // $ NYI
    }

    void TreeConstruction::remove_node_from_the_stack_of_open_elements(ElementNode* node)
    {
        for (ElementList::iterator it = this->open_elements.begin(); it != this->open_elements.end(); it++)
        {
            if (it->item() == node)
            {
                this->open_elements.erase(it);
                return;
            }
        }

        HandleError("TreeConstruction::remove_node_from_the_stack_of_open_elements");
    }

    void TreeConstruction::remove_node_from_the_list_of_active_formatting_elements(ElementNode* node)
    {
        for (FormattingElementList::iterator it = this->active_formatting_elements.begin(); it != this->active_formatting_elements.end(); it++)
        {
            if ((*it)->element.item() == node)
            {
                this->active_formatting_elements.erase(it);
                return;
            }
        }

        HandleError("TreeConstruction::remove_node_from_the_list_of_active_formatting_elements");
    }

    void TreeConstruction::create_an_element_for_a_token(TagToken* token, UnicodeString* name_space, ElementNode::Ref* element)
    {
        // http://www.whatwg.org/specs/web-apps/current-work/multipage/tree-construction.html#creating-and-inserting-elements
        // When the steps below require the UA to create an element for a token in a particular namespace, the UA must create a
        // node implementing the interface appropriate for the element type corresponding to the tag name of the token in the
        // given namespace (as given in the specification that defines that element, e.g. for an a element in the HTML namespace,
        // this specification defines it to be the HTMLAnchorElement interface), with the tag name being the name of that element,
        // with the node being in the given namespace, and with the attributes on the node being those given in the given token.
        //
        // The interface appropriate for an element in the HTML namespace that is not defined in this specification (or other
        // applicable specifications) is HTMLUnknownElement. Elements in other namespaces whose interface is not defined by that
        // namespace's specification must use the interface Element.
        //
        // When a resettable element is created in this manner, its reset algorithm must be invoked once the attributes are set.
        // (This initializes the element's value and checkedness based on the element's attributes.)
        HandleNyi("TreeConstruction::create_an_element_for_a_token", false); // $ NYI

        ElementName::Ref name = New<ElementName>();
        name->Initialize(name_space, token->name);

        ElementNode::Ref local_element = New<ElementNode>();
        local_element->Initialize(name, token->attributes);

        if (element != 0)
            (*element) = local_element;
    }

    void TreeConstruction::perform_a_microtask_checkpoint()
    {
        HandleNyi("TreeConstruction::perform_a_microtask_checkpoint", false); // $ NYI
    }

    void TreeConstruction::provide_a_stable_state()
    {
        HandleNyi("TreeConstruction::provide_a_stable_state", false); // $ NYI
    }

    void TreeConstruction::insert_an_HTML_element(ElementNode* place, TagToken* token, ElementNode::Ref* element)
    {
        // $ this algorithm is outdated.  see current section 12.2.5.1

        ElementNode::Ref local_element;
        create_an_element_for_a_token(token, Html::globals->Namespace_HTML, &local_element);

        if (local_element->IsFormAssociated()
            && this->form_element.item() != 0
            && !local_element->has_attribute(Html::globals->form_attribute_name))
        {
            // When a form-associated element or one of its ancestors is inserted into a Document, then the user agent must 
            // reset the form owner of that form-associated element. The HTML parser overrides this requirement when inserting 
            // form controls.

            // If an element created by the insert an HTML element algorithm is a form-associated element, and the form element 
            // pointer is not null, and the newly created element doesn't have a form attribute, the user agent must associate 
            // the newly created element with the form element pointed to by the form element pointer when the element is inserted, 
            // instead of running the reset the form owner algorithm.
            local_element->form_owner = this->form_element;
        }

        place->Append(local_element);
        push_onto_the_stack_of_open_elements(local_element);

        if (element != 0)
            (*element) = local_element;
    }

    void TreeConstruction::insert_an_HTML_element(TagToken* token, ElementNode::Ref* element)
    {
        insert_an_HTML_element(this->CurrentNode(), token, element);
    }

    void TreeConstruction::insert_a_foreign_element(TagToken* token, UnicodeString* name_space)
    {
        // When the steps below require the UA to insert a foreign element for a token, the UA must first create an element for the
        // token in the given namespace, and then append this node to the current node, and push it onto the stack of open elements
        // so that it is the new current node. If the newly created element has an xmlns attribute in the XMLNS namespace whose value
        // is not exactly the same as the element's namespace, that is a parse error. Similarly, if the newly created element has an
        // xmlns:xlink attribute in the XMLNS namespace whose value is not the XLink Namespace, that is a parse error.

        HandleNyi("TreeConstruction::insert_a_foreign_element", true); // $ NYI
    }

    void TreeConstruction::generic_raw_text_element_parsing_algorithm(TagToken* token)
    {
        generic_element_parsing_algorithm(token, TokenizerState::RAWTEXT_state);
    }

    void TreeConstruction::generic_RCDATA_element_parsing_algorithm(TagToken* token)
    {
        generic_element_parsing_algorithm(token, TokenizerState::RCDATA_state);
    }

    void TreeConstruction::generic_element_parsing_algorithm(TagToken* token, TokenizerState state)
    {
        insert_an_HTML_element(token, 0);
        this->parser->tokenizer->SwitchToState(state);
        switch_the_insertion_mode(InsertionMode::text_insertion_mode);
    }

    void TreeConstruction::generate_implied_end_tags()
    {
        generate_implied_end_tags((TagToken*)0);
    }

    void TreeConstruction::generate_implied_end_tags(TagToken* except_for)
    {
        while ((this->CurrentNode()->has_element_name(Html::globals->HTML_dd) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_dt) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_li) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_option) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_p) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rp) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rt)) &&
            (except_for == 0 || !except_for->HasNameOf(this->CurrentNode()->element_name)))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }
    }

    void TreeConstruction::generate_implied_end_tags(ElementName* except_for)
    {
        while ((this->CurrentNode()->has_element_name(Html::globals->HTML_dd) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_dt) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_li) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_option) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_p) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rp) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rt)) &&
            (except_for == 0 || !except_for->equals(this->CurrentNode()->element_name)))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }
    }

    void TreeConstruction::foster_parent(ElementNode* node)
    {
        HandleNyi("TreeConstruction::foster_parent", true); // $ NYI
    }

    void TreeConstruction::reconstruct_the_active_formatting_elements()
    {
        FormattingElement::Ref entry;

        // 1. If there are no entries in the list of active formatting elements, then there is nothing to reconstruct; stop this algorithm.
        if (this->active_formatting_elements.size() == 0)
            return;

        // 2. If the last (most recently added) entry in the list of active formatting elements is a marker, or if it is an element that is
        // in the stack of open elements, then there is nothing to reconstruct; stop this algorithm.
        if (this->active_formatting_elements.back()->IsMarker() ||
            is_in_the_stack_of_open_elements(this->active_formatting_elements.back()->element))
        {
            return;
        }

        // 3. Let entry be the last (most recently added) element in the list of active formatting elements.
        int entry_index = this->active_formatting_elements.size() - 1;

step_4:
        // 4. If there are no entries before entry in the list of active formatting elements, then jump to step 8.
        if (entry_index == 0)
            goto step_8;

        // 5. Let entry be the entry one earlier than entry in the list of active formatting elements.
        entry_index--;

        // 6. If entry is neither a marker nor an element that is also in the stack of open elements, go to step 4.
        entry = this->active_formatting_elements.at(entry_index);
        if (!entry->IsMarker() && !is_in_the_stack_of_open_elements(entry->element))
            goto step_4;

step_7:
        // 7. Let entry be the element one later than entry in the list of active formatting elements.
        entry_index++;
        entry = this->active_formatting_elements.at(entry_index);

        if (entry->IsMarker())
        {
            HandleError("state machine bug(?)"); // $
            return;
        }

step_8:
        // 8. Create an element for the token for which the element entry was created, to obtain new element.
        ElementNode::Ref new_element;
        create_an_element_for_a_token(entry->token, entry->element->element_name->name_space, &new_element);

        // 9. Append new element to the current node and push it onto the stack of open elements so that it is the new current node.
        this->CurrentNode()->Append(new_element);
        push_onto_the_stack_of_open_elements(new_element);

        // 10. Replace the entry for entry in the list with an entry for new element.
        entry->element = new_element;

        // 11. If the entry for new element in the list of active formatting elements is not the last entry in the list, return to step 7.
        if (entry != this->active_formatting_elements.back())
            goto step_7;

        // This has the effect of reopening all the formatting elements that were opened in the current body, cell, or caption (whichever
        // is youngest) that haven't been explicitly closed.

        // Note: The way this specification is written, the list of active formatting elements always consists of elements in
        // chronological order with the least recently added element first and the most recently added element last (except for 
        // while steps 8 to 11 of the above algorithm are being executed, of course).
    }

    void TreeConstruction::clear_the_stack_back_to_a_table_context()
    {
        while (!(this->CurrentNode()->has_element_name(Html::globals->HTML_table) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_html)))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }

        // Note: The current node being an html element after this process is a fragment case.
    }

    void TreeConstruction::clear_the_stack_back_to_a_table_body_context()
    {
        while (!(this->CurrentNode()->has_element_name(Html::globals->HTML_tbody) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_tfoot) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_thead) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_html)))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }

        // Note: The current node being an html element after this process is a fragment case.
    }

    void TreeConstruction::clear_the_stack_back_to_a_table_row_context()
    {
        while (!(this->CurrentNode()->has_element_name(Html::globals->HTML_tr) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_html)))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }

        // Note: The current node being an html element after this process is a fragment case.
    }

    void TreeConstruction::close_the_cell()
    {
        if (has_element_in_table_scope(Html::globals->HTML_td))
        {
            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_td, 0);
        }
        else if (has_element_in_table_scope(Html::globals->HTML_th))
        {
            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_th, 0);
        }

        // Note: The stack of open elements cannot have both a td and a th element in table scope at the same time, nor can
        // it have neither when the close the cell algorithm is invoked.
    }

    bool TreeConstruction::is_in_the_stack_of_open_elements(ElementNode* element)
    {
        for (ElementList::iterator it = this->open_elements.begin(); it != this->open_elements.end(); it++)
        {
            if (it->item() == element)
                return true;
        }

        return false;
    }

    bool TreeConstruction::is_in_the_list_of_active_formatting_elements(ElementNode* element)
    {
        for (FormattingElementList::iterator it = this->active_formatting_elements.begin(); it != this->active_formatting_elements.end(); it++)
        {
            if ((*it)->element.item() == element)
                return true;
        }

        return false;
    }

    void TreeConstruction::insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements()
    {
        FormattingElement::Ref marker = New<FormattingElement>();

        this->active_formatting_elements.push_back(marker);
    }

    void TreeConstruction::push_onto_the_list_of_active_formatting_elements(ElementNode* element, TagToken* token)
    {
        // 1. If there are already three elements in the list of active formatting elements after the last list marker, if any,
        // or anywhere in the list if there are no list markers, that have the same tag name, namespace, and attributes as element,
        // then remove the earliest such element from the list of active formatting elements. For these purposes, the attributes must
        // be compared as they were when the elements were created by the parser; two elements have the same attributes if all their 
        // parsed attributes can be paired such that the two attributes in each pair have identical names, namespaces, and values (the
        // order of the attributes does not matter).
        //
        // Note: This is the Noah's Ark clause. But with three per family instead of two.
        FormattingElementList::iterator earliest = this->active_formatting_elements.end();
        int count = 0;

        for (FormattingElementList::iterator it = this->active_formatting_elements.begin(); it != this->active_formatting_elements.end(); it++)
        {
            if ((*it)->IsMarker())
            {
                earliest = this->active_formatting_elements.end();
                count = 0;
            }
            else
            {
                if ((*it)->equals(element, token))
                {
                    count++;

                    if (earliest == this->active_formatting_elements.end())
                        earliest = it;
                }
            }
        }

        if (earliest != this->active_formatting_elements.end() && count == 3)
            this->active_formatting_elements.erase(earliest);

        // 2. Add element to the list of active formatting elements.
        FormattingElement::Ref formatting_element = New<FormattingElement>();
        formatting_element->Initialize(element, token);

        this->active_formatting_elements.push_back(formatting_element);
    }

    void TreeConstruction::clear_the_list_of_active_formatting_elements_up_to_the_last_marker()
    {
        while (true)
        {
            FormattingElement::Ref entry = this->active_formatting_elements.back();
            this->active_formatting_elements.pop_back();

            if (entry->IsMarker())
                return;
        }
    }

    void TreeConstruction::WriteEOF()
    {
    }

    void TreeConstruction::switch_the_insertion_mode(InsertionMode insertion_mode)
    {
        switch(insertion_mode)
        {
        case InsertionMode::text_insertion_mode:
        case InsertionMode::in_table_text_insertion_mode:
            this->original_insertion_mode = this->insertion_mode;
            break;
        }

        this->insertion_mode = insertion_mode;
    }

    std::string TreeConstruction::StackString() const
    {
        std::string result;

        for (ElementList::const_iterator it = this->open_elements.cbegin(); it != this->open_elements.cend(); it++)
        {
            result.push_back('/');
            result.insert(result.end(), (*it)->element_name->name->cbegin(), (*it)->element_name->name->cbegin() + (*it)->element_name->name->size());
        }

        return result;
    }

    void TreeConstruction::push_onto_the_stack_of_open_elements(ElementList::value_type& value)
    {
        this->open_elements.push_back(value);
    }

    void TreeConstruction::pop_the_current_node_off_the_stack_of_open_elements()
    {
        this->open_elements.pop_back();
    }

    void TreeConstruction::reset_the_insertion_mode_appropriately()
    {
        bool last = false;

        ElementList::reverse_iterator it = this->open_elements.rbegin();

        while(true)
        {
            ElementNode::Ref& node = (*it);

            // fragment case
            if (it == this->open_elements.rend() - 1)
            {
                last = true;
                node = this->fragment_context;
            }

            // fragment case
            if (node->has_element_name(Html::globals->select_element_name))
            {
                this->insertion_mode = InsertionMode::in_select_insertion_mode;
                return;
            }

            if ((node->has_element_name(Html::globals->HTML_td) || node->has_element_name(Html::globals->HTML_th)) && last == false)
            {
                this->insertion_mode = InsertionMode::in_cell_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_tr))
            {
                this->insertion_mode = InsertionMode::in_row_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_tbody) || node->has_element_name(Html::globals->HTML_thead) || node->has_element_name(Html::globals->HTML_tfoot))
            {
                this->insertion_mode = InsertionMode::in_table_body_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_caption))
            {
                this->insertion_mode = InsertionMode::in_caption_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_colgroup))
            {
                this->insertion_mode = InsertionMode::in_column_group_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_table))
            {
                this->insertion_mode = InsertionMode::in_table_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_head))
            {
                // Intentional: ("in body"!  not "in head"!)
                this->insertion_mode = InsertionMode::in_body_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_body))
            {
                this->insertion_mode = InsertionMode::in_body_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_frameset))
            {
                this->insertion_mode = InsertionMode::in_frameset_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_html))
            {
                this->insertion_mode = InsertionMode::before_head_insertion_mode;
                return;
            }

            // fragment case
            if (last == true)
            {
                this->insertion_mode = InsertionMode::in_body_insertion_mode;
                return;
            }

            it++;
        }
    }

    void TreeConstruction::in_table_insertion_mode_anything_else(TokenPointer token)
    {
        ParseError(token);

        // Process the token using the rules for the "in body" insertion mode, except that whenever a node would be 
        // inserted into the current node when the current node is a table, tbody, tfoot, thead, or tr element, then it 
        // must instead be foster parented.
        //apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0);
        HandleNyi("TreeConstruction::in_table_insertion_mode_anything_else", true); // $ NYI
    }

    bool TreeConstruction::has_element_in_specific_scope(ElementName* target, ElementNameList* list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if ((*it)->has_element_name(target))
                return true;

            for (ElementNameList::iterator it2 = list->begin(); it2 != list->end(); it2++)
            {
                if ((*it)->has_element_name(*it2))
                    return false;
            }
        }

        HandleError("TreeConstruction::has_element_in_specific_scope missing html root node");
        return false;
    }

    bool TreeConstruction::has_element_in_specific_scope(TagToken* target, ElementNameList* list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if (target->HasNameOf((*it)->element_name))
                return true;

            for (ElementNameList::iterator it2 = list->begin(); it2 != list->end(); it2++)
            {
                if ((*it)->has_element_name(*it2))
                    return false;
            }
        }

        HandleError("TreeConstruction::has_element_in_specific_scope missing html root node");
        return false;
    }

    bool TreeConstruction::has_element_in_specific_anti_scope(ElementName* target, ElementNameList* list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if ((*it)->has_element_name(target))
                return true;

            bool match = true;

            for (ElementNameList::iterator it2 = list->begin(); it2 != list->end(); it2++)
            {
                if ((*it)->has_element_name(*it2))
                {
                    match = false;
                    break;
                }
            }

            if (match)
                return true;
        }

        HandleError("TreeConstruction::has_element_in_specific_scope missing html root node");
        return false;
    }

    bool TreeConstruction::has_element_in_specific_anti_scope(TagToken* target, ElementNameList* list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if (target->HasNameOf((*it)->element_name))
                return true;

            bool match = true;

            for (ElementNameList::iterator it2 = list->begin(); it2 != list->end(); it2++)
            {
                if ((*it)->has_element_name(*it2))
                {
                    match = false;
                    break;
                }
            }

            if (match)
                return true;
        }

        HandleError("TreeConstruction::has_element_in_specific_scope missing html root node");
        return false;
    }

    bool TreeConstruction::has_element_in_scope(ElementNode* target)
    {
        return has_element_in_specific_scope(target->element_name, Html::globals->Scope);
    }

    bool TreeConstruction::has_element_in_scope(TagToken* token)
    {
        return has_element_in_specific_scope(token, Html::globals->Scope);
    }

    bool TreeConstruction::has_element_in_scope(ElementName* target)
    {
        return has_element_in_specific_scope(target, Html::globals->Scope);
    }

    bool TreeConstruction::has_element_in_list_item_scope(TagToken* target)
    {
        return has_element_in_specific_scope(target, Html::globals->ListItemScope);
    }

    bool TreeConstruction::has_element_in_button_scope(ElementName* target)
    {
        return has_element_in_specific_scope(target, Html::globals->ButtonScope);
    }

    bool TreeConstruction::has_element_in_button_scope(TagToken* target)
    {
        return has_element_in_specific_scope(target, Html::globals->ButtonScope);
    }

    bool TreeConstruction::has_element_in_table_scope(ElementName* target)
    {
        return has_element_in_specific_scope(target, Html::globals->TableScope);
    }

    bool TreeConstruction::has_element_in_table_scope(TagToken* target)
    {
        return has_element_in_specific_scope(target, Html::globals->TableScope);
    }

    bool TreeConstruction::has_element_in_select_scope(ElementName* target)
    {
        return has_element_in_specific_anti_scope(target, Html::globals->SelectScope);
    }

    bool TreeConstruction::has_element_in_select_scope(TagToken* target)
    {
        return has_element_in_specific_anti_scope(target, Html::globals->SelectScope);
    }

    bool TreeConstruction::is_special(ElementName* name)
    {
        if (name->equals(Html::globals->HTML_address) ||
            name->equals(Html::globals->HTML_applet) ||
            name->equals(Html::globals->HTML_area) || 
            name->equals(Html::globals->HTML_article) || 
            name->equals(Html::globals->HTML_aside) || 
            name->equals(Html::globals->HTML_base) ||
            name->equals(Html::globals->HTML_basefont) || 
            name->equals(Html::globals->HTML_bgsound) || 
            name->equals(Html::globals->HTML_blockquote) || 
            name->equals(Html::globals->HTML_body) || 
            name->equals(Html::globals->HTML_br) || 
            name->equals(Html::globals->button_element_name) || 
            name->equals(Html::globals->HTML_caption) || 
            name->equals(Html::globals->HTML_center) || 
            name->equals(Html::globals->HTML_col) || 
            name->equals(Html::globals->HTML_colgroup) || 
            name->equals(Html::globals->HTML_dd) || 
            name->equals(Html::globals->HTML_details) || 
            name->equals(Html::globals->HTML_dir) || 
            name->equals(Html::globals->HTML_div) || 
            name->equals(Html::globals->HTML_dl) || 
            name->equals(Html::globals->HTML_dt) || 
            name->equals(Html::globals->HTML_embed) || 
            name->equals(Html::globals->HTML_fieldset) || 
            name->equals(Html::globals->HTML_figcaption) || 
            name->equals(Html::globals->HTML_figure) || 
            name->equals(Html::globals->HTML_footer) || 
            name->equals(Html::globals->HTML_form) || 
            name->equals(Html::globals->HTML_frame) || 
            name->equals(Html::globals->HTML_frameset) || 
            name->equals(Html::globals->HTML_h1) || 
            name->equals(Html::globals->HTML_h2) || 
            name->equals(Html::globals->HTML_h3) || 
            name->equals(Html::globals->HTML_h4) || 
            name->equals(Html::globals->HTML_h5) || 
            name->equals(Html::globals->HTML_h6) || 
            name->equals(Html::globals->HTML_head) || 
            name->equals(Html::globals->HTML_header) || 
            name->equals(Html::globals->HTML_hgroup) || 
            name->equals(Html::globals->HTML_hr) || 
            name->equals(Html::globals->HTML_html) || 
            name->equals(Html::globals->HTML_iframe) ||  
            name->equals(Html::globals->HTML_img) || 
            name->equals(Html::globals->input_element_name) || 
            name->equals(Html::globals->HTML_isindex) || 
            name->equals(Html::globals->HTML_li) || 
            name->equals(Html::globals->HTML_link) || 
            name->equals(Html::globals->HTML_listing) || 
            name->equals(Html::globals->HTML_main) || 
            name->equals(Html::globals->HTML_marquee) || 
            name->equals(Html::globals->HTML_menu) || 
            name->equals(Html::globals->HTML_menuitem) || 
            name->equals(Html::globals->HTML_meta) || 
            name->equals(Html::globals->HTML_nav) || 
            name->equals(Html::globals->HTML_noembed) || 
            name->equals(Html::globals->HTML_noframes) || 
            name->equals(Html::globals->HTML_noscript) || 
            name->equals(Html::globals->object_element_name) || 
            name->equals(Html::globals->HTML_ol) || 
            name->equals(Html::globals->HTML_p) || 
            name->equals(Html::globals->HTML_param) || 
            name->equals(Html::globals->HTML_plaintext) || 
            name->equals(Html::globals->HTML_pre) || 
            name->equals(Html::globals->HTML_script) || 
            name->equals(Html::globals->HTML_section) || 
            name->equals(Html::globals->select_element_name) || 
            name->equals(Html::globals->HTML_source) || 
            name->equals(Html::globals->HTML_style) || 
            name->equals(Html::globals->HTML_summary) || 
            name->equals(Html::globals->HTML_table) || 
            name->equals(Html::globals->HTML_tbody) || 
            name->equals(Html::globals->HTML_td) || 
            name->equals(Html::globals->HTML_textarea) || 
            name->equals(Html::globals->HTML_tfoot) || 
            name->equals(Html::globals->HTML_th) || 
            name->equals(Html::globals->HTML_thead) || 
            name->equals(Html::globals->HTML_title) || 
            name->equals(Html::globals->HTML_tr) || 
            name->equals(Html::globals->HTML_track) || 
            name->equals(Html::globals->HTML_ul) || 
            name->equals(Html::globals->HTML_wbr) || 
            name->equals(Html::globals->HTML_xmp) ||
            name->equals(Html::globals->MathML_mi) ||
            name->equals(Html::globals->MathML_mo) || 
            name->equals(Html::globals->MathML_mn) || 
            name->equals(Html::globals->MathML_ms) || 
            name->equals(Html::globals->MathML_mtext) ||
            name->equals(Html::globals->MathML_annotation_xml) ||
            name->equals(Html::globals->SVG_foreignObject) ||
            name->equals(Html::globals->SVG_desc) || 
            name->equals(Html::globals->SVG_title))
        {
            return true;
        }

        return false;
    }
    
    void TreeConstruction::adoption_agency_algorithm(EndTagToken* tag, bool* ignored)
    {
        // 1. Let outer loop counter be zero.
        uint8 outer_loop_counter = 0;

outer_loop:
        // 2. Outer loop: If outer loop counter is greater than or equal to eight, then abort these steps.
        if (outer_loop_counter >= 8)
            return;

        // 3. Increment outer loop counter by one.
        outer_loop_counter += 1;

        // 4. Let the formatting element be the last element in the list of active formatting elements that:
        //     ◦ is between the end of the list and the last scope marker in the list, if any, or the start of the list otherwise, and
        //     ◦ has the same tag name as the token.
        FormattingElement::Ref formatting_element;
        for (uint32 counter = 0; counter < this->active_formatting_elements.size(); counter++)
        {
            int index = this->active_formatting_elements.size() - counter - 1;
            FormattingElement::Ref last_element = this->active_formatting_elements.at(index);

            if (last_element->IsMarker())
                break;

            if (tag->HasNameOf(last_element->element->element_name))
            {
                formatting_element = last_element;
                break;
            }
        }

        // If there is no such node, then abort these steps and instead act as described in the "any other end tag" entry below.
        if (formatting_element.item() == 0)
        {
            any_other_end_tag_in_body(tag, ignored);
            return;
        }

        // a number of steps below need to know where this element is in the stack of open elements
        int stack_index = -1;

        for (int index = 0; index != this->open_elements.size(); index++)
        {
            if (this->open_elements.at(index) == formatting_element->element)
            {
                stack_index = index;
                break;
            }
        }

        // Otherwise, if there is such a node, but that node is not in the stack of open elements, then this is a parse error;
        // remove the element from the list, and abort these steps.
        if (stack_index == -1)
        {
            ParseError(tag);
            remove_node_from_the_list_of_active_formatting_elements(formatting_element->element);
            return;
        }

        // Otherwise, if there is such a node, and that node is also in the stack of open elements, but the element is not in scope,
        // then this is a parse error; ignore the token, and abort these steps.
        if (!has_element_in_scope(formatting_element->element))
        {
            ParseError(tag);
            (*ignored) = true;
            return;
        }

        // Otherwise, there is a formatting element and that element is in the stack and is in scope. If the element is not the current node, 
        // this is a parse error. In any case, proceed with the algorithm as written in the following steps.
        if (formatting_element->element.item() != this->CurrentNode())
            ParseError(tag);

        // 5. Let the furthest block be the topmost node in the stack of open elements that is lower in the stack than the formatting element, 
        // and is an element in the special category. There might not be one.
        ElementNode::Ref furthest_block;
        ElementNode::Ref immediately_above;
        for (int counter = 0; counter != this->open_elements.size(); counter++)
        {
            int index = this->open_elements.size() - counter - 1;
            if (this->open_elements.at(index) == formatting_element->element)
                break;

            if (is_special(this->open_elements.at(index)->element_name))
            {
                furthest_block = this->open_elements.at(index);
                immediately_above = this->open_elements.at(index - 1);
            }
        }

        // 6. If there is no furthest block, then the UA must first pop all the nodes from the bottom of the stack of open elements,
        // from the current node up to and including the formatting element, then remove the formatting element from the list of active
        // formatting elements, and finally abort these steps.
        if (furthest_block.item() == 0)
        {
            while (true)
            {
                ElementNode::Ref node = this->CurrentNode();
                pop_the_current_node_off_the_stack_of_open_elements();

                if (node == formatting_element->element)
                    break;
            }

            remove_node_from_the_list_of_active_formatting_elements(formatting_element->element);

            return;
        }

        // 7. Let the common ancestor be the element immediately above the formatting element in the stack of open elements.
        ElementNode::Ref common_ancestor = this->open_elements.at(stack_index - 1);

        // 8. Let a bookmark note the position of the formatting element in the list of active formatting elements relative to the elements
        // on either side of it in the list.
        FormattingElement::Ref bookmark_after;
        for (uint32 index = 0; index < this->active_formatting_elements.size(); index++)
        {
            if (this->active_formatting_elements.at(index) == formatting_element)
            {
                bookmark_after = active_formatting_elements.at(index - 1);
                break;
            }
        }

        // 9. Let node and last node be the furthest block. Follow these steps:
        ElementNode::Ref node = furthest_block;
        ElementNode::Ref last_node = furthest_block;

        // 9.1. Let inner loop counter be zero.
        int inner_loop_counter = 0;

inner_loop:
        // 9.2. Inner loop: If inner loop counter is greater than or equal to three, then go to the next step in the overall algorithm.
        if (inner_loop_counter >= 3)
            goto step_10;

        // 9.3. Increment inner loop counter by one.
        inner_loop_counter++;

        // 9.4. Let node be the element immediately above node in the stack of open elements, or if node is no longer in the stack of open
        // elements (e.g. because it got removed by the next step), the element that was immediately above node in the stack of open elements 
        // before node was removed.
        node = immediately_above;

        // 9.5. If node is not in the list of active formatting elements, then remove node from the stack of open elements and then go back 
        // to the step labeled inner loop.
        if (!is_in_the_list_of_active_formatting_elements(node))
        {
            remove_node_from_the_stack_of_open_elements(node);
            goto inner_loop;
        }

        // 9.6. Otherwise, if node is the formatting element, then go to the next step in the overall algorithm.
        if (node == formatting_element->element)
            goto step_10;

        // 9.7. Create an element for the token for which the element node was created, replace the entry for node in the list of active 
        // formatting elements with an entry for the new element, replace the entry for node in the stack of open elements with an entry 
        // for the new element, and let node be the new element.
        for (uint32 index = 0; index < this->active_formatting_elements.size(); index++)
        {
            FormattingElement::Ref list_entry = this->active_formatting_elements.at(index);

            if (list_entry->element == node)
            {
                ElementNode::Ref new_element;
                create_an_element_for_a_token(list_entry->token, node->element_name->name_space, &new_element);

                list_entry->element = new_element;

                for (uint32 stack_index = 0; stack_index < this->open_elements.size(); stack_index++)
                {
                    if (this->open_elements.at(stack_index) == node)
                        this->open_elements[stack_index] = new_element;
                }

                node = new_element;

                // 9.8. If last node is the furthest block, then move the aforementioned bookmark to be immediately after the new node in the list
                // of active formatting elements.
                if (last_node == furthest_block)
                    bookmark_after = list_entry;

                break;
            }
        }

        // 9.9. Insert last node into node, first removing it from its previous parent node if any.
        last_node->remove_from_parent();
        node->Append(last_node);

        // 9.10. Let last node be node.
        last_node = node;

        // 9.11. Return to the step labeled inner loop.
        goto inner_loop;

step_10:
        // 10. If the common ancestor node is a table, tbody, tfoot, thead, or tr element, then, foster parent whatever last node ended up 
        // being in the previous step, first removing it from its previous parent node if any.
        if (common_ancestor->has_element_name(Html::globals->HTML_table) ||
            common_ancestor->has_element_name(Html::globals->HTML_tbody) ||
            common_ancestor->has_element_name(Html::globals->HTML_tfoot) ||
            common_ancestor->has_element_name(Html::globals->HTML_thead) ||
            common_ancestor->has_element_name(Html::globals->HTML_tr))
        {
            last_node->remove_from_parent();
            foster_parent(last_node);
        }
        else
        {
            // Otherwise, append whatever last node ended up being in the previous step to the common ancestor node, first removing it from its
            // previous parent node if any.
            last_node->remove_from_parent();
            common_ancestor->Append(last_node);
        }

        // 11. Create an element for the token for which the formatting element was created.
        ElementNode::Ref new_element;
        create_an_element_for_a_token(formatting_element->token, formatting_element->element->element_name->name_space, &new_element);

        // 12. Take all of the child nodes of the furthest block and append them to the element created in the last step.
        new_element->take_all_child_nodes_of(furthest_block);

        // 13. Append that new element to the furthest block.
        furthest_block->Append(new_element);

        // 14. Remove the formatting element from the list of active formatting elements, and insert the new element into the list of 
        // active formatting elements at the position of the aforementioned bookmark.
        remove_node_from_the_list_of_active_formatting_elements(formatting_element->element);

        FormattingElement::Ref new_formatting_element = New<FormattingElement>();
        new_formatting_element->Initialize(new_element, formatting_element->token);

        for (uint32 index = 0; index < this->active_formatting_elements.size(); index++)
        {
            if (this->active_formatting_elements.at(index) == bookmark_after)
                this->active_formatting_elements.insert(this->active_formatting_elements.begin() + index + 1, new_formatting_element);
        }

        // 15. Remove the formatting element from the stack of open elements, and insert the new element into the stack of open elements 
        // immediately below the position of the furthest block in that stack.
        remove_node_from_the_stack_of_open_elements(formatting_element->element);

        for (uint32 index = 0; index < this->open_elements.size(); index++)
        {
            if (this->open_elements.at(index) == furthest_block)
                this->open_elements.insert(this->open_elements.begin() + index + 1, new_element);
        }

        // 16. Jump back to the step labeled outer loop.
        goto outer_loop;
    }

    void TreeConstruction::any_other_end_tag_in_body(EndTagToken* tag, bool* ignored)
    {
        // 1. Initialize node to be the current node (the bottommost node of the stack).
        int node_index = this->open_elements.size() - 1;

loop:
        // 2. Loop: If node has the same tag name as the token, then:
        ElementNode::Ref node = this->open_elements.at(node_index);
        if (tag->HasNameOf(node->element_name))
        {
            // 2.1. Generate implied end tags, except for elements with the same tag name as the token.
            generate_implied_end_tags(tag);

            // 2.2. If the tag name of the end tag token does not match the tag name of the current node, this is a parse error.
            if (!tag->HasNameOf(node->element_name))
                ParseError(tag);

            // 2.3. Pop all the nodes from the current node up to node, including node, then stop these steps.
            while (this->open_elements.size() != node_index)
                pop_the_current_node_off_the_stack_of_open_elements();

            return;
        }

        // 3. Otherwise, if node is in the special category, then this is a parse error; ignore the token, and abort these steps.
        if (is_special(node->element_name))
        {
            ParseError(tag);

            (*ignored) = true;

            return;
        }

        // 4. Set node to the previous entry in the stack of open elements.
        node_index--;

        // 5. Return to the step labeled loop.
        goto loop;
    }

    bool TreeConstruction::IsValidDocType(DocTypeToken* doctype_token)
    {
        if (
            (
                !doctype_token->HasNameOf(Html::globals->HTML_html) ||
                doctype_token->public_identifier.item() != 0 ||
                (doctype_token->system_identifier.item() != 0 && !doctype_token->system_identifier.equals<true>(Html::globals->DOCTYPE_legacy_compat))
            )
            &&
            (
                !(doctype_token->public_identifier.equals<true>(Html::globals->DOCTYPE_html_4_0_public_identifier) &&
                    (doctype_token->system_identifier.item() == 0 || doctype_token->system_identifier.equals<true>(Html::globals->DOCTYPE_html_4_0_system_identifier)))
                    &&
                !(doctype_token->public_identifier.equals<true>(Html::globals->DOCTYPE_html_4_01_public_identifier) &&
                    (doctype_token->system_identifier.item() == 0 || doctype_token->system_identifier.equals<true>(Html::globals->DOCTYPE_html_4_01_system_identifier)))
                    &&
                !(doctype_token->public_identifier.equals<true>(Html::globals->DOCTYPE_xhtml_1_0_public_identifier) &&
                    (doctype_token->system_identifier.item() == 0 || doctype_token->system_identifier.equals<true>(Html::globals->DOCTYPE_xhtml_1_0_system_identifier)))
                    &&
                !(doctype_token->public_identifier.equals<true>(Html::globals->DOCTYPE_xhtml_1_1_public_identifier) &&
                    (doctype_token->system_identifier.item() == 0 || doctype_token->system_identifier.equals<true>(Html::globals->DOCTYPE_xhtml_1_1_system_identifier)))
            ))
            return false;

        return true;
    }

    Document::Mode TreeConstruction::QuirksMode(DocTypeToken* doctype_token)
    {
        // Then, if the DOCTYPE token matches one of the conditions in the following list, then set the Document to quirks mode:

        //• The force-quirks flag is set to on. 
        if (doctype_token->force_quirks)
            return Document::Mode::quirks_mode;

        //• The name is set to anything other than "html" (compared case-sensitively). 
        if (doctype_token->name.item() != 0 && !doctype_token->HasNameOf(Html::globals->HTML_html))
            return Document::Mode::quirks_mode;

        //• The public identifier starts with: "+//Silmaril//dtd html Pro v0r11 19970101//" 
        //• The public identifier starts with: "-//AdvaSoft Ltd//DTD HTML 3.0 asWedit + extensions//" 
        //• The public identifier starts with: "-//AS//DTD HTML 3.0 asWedit + extensions//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.0 Level 1//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.0 Level 2//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.0 Strict Level 1//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.0 Strict Level 2//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.0 Strict//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.0//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 2.1E//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 3.0//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 3.2 Final//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 3.2//" 
        //• The public identifier starts with: "-//IETF//DTD HTML 3//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Level 0//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Level 1//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Level 2//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Level 3//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Strict Level 0//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Strict Level 1//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Strict Level 2//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Strict Level 3//" 
        //• The public identifier starts with: "-//IETF//DTD HTML Strict//" 
        //• The public identifier starts with: "-//IETF//DTD HTML//" 
        //• The public identifier starts with: "-//Metrius//DTD Metrius Presentational//" 
        //• The public identifier starts with: "-//Microsoft//DTD Internet Explorer 2.0 HTML Strict//" 
        //• The public identifier starts with: "-//Microsoft//DTD Internet Explorer 2.0 HTML//" 
        //• The public identifier starts with: "-//Microsoft//DTD Internet Explorer 2.0 Tables//" 
        //• The public identifier starts with: "-//Microsoft//DTD Internet Explorer 3.0 HTML Strict//" 
        //• The public identifier starts with: "-//Microsoft//DTD Internet Explorer 3.0 HTML//" 
        //• The public identifier starts with: "-//Microsoft//DTD Internet Explorer 3.0 Tables//" 
        //• The public identifier starts with: "-//Netscape Comm. Corp.//DTD HTML//" 
        //• The public identifier starts with: "-//Netscape Comm. Corp.//DTD Strict HTML//" 
        //• The public identifier starts with: "-//O'Reilly and Associates//DTD HTML 2.0//" 
        //• The public identifier starts with: "-//O'Reilly and Associates//DTD HTML Extended 1.0//" 
        //• The public identifier starts with: "-//O'Reilly and Associates//DTD HTML Extended Relaxed 1.0//" 
        //• The public identifier starts with: "-//SoftQuad Software//DTD HoTMetaL PRO 6.0::19990601::extensions to HTML 4.0//" 
        //• The public identifier starts with: "-//SoftQuad//DTD HoTMetaL PRO 4.0::19971010::extensions to HTML 4.0//" 
        //• The public identifier starts with: "-//Spyglass//DTD HTML 2.0 Extended//" 
        //• The public identifier starts with: "-//SQ//DTD HTML 2.0 HoTMetaL + extensions//" 
        //• The public identifier starts with: "-//Sun Microsystems Corp.//DTD HotJava HTML//" 
        //• The public identifier starts with: "-//Sun Microsystems Corp.//DTD HotJava Strict HTML//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 3 1995-03-24//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 3.2 Draft//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 3.2 Final//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 3.2//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 3.2S Draft//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 4.0 Frameset//" 
        //• The public identifier starts with: "-//W3C//DTD HTML 4.0 Transitional//" 
        //• The public identifier starts with: "-//W3C//DTD HTML Experimental 19960712//" 
        //• The public identifier starts with: "-//W3C//DTD HTML Experimental 970421//" 
        //• The public identifier starts with: "-//W3C//DTD W3 HTML//" 
        //• The public identifier starts with: "-//W3O//DTD W3 HTML 3.0//" 
        //• The public identifier is set to: "-//W3O//DTD W3 HTML Strict 3.0//EN//" 
        //• The public identifier starts with: "-//WebTechs//DTD Mozilla HTML 2.0//" 
        //• The public identifier starts with: "-//WebTechs//DTD Mozilla HTML//" 
        //• The public identifier is set to: "-/W3C/DTD HTML 4.0 Transitional/EN" 
        //• The public identifier is set to: "HTML" 
        //• The system identifier is set to: "http://www.ibm.com/data/dtd/v11/ibmxhtml1-transitional.dtd" 
        //• The system identifier is missing and the public identifier starts with: "-//W3C//DTD HTML 4.01 Frameset//" 
        //• The system identifier is missing and the public identifier starts with: "-//W3C//DTD HTML 4.01 Transitional//" 
        //
        //Otherwise, if the DOCTYPE token matches one of the conditions in the following list, then set the Document to limited-quirks mode:
        //• The public identifier starts with: "-//W3C//DTD XHTML 1.0 Frameset//" 
        //• The public identifier starts with: "-//W3C//DTD XHTML 1.0 Transitional//" 
        //• The system identifier is not missing and the public identifier starts with: "-//W3C//DTD HTML 4.01 Frameset//" 
        //• The system identifier is not missing and the public identifier starts with: "-//W3C//DTD HTML 4.01 Transitional//" 
        //
        //The system identifier and public identifier strings must be compared to the values given in the lists above in an ASCII case-insensitive manner. A system identifier whose value is the empty string is not considered missing for the purposes of the conditions above.
        HandleNyi("TreeConstruction::QuirksMode", false); // $ NYI

        return Document::Mode::no_quirks_mode;
    }

    void TreeConstruction::GetInsertionModeString(InsertionMode insertion_mode, char* mode_string, int count)
    {
        switch (insertion_mode)
        {
#define CASE(e) \
        case e: \
            strcpy_s(mode_string, count, #e); \
            break

            CASE(initial_insertion_mode);
            CASE(before_html_insertion_mode);
            CASE(before_head_insertion_mode);
            CASE(in_head_insertion_mode);
            CASE(in_head_noscript_insertion_mode);
            CASE(after_head_insertion_mode);
            CASE(in_body_insertion_mode);
            CASE(text_insertion_mode);
            CASE(in_table_insertion_mode);
            CASE(in_table_text_insertion_mode);
            CASE(in_caption_insertion_mode);
            CASE(in_column_group_insertion_mode);
            CASE(in_table_body_insertion_mode);
            CASE(in_row_insertion_mode);
            CASE(in_cell_insertion_mode);
            CASE(in_select_insertion_mode);
            CASE(in_select_in_table_insertion_mode);
            CASE(after_body_insertion_mode);
            CASE(in_frameset_insertion_mode);
            CASE(after_frameset_insertion_mode);
            CASE(after_after_body_insertion_mode);
            CASE(after_after_frameset_insertion_mode);

#undef CASE

        default:
            sprintf_s(mode_string, count, "%d", this->insertion_mode);
            break;
        }
    }

    void TreeConstruction::HandleError(const char* error)
    {
        char mode_string[0x40];
        GetInsertionModeString(this->insertion_mode, mode_string, _countof(mode_string));

        char full_error[0x100];
        sprintf_s(full_error, "stack=%s; insertion_mode=%s; %s", StackString().c_str(), mode_string, error);

        Basic::globals->HandleError(full_error, 0);
    }

    void TreeConstruction::ParseError(const char* error)
    {
        char full_error[0x100];
        sprintf_s(full_error, "parse error=%s", error);

        HandleError(full_error);
    }

    void TreeConstruction::ParseError(TokenPointer token)
    {
        char token_string[0x40];
        token->GetDebugString(token_string, _countof(token_string));

        char full_error[0x100];
        sprintf_s(full_error, "token=%s", token_string);

        ParseError(full_error);
    }

    void TreeConstruction::ParseError(TokenPointer token, const char* error)
    {
        char token_string[0x40];
        token->GetDebugString(token_string, _countof(token_string));

        char full_error[0x100];
        sprintf_s(full_error, "%s; token=%s", error, token_string);

        ParseError(full_error);
    }

    void TreeConstruction::HandleNyi(TokenPointer token, bool log)
    {
        char token_string[0x40];
        token->GetDebugString(token_string, _countof(token_string));

        char full_error[0x100];
        sprintf_s(full_error, "token=%s", token_string);

        HandleNyi(full_error, log);
    }

    void TreeConstruction::HandleNyi(const char* algorithm, bool log)
    {
        if (log)
        {
            char full_error[0x100];
            sprintf_s(full_error, "nyi=%s", algorithm);

            HandleError(full_error);
        }
    }

    void TreeConstruction::stop_parsing()
    {
        // 1. Set the current document readiness to "interactive" and the insertion point to undefined.
        HandleNyi("TreeConstruction::stop_parsing", false); // $ NYI

        // 2. Pop all the nodes off the stack of open elements.
        while (this->open_elements.size() > 0)
            pop_the_current_node_off_the_stack_of_open_elements();

        // 3. If the list of scripts that will execute when the document has finished parsing is not empty, run these substeps:
        // 3.1. Spin the event loop until the first script in the list of scripts that will execute when the document has finished 
        // parsing has its "ready to be parser-executed" flag set and the parser's Document has no style sheet that is blocking scripts.
        //
        // 3.2. Execute the first script in the list of scripts that will execute when the document has finished parsing.
        //
        // 3.3. Remove the first script element from the list of scripts that will execute when the document has finished parsing (i.e.
        // shift out the first entry in the list).
        //
        // 3.4. If the list of scripts that will execute when the document has finished parsing is still not empty, repeat these substeps
        // again from substep 1.
        //
        //
        // 4. Queue a task to fire a simple event that bubbles named DOMContentLoaded at the Document.
        //
        // 5. Spin the event loop until the set of scripts that will execute as soon as possible and the list of scripts that will execute
        // in order as soon as possible are empty.
        //
        // 6. Spin the event loop until there is nothing that delays the load event in the Document.
        //
        // 7. Queue a task to run the following substeps:
        // 7.1. Set the current document readiness to "complete".
        //
        // 7.2. If the Document is in a browsing context, fire a simple event named load at the Document's Window object, but with its target
        // set to the Document object (and the currentTarget set to the Window object).
        //
        //
        // 8. If the Document is in a browsing context, then queue a task to run the following substeps:
        // 8.1. If the Document's page showing flag is true, then abort this task (i.e. don't fire the event below).
        //
        // 8.2. Set the Document's page showing flag to true.
        //
        // 8.3. Fire a trusted event with the name pageshow at the Window object of the Document, but with its target set to the Document
        // object (and the currentTarget set to the Window object), using the PageTransitionEvent interface, with the persisted attribute
        // initialized to false. This event must not bubble, must not be cancelable, and has no default action.
        //
        // 9. If the Document has any pending application cache download process tasks, then queue each such task in the order they were
        // added to the list of pending application cache download process tasks, and then empty the list of pending application cache
        // download process tasks. The task source for these tasks is the networking task source.
        //
        // 10. If the Document's print when loaded flag is set, then run the printing steps.
        //
        // 11. The Document is now ready for post-load tasks.
        //
        // 12. Queue a task to mark the Document as completely loaded.
    }

    void TreeConstruction::abort()
    {
        // 1. Throw away any pending content in the input stream, and discard any future content that would have been added to it.
        //
        // 2. Set the current document readiness to "interactive".
        //
        // 3. Pop all the nodes off the stack of open elements.
        //
        // 4. Set the current document readiness to "complete".
        // 
        // Except where otherwise specified, the task source for the tasks mentioned in this section is the DOM manipulation task source.
    }

    void TreeConstruction::act_as_if_a_start_tag_token_had_been_seen(ElementName* name)
    {
        StartTagToken::Ref tag = New<StartTagToken>();
        tag->name = name->name;

        tree_construction_dispatcher(tag, 0);
    }

    void TreeConstruction::act_as_if_an_end_tag_token_had_been_seen(ElementName* name, bool* ignored)
    {
        EndTagToken::Ref tag = New<EndTagToken>();
        tag->name = name->name;

        tree_construction_dispatcher(tag, ignored);
    }

    void TreeConstruction::close_a_p_element()
    {
        generate_implied_end_tags(Html::globals->HTML_p);

        if (this->CurrentNode()->element_name->equals(Html::globals->HTML_p) == false)
            ParseError("close_a_p_element");

        while (true)
        {
            ElementNode::Ref popped = this->CurrentNode();
            pop_the_current_node_off_the_stack_of_open_elements();

            if (this->CurrentNode()->element_name->equals(Html::globals->HTML_p))
                break;
        }
    }

    void TreeConstruction::Write(const TokenPointer* elements, uint32 count)
    {
        for (uint32 i = 0; i < count; i++)
        {
            TokenPointer token = elements[i];
            tree_construction_dispatcher(token, 0);
        }
    }

    void TreeConstruction::tree_construction_dispatcher(TokenPointer token, bool* ignored)
    {
        bool reprocess;

        if (InForeignContent(token))
            apply_the_rules_for_parsing_tokens_in_foreign_content(token, ignored, &reprocess);
        else
            apply_the_rules_for(this->insertion_mode, token, ignored, &reprocess);

        if (reprocess)
            tree_construction_dispatcher(token, ignored);
    }

    void TreeConstruction::apply_the_rules_for(InsertionMode insertion_mode, TokenPointer token, bool* ignored, bool* reprocess)
    {
        bool local_reprocess;

        if (reprocess == 0)
            reprocess = &local_reprocess;

        (*reprocess) = false;

        bool local_ignore;

        if (ignored == 0)
            ignored = &local_ignore;

        (*ignored) = false;

        // If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to
        // the next one. (Newlines at the start of textarea elements are ignored as an authoring convenience.)

        if (this->ignore_line_feed)
        {
            this->ignore_line_feed = false;

            if (token->type == Token::Type::character_token)
            {
                CharacterToken* character_token = (CharacterToken*)token;

                if (character_token->data == 0x000A)
                {
                    (*ignored) = true;
                    return;
                }
            }
        }

        switch (insertion_mode)
        {
        case initial_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            (*ignored) = true;
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    {
                        DocTypeToken* doctype_token = (DocTypeToken*)token;

                        if (!IsValidDocType(doctype_token))
                            ParseError("!IsValidDocType(doctype_token)");

                        DocumentTypeNode::Ref doctype_node = New<DocumentTypeNode>();
                        doctype_node->name = doctype_token->name.item() == 0 ? New<UnicodeString>() : doctype_token->name;
                        doctype_node->publicId = doctype_token->public_identifier.item() == 0 ? New<UnicodeString>() : doctype_token->public_identifier;
                        doctype_node->systemId = doctype_token->system_identifier.item() == 0 ? New<UnicodeString>() : doctype_token->system_identifier;

                        // and the other attributes specific to DocumentType objects set to null and empty lists as appropriate.

                        this->document->Append(doctype_node);
                        this->document->doctype = doctype_node;
                        this->document->mode = QuirksMode(doctype_token);

                        switch_the_insertion_mode(InsertionMode::before_html_insertion_mode);
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    // If the document is not an iframe srcdoc document, then this is a parse error; set the Document to quirks mode.
                    HandleNyi(token, true); // $ NYI

                    // In any case, switch the insertion mode to "before html", then reprocess the current token.
                    switch_the_insertion_mode(InsertionMode::before_html_insertion_mode);
                    (*reprocess) = true;
                }
            }
            break;

        case before_html_insertion_mode:
            {
                bool anything_else = false;

                switch(token->type)
                {
                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            (*ignored) = true;
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            ElementNode::Ref element;
                            create_an_element_for_a_token(tag, Html::globals->Namespace_HTML, &element);

                            this->document->Append(element);
                            push_onto_the_stack_of_open_elements(element);

                            // If the Document is being loaded as part of navigation of a browsing context, then: if the newly 
                            // created element has a manifest attribute whose value is not the empty string, then resolve the 
                            // value of that attribute to an absolute URL, relative to the newly created element, and if that is 
                            // successful, run the application cache selection algorithm with the result of applying the URL 
                            // serializer algorithm to the resulting parsed URL with the exclude fragment flag set; otherwise, if
                            // there is no such attribute, or its value is the empty string, or resolving its value fails, run the
                            // application cache selection algorithm with no manifest. The algorithm must be passed the Document object.
                            HandleNyi(token, false); // $ NYI

                            switch_the_insertion_mode(InsertionMode::before_head_insertion_mode);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_head) ||
                            tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_br))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token, "not head, body, html, or br");
                            (*ignored) = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ElementNode::Ref element = New<ElementNode>();
                    element->Initialize(Html::globals->HTML_html, New<StringMap>());

                    push_onto_the_stack_of_open_elements(element);

                    // If the Document is being loaded as part of navigation of a browsing context, then: run the application 
                    // cache selection algorithm with no manifest, passing it the Document object.
                    HandleNyi(token, true); // $ NYI

                    switch_the_insertion_mode(InsertionMode::before_head_insertion_mode);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::before_head_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            (*ignored) = true;
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_head))
                        {
                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);

                            this->head_element = element;

                            switch_the_insertion_mode(InsertionMode::in_head_insertion_mode);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_head) ||
                            tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_br))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_head);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::in_head_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            this->CurrentNode()->Insert(character_token->data);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_base) ||
                            tag->HasNameOf(Html::globals->HTML_basefont) ||
                            tag->HasNameOf(Html::globals->HTML_bgsound) ||
                            tag->HasNameOf(Html::globals->HTML_link))
                        {
                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);
                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_meta))
                        {
                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);
                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            // If the element has a charset attribute, and getting an encoding from its value results in a supported
                            // ASCII-compatible character encoding or a UTF-16 encoding, and the confidence is currently tentative, 
                            // then change the encoding to the resulting encoding.
                            //
                            // Otherwise, if the element has an http-equiv attribute whose value is an ASCII case-insensitive match 
                            // for the string "Content-Type", and the element has a content attribute, and applying the algorithm for
                            // extracting a character encoding from a meta element to that attribute's value returns a supported 
                            // ASCII-compatible character encoding or a UTF-16 encoding, and the confidence is currently tentative, 
                            // then change the encoding to the extracted encoding.
                            HandleNyi(token, false); // $ NYI
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_title))
                        {
                            generic_RCDATA_element_parsing_algorithm(tag);
                        }
                        else if ((tag->HasNameOf(Html::globals->HTML_noscript) && this->scripting_flag) ||
                            tag->HasNameOf(Html::globals->HTML_noframes) ||
                            tag->HasNameOf(Html::globals->HTML_style))
                        {
                            generic_raw_text_element_parsing_algorithm(tag);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_noscript) && !this->scripting_flag)
                        {
                            insert_an_HTML_element(tag, 0);
                            switch_the_insertion_mode(InsertionMode::in_head_noscript_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_script))
                        {
                            ElementNode::Ref element;
                            create_an_element_for_a_token(tag, Html::globals->Namespace_HTML, &element);

                            // 2. Mark the element as being "parser-inserted" and unset the element's "force-async" flag.
                            //
                            // Note: 
                            //
                            // This ensures that, if the script is external, any document.write() calls in the script will execute in-line,
                            // instead of blowing the document away, as would happen in most other cases. It also prevents the script from
                            // executing until the end tag is seen.
                            //
                            // 3. If the parser was originally created for the HTML fragment parsing algorithm, then mark the script element 
                            // as "already started". (fragment case)
                            HandleNyi(token, false); // $ NYI

                            this->CurrentNode()->Append(element);
                            push_onto_the_stack_of_open_elements(element);

                            this->parser->tokenizer->SwitchToState(TokenizerState::script_data_state);
                            this->original_insertion_mode = this->insertion_mode;
                            this->insertion_mode = InsertionMode::text_insertion_mode;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_head))
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_head))
                        {
                            pop_the_current_node_off_the_stack_of_open_elements();
                            switch_the_insertion_mode(InsertionMode::after_head_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_br))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_head, 0);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::in_head_noscript_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_basefont) ||
                            tag->HasNameOf(Html::globals->HTML_bgsound) ||
                            tag->HasNameOf(Html::globals->HTML_link) ||
                            tag->HasNameOf(Html::globals->HTML_meta) ||
                            tag->HasNameOf(Html::globals->HTML_noframes) ||
                            tag->HasNameOf(Html::globals->HTML_style))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_head) ||
                            tag->HasNameOf(Html::globals->HTML_noscript))
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_noscript))
                        {
                            pop_the_current_node_off_the_stack_of_open_elements();
                            switch_the_insertion_mode(InsertionMode::in_head_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_br))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                    }
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_noscript, 0);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::after_head_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            this->CurrentNode()->Insert(character_token->data);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body))
                        {
                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);

                            this->frameset_ok = false;

                            switch_the_insertion_mode(InsertionMode::in_body_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_frameset))
                        {
                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);

                            switch_the_insertion_mode(InsertionMode::in_frameset_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_base) ||
                            tag->HasNameOf(Html::globals->HTML_basefont) ||
                            tag->HasNameOf(Html::globals->HTML_bgsound) ||
                            tag->HasNameOf(Html::globals->HTML_link) ||
                            tag->HasNameOf(Html::globals->HTML_meta) ||
                            tag->HasNameOf(Html::globals->HTML_noframes) ||
                            tag->HasNameOf(Html::globals->HTML_script) ||
                            tag->HasNameOf(Html::globals->HTML_style) ||
                            tag->HasNameOf(Html::globals->HTML_title))
                        {
                            ParseError(token);

                            push_onto_the_stack_of_open_elements(this->head_element);

                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);

                            remove_node_from_the_stack_of_open_elements(this->head_element);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_head))
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_br))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_body);

                    this->frameset_ok = true;

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::in_body_insertion_mode:
            {
                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0000:
                            ParseError(token);
                            (*ignored) = true;
                            break;

                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            reconstruct_the_active_formatting_elements();
                            this->CurrentNode()->Insert(character_token->data);
                            break;

                        default:
                            reconstruct_the_active_formatting_elements();
                            this->CurrentNode()->Insert(character_token->data);
                            this->frameset_ok = false;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            ParseError(token);

                            // For each attribute on the token, check to see if the attribute is already present on the top element of 
                            // the stack of open elements. If it is not, add the attribute and its corresponding value to that element.
                            HandleNyi(token, true); // $ NYI
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_base) ||
                            tag->HasNameOf(Html::globals->HTML_basefont) ||
                            tag->HasNameOf(Html::globals->HTML_bgsound) ||
                            tag->HasNameOf(Html::globals->HTML_link) ||
                            tag->HasNameOf(Html::globals->HTML_meta) ||
                            tag->HasNameOf(Html::globals->HTML_noframes) ||
                            tag->HasNameOf(Html::globals->HTML_script) ||
                            tag->HasNameOf(Html::globals->HTML_style) ||
                            tag->HasNameOf(Html::globals->HTML_title))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body))
                        {
                            ParseError(token);

                            if (this->open_elements.size() == 1 || !this->open_elements[1]->has_element_name(Html::globals->HTML_body))
                            {
                                // fragment case
                                (*ignored) = true;
                            }
                            else
                            {
                                this->frameset_ok = false;

                                ElementNode::Ref element = this->open_elements[1];

                                // for each attribute on the token, check to see if the attribute is already present on the body 
                                // (the second element) on the stack of open elements, and if it is not, add the attribute and its 
                                // corresponding value to that element.
                                HandleNyi(token, false); // $ NYI
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_frameset))
                        {
                            ParseError(token);

                            if (this->open_elements.size() == 1 || !this->open_elements[1]->has_element_name(Html::globals->HTML_body))
                            {
                                // fragment case
                                (*ignored) = true;
                            }
                            else if (this->frameset_ok == false)
                            {
                                (*ignored) = true;
                            }
                            else
                            {
                                // 1. Remove the second element on the stack of open elements from its parent node, if it has one.
                                //
                                // 2. Pop all the nodes from the bottom of the stack of open elements, from the current node up to,
                                // but not including, the root html element.
                                //
                                // 3. Insert an HTML element for the token.
                                //
                                // 4. Switch the insertion mode to "in frameset".
                                HandleNyi(token, true); // $ NYI
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_address) ||
                            tag->HasNameOf(Html::globals->HTML_article) ||
                            tag->HasNameOf(Html::globals->HTML_aside) ||
                            tag->HasNameOf(Html::globals->HTML_blockquote) ||
                            tag->HasNameOf(Html::globals->HTML_center) ||
                            tag->HasNameOf(Html::globals->HTML_details) ||
                            tag->HasNameOf(Html::globals->HTML_dialog) ||
                            tag->HasNameOf(Html::globals->HTML_dir) ||
                            tag->HasNameOf(Html::globals->HTML_div) ||
                            tag->HasNameOf(Html::globals->HTML_dl) ||
                            tag->HasNameOf(Html::globals->HTML_fieldset) ||
                            tag->HasNameOf(Html::globals->HTML_figcaption) ||
                            tag->HasNameOf(Html::globals->HTML_figure) ||
                            tag->HasNameOf(Html::globals->HTML_footer) ||
                            tag->HasNameOf(Html::globals->HTML_header) ||
                            tag->HasNameOf(Html::globals->HTML_hgroup) ||
                            tag->HasNameOf(Html::globals->HTML_main) ||
                            tag->HasNameOf(Html::globals->HTML_menu) ||
                            tag->HasNameOf(Html::globals->HTML_nav) ||
                            tag->HasNameOf(Html::globals->HTML_ol) ||
                            tag->HasNameOf(Html::globals->HTML_p) ||
                            tag->HasNameOf(Html::globals->HTML_section) ||
                            tag->HasNameOf(Html::globals->HTML_summary) ||
                            tag->HasNameOf(Html::globals->HTML_ul))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_h1) ||
                            tag->HasNameOf(Html::globals->HTML_h2) ||
                            tag->HasNameOf(Html::globals->HTML_h3) ||
                            tag->HasNameOf(Html::globals->HTML_h4) ||
                            tag->HasNameOf(Html::globals->HTML_h5) ||
                            tag->HasNameOf(Html::globals->HTML_h6))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_h1) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h2) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h3) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h4) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h5) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h6))
                            {
                                ParseError(token);
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_pre) ||
                            tag->HasNameOf(Html::globals->HTML_listing))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            insert_an_HTML_element(tag, 0);

                            // If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to the
                            // next one. (Newlines at the start of pre blocks are ignore as an authoring convenience.)
                            this->ignore_line_feed = true;

                            this->frameset_ok = false;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_form))
                        {
                            if (this->form_element.item() != 0)
                            {
                                ParseError(token);
                                (*ignored) = true;
                            }
                            else
                            {
                                if (has_element_in_button_scope(Html::globals->HTML_p))
                                {
                                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                                }

                                ElementNode::Ref element;
                                insert_an_HTML_element(tag, &element);

                                this->form_element = element;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_li))
                        {
                            // 1. Set the frameset-ok flag to "not ok".
                            this->frameset_ok = false;

                            // 2. Initialize node to be the current node (the bottommost node of the stack).
                            for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
                            {
                                ElementNode::Ref node = (*it);

                                // 3. Loop: If node is an li element, then act as if an end tag with the tag name "li" had been seen, then jump to the last step.
                                if (node->has_element_name(Html::globals->HTML_li))
                                {
                                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_li, 0);
                                    break;
                                }

                                if (is_special(node->element_name) &&
                                    !(node->has_element_name(Html::globals->HTML_address) ||
                                    node->has_element_name(Html::globals->HTML_div) ||
                                    node->has_element_name(Html::globals->HTML_p)))
                                {
                                    // 4. If node is in the special category, but is not an address, div, or p element, then jump to the last step.
                                    break;
                                }

                                // 5. Otherwise, set node to the previous entry in the stack of open elements and return to the step labeled loop.                            }
                            }

                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_dd) ||
                            tag->HasNameOf(Html::globals->HTML_dt))
                        {
                            this->frameset_ok = false;

                            for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
                            {
                                ElementNode::Ref node = (*it);

                                if (node->has_element_name(Html::globals->HTML_dd))
                                {
                                    generate_implied_end_tags(Html::globals->HTML_dd);

                                    if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dd) == false)
                                        ParseError(tag);

                                    while (true)
                                    {
                                        ElementNode::Ref popped = this->CurrentNode();
                                        pop_the_current_node_off_the_stack_of_open_elements();

                                        if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dd))
                                            break;
                                    }

                                    break;
                                }

                                if (node->has_element_name(Html::globals->HTML_dt))
                                {
                                    generate_implied_end_tags(Html::globals->HTML_dt);

                                    if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dt) == false)
                                        ParseError(tag);

                                    while (true)
                                    {
                                        ElementNode::Ref popped = this->CurrentNode();
                                        pop_the_current_node_off_the_stack_of_open_elements();

                                        if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dt))
                                            break;
                                    }

                                    break;
                                }

                                // 4. If node is in the special category, but is not an address, div, or p element, then jump to the last step.
                                if (is_special(node->element_name) &&
                                    !node->element_name->equals(Html::globals->HTML_address) &&
                                    !node->element_name->equals(Html::globals->HTML_div) &&
                                    !node->element_name->equals(Html::globals->HTML_p))
                                {
                                    break;
                                }
                            }

                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                close_a_p_element();
                            }

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_plaintext))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            insert_an_HTML_element(tag, 0);

                            this->parser->tokenizer->SwitchToState(TokenizerState::PLAINTEXT_state);

                            // Note: Once a start tag with the tag name "plaintext" has been seen, that will be the last token ever
                            // seen other than character tokens (and the end-of-file token), because there is no way to switch out of
                            // the PLAINTEXT state.
                        }
                        else if (tag->HasNameOf(Html::globals->button_element_name))
                        {
                            if (has_element_in_scope(Html::globals->button_element_name))
                            {
                                ParseError(token);

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->button_element_name, 0);

                                (*reprocess = true);
                            }
                            else
                            {
                                reconstruct_the_active_formatting_elements();
                                insert_an_HTML_element(tag, 0);
                                this->frameset_ok = false;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_a))
                        {
                            // If the list of active formatting elements contains an element whose tag name is "a" between the end of 
                            // the list and the last marker on the list (or the start of the list if there is no marker on the list), 
                            // then this is a parse error; act as if an end tag with the tag name "a" had been seen, then remove that 
                            // element from the list of active formatting elements and the stack of open elements if the end tag didn't 
                            // already remove it (it might not have if the element is not in table scope).

                            int formatting_element_index = this->active_formatting_elements.size() - 1;
                            FormattingElement::Ref formatting_element;

                            while (true)
                            {
                                if (formatting_element_index == -1)
                                    break;

                                FormattingElement::Ref item = this->active_formatting_elements.at(formatting_element_index);

                                if (item->IsMarker())
                                    break;

                                if (item->element->has_element_name(Html::globals->HTML_a))
                                {
                                    formatting_element = item;
                                    break;
                                }

                                formatting_element_index--;
                            }

                            if (formatting_element.item() != 0)
                            {
                                ParseError(tag);

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_a, 0);

                                remove_node_from_the_list_of_active_formatting_elements(formatting_element->element);
                                remove_node_from_the_stack_of_open_elements(formatting_element->element);
                            }

                            reconstruct_the_active_formatting_elements();

                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);

                            push_onto_the_list_of_active_formatting_elements(element, tag);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_b) ||
                            tag->HasNameOf(Html::globals->HTML_big) ||
                            tag->HasNameOf(Html::globals->HTML_code) ||
                            tag->HasNameOf(Html::globals->HTML_em) ||
                            tag->HasNameOf(Html::globals->HTML_font) ||
                            tag->HasNameOf(Html::globals->HTML_i) ||
                            tag->HasNameOf(Html::globals->HTML_s) ||
                            tag->HasNameOf(Html::globals->HTML_small) ||
                            tag->HasNameOf(Html::globals->HTML_strike) ||
                            tag->HasNameOf(Html::globals->HTML_strong) ||
                            tag->HasNameOf(Html::globals->HTML_tt) ||
                            tag->HasNameOf(Html::globals->HTML_u))
                        {
                            reconstruct_the_active_formatting_elements();

                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);

                            push_onto_the_list_of_active_formatting_elements(element, tag);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_nobr))
                        {
                            reconstruct_the_active_formatting_elements();

                            if (has_element_in_scope(Html::globals->HTML_nobr))
                            {
                                ParseError(token);

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_nobr, 0);

                                reconstruct_the_active_formatting_elements();
                            }

                            ElementNode::Ref element;
                            insert_an_HTML_element(tag, &element);

                            push_onto_the_list_of_active_formatting_elements(element, tag);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_applet) ||
                            tag->HasNameOf(Html::globals->HTML_marquee) ||
                            tag->HasNameOf(Html::globals->object_element_name))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);

                            insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();

                            this->frameset_ok = false;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_table))
                        {
                            if (this->document->mode != Document::quirks_mode &&
                                has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            insert_an_HTML_element(tag, 0);

                            this->frameset_ok = false;

                            switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_area) ||
                            tag->HasNameOf(Html::globals->HTML_br) ||
                            tag->HasNameOf(Html::globals->HTML_embed) ||
                            tag->HasNameOf(Html::globals->HTML_img) ||
                            tag->HasNameOf(Html::globals->HTML_keygen) ||
                            tag->HasNameOf(Html::globals->HTML_wbr))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            this->frameset_ok = false;
                        }
                        else if (tag->HasNameOf(Html::globals->input_element_name))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            StringMap::iterator type_attribute_iterator = tag->attributes->find(Html::globals->type_attribute_name);
                            if (type_attribute_iterator == tag->attributes->end()
                                || type_attribute_iterator->second.equals<false>(Html::globals->hidden_type))
                            {
                                this->frameset_ok = false;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_menuitem) ||
                            tag->HasNameOf(Html::globals->HTML_param) ||
                            tag->HasNameOf(Html::globals->HTML_source) ||
                            tag->HasNameOf(Html::globals->HTML_track))
                        {
                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_hr))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            this->frameset_ok = false;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_image))
                        {
                            ParseError(token);

                            tag->name = Html::globals->HTML_img->name;

                            (*reprocess) = true;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_isindex))
                        {
                            ParseError(token);

                            if (this->form_element.item() != 0)
                            {
                                (*ignored) = true;
                            }
                            else
                            {
                                if (tag->self_closing)
                                {
                                    // Acknowledge the token's self-closing flag, if it is set.
                                    tag->acknowledged = true;
                                }

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_form);

                                // If the token has an attribute called "action", set the action attribute on the resulting form
                                // element to the value of the "action" attribute of the token.
                                HandleNyi(token, true); // $ NYI

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_hr);
                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_label);

                                // Act as if a stream of character tokens had been seen (see below for what they should say).
                                HandleNyi(token, true); // $ NYI

                                // Act as if a start tag token with the tag name "input" had been seen, with all the attributes 
                                // from the "isindex" token except "name", "action", and "prompt". Set the name attribute of the 
                                // resulting input element to the value "isindex".
                                HandleNyi(token, true); // $ NYI

                                // Act as if a stream of character tokens had been seen (see below for what they should say).
                                HandleNyi(token, true); // $ NYI

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_label, 0);

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_hr);

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_form, 0);

                                // If the token has an attribute with the name "prompt", then the first stream of characters must 
                                // be the same string as given in that attribute, and the second stream of characters must be empty. 
                                // Otherwise, the two streams of character tokens together should, together with the input element, 
                                // express the equivalent of "This is a searchable index. Enter search keywords: (input field)" in 
                                // the user's preferred language.
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_textarea))
                        {
                            insert_an_HTML_element(tag, 0);

                            // 2. If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to
                            // the next one. (Newlines at the start of textarea elements are ignored as an authoring convenience.)
                            this->ignore_line_feed = true;

                            this->parser->tokenizer->SwitchToState(TokenizerState::RCDATA_state);

                            this->original_insertion_mode = this->insertion_mode;

                            this->frameset_ok = false;

                            this->insertion_mode = InsertionMode::text_insertion_mode;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_xmp))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p, 0);
                            }

                            reconstruct_the_active_formatting_elements();

                            this->frameset_ok = false;

                            generic_raw_text_element_parsing_algorithm(tag);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_iframe))
                        {
                            this->frameset_ok = false;

                            generic_raw_text_element_parsing_algorithm(tag);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_noembed) ||
                            (tag->HasNameOf(Html::globals->HTML_noscript) && this->scripting_flag))
                        {
                            generic_raw_text_element_parsing_algorithm(tag);
                        }
                        else if (tag->HasNameOf(Html::globals->select_element_name))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);

                            this->frameset_ok = false;

                            if (this->insertion_mode == InsertionMode::in_table_insertion_mode ||
                                this->insertion_mode == InsertionMode::in_caption_insertion_mode ||
                                this->insertion_mode == InsertionMode::in_table_body_insertion_mode ||
                                this->insertion_mode == InsertionMode::in_row_insertion_mode ||
                                this->insertion_mode == InsertionMode::in_cell_insertion_mode)
                            {
                                switch_the_insertion_mode(InsertionMode::in_select_in_table_insertion_mode);
                            }
                            else
                            {
                                switch_the_insertion_mode(InsertionMode::in_select_insertion_mode);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_optgroup) ||
                            tag->HasNameOf(Html::globals->HTML_option))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option, 0);
                            }

                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_rp) ||
                            tag->HasNameOf(Html::globals->HTML_rt))
                        {
                            if (has_element_in_scope(Html::globals->HTML_ruby))
                            {
                                generate_implied_end_tags();

                                if (!this->CurrentNode()->has_element_name(Html::globals->HTML_ruby))
                                    ParseError(token);
                            }

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->MathML_math))
                        {
                            reconstruct_the_active_formatting_elements();

                            adjust_MathML_attributes(tag);

                            adjust_foreign_attributes(tag);

                            insert_a_foreign_element(tag, Html::globals->Namespace_MathML);

                            if (tag->self_closing)
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();

                                // and acknowledge the token's self-closing flag.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->SVG_svg))
                        {
                            reconstruct_the_active_formatting_elements();

                            adjust_SVG_attributes(tag);

                            adjust_foreign_attributes(tag);

                            insert_a_foreign_element(tag, Html::globals->Namespace_SVG);

                            if (tag->self_closing)
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();

                                // and acknowledge the token's self-closing flag.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_frame) ||
                            tag->HasNameOf(Html::globals->HTML_head) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                        else
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);

                            // Note: This element will be an ordinary element.
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        // If there is a node in the stack of open elements that is not either a dd element, a dt element, an 
                        // li element, a p element, a tbody element, a td element, a tfoot element, a th element, a thead element, 
                        // a tr element, the body element, or the html element, then this is a parse error.

                        for (ElementList::iterator it = this->open_elements.begin(); it != this->open_elements.end(); it++)
                        {
                            if ((*it)->has_element_name(Html::globals->HTML_dd) ||
                                (*it)->has_element_name(Html::globals->HTML_dt) ||
                                (*it)->has_element_name(Html::globals->HTML_li) ||
                                (*it)->has_element_name(Html::globals->HTML_p) ||
                                (*it)->has_element_name(Html::globals->HTML_tbody) ||
                                (*it)->has_element_name(Html::globals->HTML_td) ||
                                (*it)->has_element_name(Html::globals->HTML_tfoot) ||
                                (*it)->has_element_name(Html::globals->HTML_th) ||
                                (*it)->has_element_name(Html::globals->HTML_thead) ||
                                (*it)->has_element_name(Html::globals->HTML_tr) ||
                                (*it)->has_element_name(Html::globals->HTML_body) ||
                                (*it)->has_element_name(Html::globals->HTML_html))
                            {
                                continue;
                            }

                            ParseError(token);
                            break;
                        }

                        stop_parsing();
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_body))
                        {
                            if (!has_element_in_scope(Html::globals->HTML_body))
                            {
                                ParseError(token);
                            }
                            else
                            {
                                // Otherwise, if there is a node in the stack of open elements that is not either a dd element,
                                // a dt element, an li element, an optgroup element, an option element, a p element, an rp element,
                                // an rt element, a tbody element, a td element, a tfoot element, a th element, a thead element, a 
                                // tr element, the body element, or the html element, then this is a parse error.

                                for (ElementList::iterator it = this->open_elements.begin(); it != this->open_elements.end(); it++)
                                {
                                    if ((*it)->has_element_name(Html::globals->HTML_dd) ||
                                        (*it)->has_element_name(Html::globals->HTML_dt) ||
                                        (*it)->has_element_name(Html::globals->HTML_li) ||
                                        (*it)->has_element_name(Html::globals->HTML_optgroup) ||
                                        (*it)->has_element_name(Html::globals->HTML_option) ||
                                        (*it)->has_element_name(Html::globals->HTML_p) ||
                                        (*it)->has_element_name(Html::globals->HTML_rp) ||
                                        (*it)->has_element_name(Html::globals->HTML_rt) ||
                                        (*it)->has_element_name(Html::globals->HTML_tbody) ||
                                        (*it)->has_element_name(Html::globals->HTML_td) ||
                                        (*it)->has_element_name(Html::globals->HTML_tfoot) ||
                                        (*it)->has_element_name(Html::globals->HTML_th) ||
                                        (*it)->has_element_name(Html::globals->HTML_thead) ||
                                        (*it)->has_element_name(Html::globals->HTML_tr) ||
                                        (*it)->has_element_name(Html::globals->HTML_body) ||
                                        (*it)->has_element_name(Html::globals->HTML_html))
                                    {
                                        continue;
                                    }

                                    ParseError(token);
                                    break;
                                }
                            }

                            switch_the_insertion_mode(InsertionMode::after_body_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_body, &ignored);

                            if (!ignored)
                                (*reprocess) = true;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_address) ||
                            tag->HasNameOf(Html::globals->HTML_article) ||
                            tag->HasNameOf(Html::globals->HTML_aside) ||
                            tag->HasNameOf(Html::globals->HTML_blockquote) ||
                            tag->HasNameOf(Html::globals->button_element_name) ||
                            tag->HasNameOf(Html::globals->HTML_center) ||
                            tag->HasNameOf(Html::globals->HTML_details) ||
                            tag->HasNameOf(Html::globals->HTML_dialog) ||
                            tag->HasNameOf(Html::globals->HTML_dir) ||
                            tag->HasNameOf(Html::globals->HTML_div) ||
                            tag->HasNameOf(Html::globals->HTML_dl) ||
                            tag->HasNameOf(Html::globals->HTML_fieldset) ||
                            tag->HasNameOf(Html::globals->HTML_figcaption) ||
                            tag->HasNameOf(Html::globals->HTML_figure) ||
                            tag->HasNameOf(Html::globals->HTML_footer) ||
                            tag->HasNameOf(Html::globals->HTML_header) ||
                            tag->HasNameOf(Html::globals->HTML_hgroup) ||
                            tag->HasNameOf(Html::globals->HTML_listing) ||
                            tag->HasNameOf(Html::globals->HTML_main) ||
                            tag->HasNameOf(Html::globals->HTML_menu) ||
                            tag->HasNameOf(Html::globals->HTML_nav) ||
                            tag->HasNameOf(Html::globals->HTML_ol) ||
                            tag->HasNameOf(Html::globals->HTML_pre) ||
                            tag->HasNameOf(Html::globals->HTML_section) ||
                            tag->HasNameOf(Html::globals->HTML_summary) ||
                            tag->HasNameOf(Html::globals->HTML_ul))
                        {
                            if (!has_element_in_scope(tag))
                            {
                                ParseError(token);
                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(token);

                                while(true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->HasNameOf(element->element_name))
                                        break;
                                }
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_form))
                        {
                            ElementNode::Ref node = this->form_element;
                            this->form_element = 0;

                            if (node.item() == 0 || !has_element_in_scope(node))
                            {
                                ParseError(token);
                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (this->CurrentNode() != node)
                                    ParseError(token);

                                remove_node_from_the_stack_of_open_elements(node);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_p))
                        {
                            if (!has_element_in_button_scope(tag))
                            {
                                ParseError(token);

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_p);

                                (*reprocess) = true;
                            }
                            else
                            {
                                // 1. Generate implied end tags, except for elements with the same tag name as the token.
                                generate_implied_end_tags(tag);

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(token);

                                while(true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->HasNameOf(element->element_name))
                                        break;
                                }
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_li))
                        {
                            if (!has_element_in_list_item_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                // 1. Generate implied end tags, except for elements with the same tag name as the token.
                                generate_implied_end_tags(tag);

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(token);

                                while(true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->HasNameOf(element->element_name))
                                        break;
                                }
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_dd) ||
                            tag->HasNameOf(Html::globals->HTML_dt))
                        {
                            if (!has_element_in_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                // 1. Generate implied end tags, except for elements with the same tag name as the token.
                                generate_implied_end_tags(tag);

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(token);

                                while(true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->HasNameOf(element->element_name))
                                        break;
                                }
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_h1) ||
                            tag->HasNameOf(Html::globals->HTML_h2) ||
                            tag->HasNameOf(Html::globals->HTML_h3) ||
                            tag->HasNameOf(Html::globals->HTML_h4) ||
                            tag->HasNameOf(Html::globals->HTML_h5) ||
                            tag->HasNameOf(Html::globals->HTML_h6))
                        {
                            if (!(has_element_in_scope(Html::globals->HTML_h1) ||
                                has_element_in_scope(Html::globals->HTML_h2) ||
                                has_element_in_scope(Html::globals->HTML_h3) ||
                                has_element_in_scope(Html::globals->HTML_h4) ||
                                has_element_in_scope(Html::globals->HTML_h5) ||
                                has_element_in_scope(Html::globals->HTML_h6)))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(token);

                                while(true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->HTML_h1) ||
                                        element->has_element_name(Html::globals->HTML_h2) ||
                                        element->has_element_name(Html::globals->HTML_h3) ||
                                        element->has_element_name(Html::globals->HTML_h4) ||
                                        element->has_element_name(Html::globals->HTML_h5) ||
                                        element->has_element_name(Html::globals->HTML_h6))
                                        break;
                                }
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_a) ||
                            tag->HasNameOf(Html::globals->HTML_b) ||
                            tag->HasNameOf(Html::globals->HTML_big) ||
                            tag->HasNameOf(Html::globals->HTML_code) ||
                            tag->HasNameOf(Html::globals->HTML_em) ||
                            tag->HasNameOf(Html::globals->HTML_font) ||
                            tag->HasNameOf(Html::globals->HTML_i) ||
                            tag->HasNameOf(Html::globals->HTML_nobr) ||
                            tag->HasNameOf(Html::globals->HTML_s) ||
                            tag->HasNameOf(Html::globals->HTML_small) ||
                            tag->HasNameOf(Html::globals->HTML_strike) ||
                            tag->HasNameOf(Html::globals->HTML_strong) ||
                            tag->HasNameOf(Html::globals->HTML_tt) ||
                            tag->HasNameOf(Html::globals->HTML_u))
                        {
                            adoption_agency_algorithm(tag, ignored);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_applet) ||
                            tag->HasNameOf(Html::globals->HTML_marquee) ||
                            tag->HasNameOf(Html::globals->object_element_name))
                        {
                            if (!has_element_in_scope(tag))
                            {
                                ParseError(token);
                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(token);

                                while(true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->HasNameOf(element->element_name))
                                        break;
                                }

                                clear_the_list_of_active_formatting_elements_up_to_the_last_marker();
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_br))
                        {
                            ParseError(token);

                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_br);

                            (*ignored) = true;
                        }
                        else
                        {
                            any_other_end_tag_in_body(tag, ignored);
                        }
                    }
                    break;

                default:
                    HandleError("unexpected token");
                    break;
                }
            }
            break;

        case InsertionMode::text_insertion_mode:
            {
                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;
                        this->CurrentNode()->Insert(character_token->data);
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        ParseError(token);

                        if (this->CurrentNode()->has_element_name(Html::globals->HTML_script))
                        {
                            // mark the script element as "already started".
                            HandleNyi(token, true); // $ NYI
                        }

                        pop_the_current_node_off_the_stack_of_open_elements();

                        switch_the_insertion_mode(this->original_insertion_mode);

                        (*reprocess) = true;
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_script))
                        {
                            perform_a_microtask_checkpoint();

                            provide_a_stable_state();

                            ElementNode::Ref script = this->CurrentNode();

                            pop_the_current_node_off_the_stack_of_open_elements();

                            switch_the_insertion_mode(this->original_insertion_mode);

                            HandleNyi("</script>", false); // $ NYI
                        }
                        else
                        {
                            pop_the_current_node_off_the_stack_of_open_elements();

                            switch_the_insertion_mode(this->original_insertion_mode);
                        }
                    }
                    break;

                default:
                    HandleError("unexpected token");
                    break;
                }
            }
            break;

        case InsertionMode::in_table_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        if (this->CurrentNode()->has_element_name(Html::globals->HTML_table) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_tbody) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_tfoot) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_thead) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_tr))
                        {
                            this->pending_table_character_tokens.clear();

                            this->original_insertion_mode = this->insertion_mode;

                            switch_the_insertion_mode(InsertionMode::in_table_text_insertion_mode);

                            (*reprocess) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_caption))
                        {
                            clear_the_stack_back_to_a_table_context();

                            insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_caption_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_colgroup))
                        {
                            clear_the_stack_back_to_a_table_context();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_column_group_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_col))
                        {
                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_colgroup);

                            (*reprocess) = true;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead))
                        {
                            clear_the_stack_back_to_a_table_context();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_table_body_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_tbody);

                            (*reprocess) = true;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_table))
                        {
                            ParseError(token);

                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_table, &ignored);

                            if (!ignored)
                            {
                                // Note: The fake end tag token here can only be ignored in the fragment case.
                                (*reprocess) = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_style) ||
                            tag->HasNameOf(Html::globals->HTML_script))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, tag, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->input_element_name))
                        {
                            // If the token does not have an attribute with the name "type", or if it does, but that attribute's 
                            // value is not an ASCII case-insensitive match for the string "hidden", then: act as described in 
                            // the "anything else" entry below.
                            HandleNyi(token, true); // $ NYI

                            if (false) // $
                            {
                            }
                            else
                            {
                                ParseError(token);

                                insert_an_HTML_element(tag, 0);

                                pop_the_current_node_off_the_stack_of_open_elements();

                                if (tag->self_closing)
                                {
                                    // Acknowledge the token's self-closing flag, if it is set.
                                    tag->acknowledged = true;
                                }
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_form))
                        {
                            ParseError(token);

                            if (this->form_element.item() != 0)
                            {
                                (*ignored) = true;
                            }
                            else
                            {
                                insert_an_HTML_element(tag, &this->form_element);

                                pop_the_current_node_off_the_stack_of_open_elements();
                            }
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_table))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                // fragment case
                                ParseError(token);
                                (*ignored) = true;
                            }
                            else
                            {
                                while (true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->HTML_table))
                                        break;
                                }

                                reset_the_insertion_mode_appropriately();
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            ParseError(token);
                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        if (this->CurrentNode() != this->open_elements.front())
                            ParseError(token);

                        // Note: The current node can only be the root html element in the fragment case.

                        stop_parsing();
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    in_table_insertion_mode_anything_else(token);
                }
            }
            break;

        case InsertionMode::in_table_text_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0000:
                            ParseError(token);
                            (*ignored) = true;
                            break;

                        default:
                            this->pending_table_character_tokens.push_back(character_token);
                            break;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    bool reprocess_pending = false;

                    for (std::vector<CharacterToken::Ref>::iterator it = this->pending_table_character_tokens.begin(); it != this->pending_table_character_tokens.end(); it++)
                    {
                        if (!((*it)->data == 0x0020 ||
                            (*it)->data == 0x0009 ||
                            (*it)->data == 0x000A ||
                            (*it)->data == 0x000C ||
                            (*it)->data == 0x000D))
                        {
                            reprocess_pending = true;
                        }
                    }

                    if (reprocess_pending)
                    {
                        for (std::vector<CharacterToken::Ref>::iterator it = this->pending_table_character_tokens.begin(); it != this->pending_table_character_tokens.end(); it++)
                        {
                            in_table_insertion_mode_anything_else(*it);
                        }
                    }
                    else
                    {
                        for (std::vector<CharacterToken::Ref>::iterator it = this->pending_table_character_tokens.begin(); it != this->pending_table_character_tokens.end(); it++)
                        {
                            this->CurrentNode()->Insert((*it)->data);
                        }
                    }

                    switch_the_insertion_mode(this->original_insertion_mode);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::in_caption_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_caption))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!this->CurrentNode()->has_element_name(Html::globals->HTML_caption))
                                    ParseError(token);

                                while (true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->HTML_caption))
                                        break;
                                }

                                clear_the_list_of_active_formatting_elements_up_to_the_last_marker();

                                switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_table))
                        {
                            ParseError(token);

                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_caption, &ignored);

                            if (!ignored)
                                (*reprocess) = true;

                            // Note: The fake end tag token here can only be ignored in the fragment case.
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            ParseError(token);

                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            ParseError(token);

                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_caption, &ignored);

                            if (!ignored)
                                (*reprocess) = true;

                            // Note: The fake end tag token here can only be ignored in the fragment case.
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                }
            }
            break;

        case InsertionMode::in_column_group_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            this->CurrentNode()->Insert(character_token->data);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_col))
                        {
                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_colgroup))
                        {
                            if (this->CurrentNode() == this->open_elements.front().item())
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();

                                switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_col))
                        {
                            ParseError(token);

                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = false;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        if (this->CurrentNode() == this->open_elements.front().item())
                        {
                            stop_parsing();

                            // fragment case
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    bool ignored = false;

                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_colgroup, &ignored);

                    if (!ignored)
                        (*reprocess) = true;

                    // Note: The fake end tag token here can only be ignored in the fragment case.
                }
            }
            break;

        case InsertionMode::in_table_body_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            clear_the_stack_back_to_a_table_body_context();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_row_insertion_mode);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_td))
                        {
                            ParseError(token);

                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_tr);

                            (*reprocess) = true;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead))
                        {
                            if (!(has_element_in_table_scope(Html::globals->HTML_tbody) ||
                                has_element_in_table_scope(Html::globals->HTML_thead) ||
                                has_element_in_table_scope(Html::globals->HTML_tfoot)))
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_body_context();

                                act_as_if_an_end_tag_token_had_been_seen(this->CurrentNode()->element_name, 0);

                                (*reprocess) = true;
                            }
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_body_context();

                                pop_the_current_node_off_the_stack_of_open_elements();

                                switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_table))
                        {
                            if (!(has_element_in_table_scope(Html::globals->HTML_tbody) ||
                                has_element_in_table_scope(Html::globals->HTML_thead) ||
                                has_element_in_table_scope(Html::globals->HTML_tfoot)))
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_body_context();

                                act_as_if_an_end_tag_token_had_been_seen(this->CurrentNode()->element_name, 0);

                                (*reprocess) = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            ParseError(token);

                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    apply_the_rules_for(InsertionMode::in_table_insertion_mode, token, 0, reprocess);
                }
            }
            break;

        case InsertionMode::in_row_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_td))
                        {
                            clear_the_stack_back_to_a_table_row_context();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_cell_insertion_mode);

                            insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_tr, &ignored);

                            if (!ignored)
                                (*reprocess) = true;

                            // Note: The fake end tag token here can only be ignored in the fragment case.
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_row_context();

                                pop_the_current_node_off_the_stack_of_open_elements();

                                switch_the_insertion_mode(InsertionMode::in_table_body_insertion_mode);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_table))
                        {
                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_tr, &ignored);

                            if (!ignored)
                                (*reprocess) = true;

                            // Note: The fake end tag token here can only be ignored in the fragment case.
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_tr, 0);

                                (*reprocess) = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_html) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_th))
                        {
                            ParseError(token);

                            (*ignored) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    apply_the_rules_for(InsertionMode::in_table_insertion_mode, token, 0, reprocess);
                }
            }
            break;

        case InsertionMode::in_cell_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            if (!(has_element_in_table_scope(Html::globals->HTML_td) ||
                                has_element_in_table_scope(Html::globals->HTML_th)))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                close_the_cell();

                                (*reprocess) = true;
                            }
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_td) ||
                            tag->HasNameOf(Html::globals->HTML_th))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->HasNameOf(this->CurrentNode()->element_name))
                                    ParseError(tag);

                                while (true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->HasNameOf(element->element_name))
                                        break;
                                }

                                clear_the_list_of_active_formatting_elements_up_to_the_last_marker();

                                switch_the_insertion_mode(InsertionMode::in_row_insertion_mode);
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_body) ||
                            tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_col) ||
                            tag->HasNameOf(Html::globals->HTML_colgroup) ||
                            tag->HasNameOf(Html::globals->HTML_html))
                        {
                            ParseError(token);

                            (*ignored) = true;
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_table) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;
                            }
                            else
                            {
                                close_the_cell();

                                (*reprocess) = true;
                            }
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                }
            }
            break;

        case InsertionMode::in_select_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0000:
                            ParseError(token);
                            (*ignored) = true;
                            break;

                        default:
                            this->CurrentNode()->Insert(character_token->data);
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_option))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option))
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option, 0);

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_optgroup))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option))
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option, 0);

                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup))
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_optgroup, 0);

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->select_element_name))
                        {
                            ParseError(token);

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->input_element_name) ||
                            tag->HasNameOf(Html::globals->HTML_keygen) ||
                            tag->HasNameOf(Html::globals->HTML_textarea))
                        {
                            ParseError(token);

                            if (!has_element_in_select_scope(Html::globals->select_element_name))
                            {
                                (*ignored) = true;
                                // fragment case
                            }
                            else
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name, 0);
                                (*reprocess) = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_script))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_optgroup))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option) &&
                                this->open_elements[this->open_elements.size() - 2]->has_element_name(Html::globals->HTML_optgroup))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option, 0);
                            }

                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup))
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }
                            else
                            {
                                ParseError(token);
                                (*ignored) = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_option))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option))
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }
                            else
                            {
                                ParseError(token);
                                (*ignored) = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->select_element_name))
                        {
                            if (!has_element_in_select_scope(tag))
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case;
                            }
                            else
                            {
                                while (true)
                                {
                                    ElementNode::Ref element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->select_element_name))
                                        break;
                                }

                                reset_the_insertion_mode_appropriately();
                            }
                        }
                        else
                        {
                            anything_else = false;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        if (this->CurrentNode() != this->open_elements.front())
                            ParseError(token);

                        // Note: The current node can only be the root html element in the fragment case.

                        stop_parsing();
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ParseError(token);
                    (*ignored) = true;
                }
            }
            break;

        case InsertionMode::in_select_in_table_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_table) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_td))
                        {
                            ParseError(token);

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name, 0);

                            (*reprocess) = true;
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_caption) ||
                            tag->HasNameOf(Html::globals->HTML_table) ||
                            tag->HasNameOf(Html::globals->HTML_tbody) ||
                            tag->HasNameOf(Html::globals->HTML_tfoot) ||
                            tag->HasNameOf(Html::globals->HTML_thead) ||
                            tag->HasNameOf(Html::globals->HTML_tr) ||
                            tag->HasNameOf(Html::globals->HTML_th) ||
                            tag->HasNameOf(Html::globals->HTML_td))
                        {
                            ParseError(token);

                            if (has_element_in_table_scope(tag))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name, 0);
                                (*reprocess) = true;
                            }
                            else
                            {
                                (*ignored) = true;
                            }
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    apply_the_rules_for(InsertionMode::in_select_insertion_mode, token, 0, reprocess);
                }
            }
            break;

        case InsertionMode::after_body_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            if (this->fragment_context.item() != 0)
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                switch_the_insertion_mode(InsertionMode::after_after_body_insertion_mode);
                            }
                        }
                        else
                        {
                            anything_else = false;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    stop_parsing();
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ParseError(token);

                    switch_the_insertion_mode(InsertionMode::in_body_insertion_mode);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::in_frameset_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            this->CurrentNode()->Insert(character_token->data);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_frameset))
                        {
                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_frame))
                        {
                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_noframes))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_frameset))
                        {
                            if (this->CurrentNode() == this->open_elements.front().item())
                            {
                                ParseError(token);

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }

                            if (this->fragment_context.item() == 0 &&
                                !this->CurrentNode()->has_element_name(Html::globals->HTML_frameset))
                            {
                                switch_the_insertion_mode(InsertionMode::after_frameset_insertion_mode);
                            }
                        }
                        else
                        {
                            anything_else = false;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        if (this->CurrentNode() != this->open_elements.front())
                            ParseError(token);

                        // Note: The current node can only be the root html element in the fragment case.

                        stop_parsing();
                    }
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ParseError(token);

                    (*ignored) = true;
                }
            }
            break;

        case InsertionMode::after_frameset_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            this->CurrentNode()->Insert(character_token->data);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token);
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_noframes))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            switch_the_insertion_mode(InsertionMode::after_after_frameset_insertion_mode);
                        }
                        else
                        {
                            anything_else = false;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    stop_parsing();
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ParseError(token);

                    (*ignored) = true;
                }
            }
            break;

        case InsertionMode::after_after_body_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    stop_parsing();
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ParseError(token);

                    switch_the_insertion_mode(InsertionMode::in_body_insertion_mode);

                    (*reprocess) = true;
                }
            }
            break;

        case InsertionMode::after_after_frameset_insertion_mode:
            {
                bool anything_else = false;

                switch (token->type)
                {
                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token;

                        CommentNode::Ref comment_node = New<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token;

                        switch (character_token->data)
                        {
                        case 0x0009:
                        case 0x000A:
                        case 0x000C:
                        case 0x000D:
                        case 0x0020:
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                            break;

                        default:
                            anything_else = true;
                            break;
                        }
                    }
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token;

                        if (tag->HasNameOf(Html::globals->HTML_html))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->HasNameOf(Html::globals->HTML_noframes))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else
                        {
                            anything_else = true;
                        }
                    }
                    break;

                case Token::Type::end_of_file_token:
                    stop_parsing();
                    break;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    ParseError(token);

                    (*ignored) = true;
                }
            }
            break;

        default:
            HandleError("unexpected insertion mode");
            break;
        }
    }

    void TreeConstruction::apply_the_rules_for_parsing_tokens_in_foreign_content(TokenPointer token, bool* ignored, bool* reprocess)
    {
        switch (token->type)
        {
        case Token::Type::character_token:
            {
                CharacterToken* character_token = (CharacterToken*)token;

                switch (character_token->data)
                {
                case 0x0000:
                    ParseError(token);
                    this->CurrentNode()->Insert(0xFFFD);
                    break;

                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x000D:
                case 0x0020:
                    this->CurrentNode()->Insert(character_token->data);
                    break;

                default:
                    this->CurrentNode()->Insert(character_token->data);
                    this->frameset_ok = false;
                    break;
                }
            }
            break;

        case Token::Type::comment_token:
            {
                CommentToken* comment_token = (CommentToken*)token;

                CommentNode::Ref comment_node = New<CommentNode>();
                comment_node->data = comment_token->data;

                this->CurrentNode()->Append(comment_node);
            }
            break;

        case Token::Type::DOCTYPE_token:
            ParseError(token);
            (*ignored) = true;
            break;

        case Token::Type::start_tag_token:
            {
                StartTagToken* tag = (StartTagToken*)token;

                //if (tag->HasNameOf())
                //{
                //}
                //else if (tag->HasNameOf())
                //{
                //}
                //else
                //{
                //}

                HandleNyi(token, true); // $ NYI
            }
            break;

        case Token::Type::end_tag_token:
            {
                EndTagToken* tag = (EndTagToken*)token;

                //if (tag->HasNameOf())
                //{
                //}
                //else
                //{
                //}

                HandleNyi(token, true); // $ NYI
            }
            break;
        }
    }
}