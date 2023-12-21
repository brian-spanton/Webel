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
#include "Basic.Globals.h"

namespace Html
{
    using namespace Basic;

    TreeConstruction::TreeConstruction(Parser* parser, std::shared_ptr<Uri> url) :
        parser(parser),
        script_nesting_level(0),
        parser_pause_flag(false),
        insertion_mode(InsertionMode::initial_insertion_mode),
        original_insertion_mode(InsertionMode::initial_insertion_mode),
        scripting_flag(false),
        frameset_ok(true),
        document(std::make_shared<Document>(url)),
        ignore_line_feed(false)
    {
    }

    std::shared_ptr<ElementNode> TreeConstruction::CurrentNode()
    {
        if (this->open_elements.size() == 0)
            return 0;

        return this->open_elements.back();
    }

    std::shared_ptr<ElementNode> TreeConstruction::AdjustedCurrentNode()
    {
        if (this->fragment_context.get() != 0 && this->open_elements.size() == 1)
            return this->fragment_context;

        return CurrentNode();
    }

    bool TreeConstruction::InForeignContent(const Token* token)
    {
        if (this->AdjustedCurrentNode() == 0)
            return false;

        if (this->AdjustedCurrentNode()->is_in_namespace(Html::globals->Namespace_HTML))
            return false;

        if (this->AdjustedCurrentNode()->IsMathMLTextIntegrationPoint() && token->type == Token::Type::start_tag_token)
        {
            StartTagToken* tag = (StartTagToken*)token;
            if (tag->has_name_of(Html::globals->MathML_mglyph.get()) && tag->has_name_of(Html::globals->MathML_malignmark.get()))
                return false;
        }

        if (this->AdjustedCurrentNode()->IsMathMLTextIntegrationPoint() && token->type == Token::Type::character_token)
            return false;

        if (this->AdjustedCurrentNode()->has_element_name(Html::globals->MathML_annotation_xml.get()) && token->type == Token::Type::start_tag_token)
        {
            StartTagToken* tag = (StartTagToken*)token;
            if (tag->has_name_of(Html::globals->SVG_svg.get()))
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
            if (it->get() == node)
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
            if ((*it)->element.get() == node)
            {
                this->active_formatting_elements.erase(it);
                return;
            }
        }

        HandleError("TreeConstruction::remove_node_from_the_list_of_active_formatting_elements");
    }

    void TreeConstruction::create_an_element_for_a_token(TagToken* token, UnicodeStringRef name_space, std::shared_ptr<ElementNode>* element)
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

        std::shared_ptr<ElementName> name = std::make_shared<ElementName>();
        name->Initialize(name_space, token->name);

        std::shared_ptr<ElementNode> local_element = std::make_shared<ElementNode>();

        std::shared_ptr<StringMap> attributes = std::make_shared<StringMap>();
        attributes->insert(token->attributes.begin(), token->attributes.end());
        local_element->Initialize(name, attributes);

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

    void TreeConstruction::insert_an_HTML_element(ElementNode* place, TagToken* token, std::shared_ptr<ElementNode>* element)
    {
        // $ this algorithm is outdated.  see current section 12.2.5.1

        std::shared_ptr<ElementNode> local_element;
        create_an_element_for_a_token(token, Html::globals->Namespace_HTML, &local_element);

        if (local_element->IsFormAssociated()
            && this->form_element.get() != 0
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

    void TreeConstruction::insert_an_HTML_element(TagToken* token, std::shared_ptr<ElementNode>* element)
    {
        insert_an_HTML_element(this->CurrentNode().get(), token, element);
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
        while ((this->CurrentNode()->has_element_name(Html::globals->HTML_dd.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_dt.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_li.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_p.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rp.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rt.get())) &&
            (except_for == 0 || !except_for->has_name_of(this->CurrentNode()->element_name.get())))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }
    }

    void TreeConstruction::generate_implied_end_tags(ElementName* except_for)
    {
        while ((this->CurrentNode()->has_element_name(Html::globals->HTML_dd.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_dt.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_li.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_p.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rp.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_rt.get())) &&
            (except_for == 0 || !except_for->equals(this->CurrentNode()->element_name.get())))
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
        std::shared_ptr<FormattingElement> entry;

        // 1. If there are no entries in the list of active formatting elements, then there is nothing to reconstruct; stop this algorithm.
        if (this->active_formatting_elements.size() == 0)
            return;

        // 2. If the last (most recently added) entry in the list of active formatting elements is a marker, or if it is an element that is
        // in the stack of open elements, then there is nothing to reconstruct; stop this algorithm.
        if (this->active_formatting_elements.back()->IsMarker() ||
            is_in_the_stack_of_open_elements(this->active_formatting_elements.back()->element.get()))
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
        if (!entry->IsMarker() && !is_in_the_stack_of_open_elements(entry->element.get()))
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
        std::shared_ptr<ElementNode> new_element;
        create_an_element_for_a_token(entry->token.get(), entry->element->element_name->name_space, &new_element);

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
        while (!(this->CurrentNode()->has_element_name(Html::globals->HTML_table.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_html.get())))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }

        // Note: The current node being an html element after this process is a fragment case.
    }

    void TreeConstruction::clear_the_stack_back_to_a_table_body_context()
    {
        while (!(this->CurrentNode()->has_element_name(Html::globals->HTML_tbody.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_tfoot.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_thead.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_html.get())))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }

        // Note: The current node being an html element after this process is a fragment case.
    }

    void TreeConstruction::clear_the_stack_back_to_a_table_row_context()
    {
        while (!(this->CurrentNode()->has_element_name(Html::globals->HTML_tr.get()) ||
            this->CurrentNode()->has_element_name(Html::globals->HTML_html.get())))
        {
            pop_the_current_node_off_the_stack_of_open_elements();
        }

        // Note: The current node being an html element after this process is a fragment case.
    }

    void TreeConstruction::close_the_cell()
    {
        if (has_element_in_table_scope(Html::globals->HTML_td.get()))
        {
            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_td.get(), 0);
        }
        else if (has_element_in_table_scope(Html::globals->HTML_th.get()))
        {
            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_th.get(), 0);
        }

        // Note: The stack of open elements cannot have both a td and a th element in table scope at the same time, nor can
        // it have neither when the close the cell algorithm is invoked.
    }

    bool TreeConstruction::is_in_the_stack_of_open_elements(ElementNode* element)
    {
        for (ElementList::iterator it = this->open_elements.begin(); it != this->open_elements.end(); it++)
        {
            if (it->get() == element)
                return true;
        }

        return false;
    }

    bool TreeConstruction::is_in_the_list_of_active_formatting_elements(ElementNode* element)
    {
        for (FormattingElementList::iterator it = this->active_formatting_elements.begin(); it != this->active_formatting_elements.end(); it++)
        {
            if ((*it)->element.get() == element)
                return true;
        }

        return false;
    }

    void TreeConstruction::insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements()
    {
        std::shared_ptr<FormattingElement> marker = std::make_shared<FormattingElement>();

        this->active_formatting_elements.push_back(marker);
    }

    void TreeConstruction::push_onto_the_list_of_active_formatting_elements(std::shared_ptr<ElementNode> element, std::shared_ptr<TagToken> token)
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
                if ((*it)->equals(element.get(), token.get()))
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
        std::shared_ptr<FormattingElement> formatting_element = std::make_shared<FormattingElement>();
        formatting_element->Initialize(element, token);

        this->active_formatting_elements.push_back(formatting_element);
    }

    void TreeConstruction::clear_the_list_of_active_formatting_elements_up_to_the_last_marker()
    {
        while (true)
        {
            std::shared_ptr<FormattingElement> entry = this->active_formatting_elements.back();
            this->active_formatting_elements.pop_back();

            if (entry->IsMarker())
                return;
        }
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
            // $$ bad form convering Unicode chars to ascii chars
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
            std::shared_ptr<ElementNode>& node = (*it);

            // fragment case
            if (it == this->open_elements.rend() - 1)
            {
                last = true;
                node = this->fragment_context;
            }

            // fragment case
            if (node->has_element_name(Html::globals->select_element_name.get()))
            {
                this->insertion_mode = InsertionMode::in_select_insertion_mode;
                return;
            }

            if ((node->has_element_name(Html::globals->HTML_td.get()) || node->has_element_name(Html::globals->HTML_th.get())) && last == false)
            {
                this->insertion_mode = InsertionMode::in_cell_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_tr.get()))
            {
                this->insertion_mode = InsertionMode::in_row_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_tbody.get()) || node->has_element_name(Html::globals->HTML_thead.get()) || node->has_element_name(Html::globals->HTML_tfoot.get()))
            {
                this->insertion_mode = InsertionMode::in_table_body_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_caption.get()))
            {
                this->insertion_mode = InsertionMode::in_caption_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_colgroup.get()))
            {
                this->insertion_mode = InsertionMode::in_column_group_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_table.get()))
            {
                this->insertion_mode = InsertionMode::in_table_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_head.get()))
            {
                // Intentional: ("in body"!  not "in head"!)
                this->insertion_mode = InsertionMode::in_body_insertion_mode;
                return;
            }

            if (node->has_element_name(Html::globals->HTML_body.get()))
            {
                this->insertion_mode = InsertionMode::in_body_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_frameset.get()))
            {
                this->insertion_mode = InsertionMode::in_frameset_insertion_mode;
                return;
            }

            // fragment case
            if (node->has_element_name(Html::globals->HTML_html.get()))
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

    void TreeConstruction::in_table_insertion_mode_anything_else(const Token* token)
    {
        ParseError(token);

        // handle_event the token using the rules for the "in body" insertion mode, except that whenever a node would be 
        // inserted into the current node when the current node is a table, tbody, tfoot, thead, or tr element, then it 
        // must instead be foster parented.
        //apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0);
        HandleNyi("TreeConstruction::in_table_insertion_mode_anything_else", true); // $ NYI
    }

    bool TreeConstruction::has_element_in_specific_scope(ElementName* target, const ElementNameList& list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if ((*it)->has_element_name(target))
                return true;

            for (ElementNameList::const_iterator it2 = list.cbegin(); it2 != list.cend(); it2++)
            {
                if ((*it)->has_element_name(it2->get()))
                    return false;
            }
        }

        HandleError("TreeConstruction::has_element_in_specific_scope missing html root node");
        return false;
    }

    bool TreeConstruction::has_element_in_specific_scope(TagToken* target, const ElementNameList& list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if (target->has_name_of((*it)->element_name.get()))
                return true;

            for (ElementNameList::const_iterator it2 = list.cbegin(); it2 != list.cend(); it2++)
            {
                if ((*it)->has_element_name(it2->get()))
                    return false;
            }
        }

        HandleError("TreeConstruction::has_element_in_specific_scope missing html root node");
        return false;
    }

    bool TreeConstruction::has_element_in_specific_anti_scope(ElementName* target, const ElementNameList& list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if ((*it)->has_element_name(target))
                return true;

            bool match = true;

            for (ElementNameList::const_iterator it2 = list.cbegin(); it2 != list.cend(); it2++)
            {
                if ((*it)->has_element_name(it2->get()))
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

    bool TreeConstruction::has_element_in_specific_anti_scope(TagToken* target, const ElementNameList& list)
    {
        for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
        {
            if (target->has_name_of((*it)->element_name.get()))
                return true;

            bool match = true;

            for (ElementNameList::const_iterator it2 = list.cbegin(); it2 != list.cend(); it2++)
            {
                if ((*it)->has_element_name(it2->get()))
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
        return has_element_in_specific_scope(target->element_name.get(), Html::globals->Scope);
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
        if (name->equals(Html::globals->HTML_address.get()) ||
            name->equals(Html::globals->HTML_applet.get()) ||
            name->equals(Html::globals->HTML_area.get()) || 
            name->equals(Html::globals->HTML_article.get()) || 
            name->equals(Html::globals->HTML_aside.get()) || 
            name->equals(Html::globals->HTML_base.get()) ||
            name->equals(Html::globals->HTML_basefont.get()) || 
            name->equals(Html::globals->HTML_bgsound.get()) || 
            name->equals(Html::globals->HTML_blockquote.get()) || 
            name->equals(Html::globals->HTML_body.get()) || 
            name->equals(Html::globals->HTML_br.get()) || 
            name->equals(Html::globals->button_element_name.get()) || 
            name->equals(Html::globals->HTML_caption.get()) || 
            name->equals(Html::globals->HTML_center.get()) || 
            name->equals(Html::globals->HTML_col.get()) || 
            name->equals(Html::globals->HTML_colgroup.get()) || 
            name->equals(Html::globals->HTML_dd.get()) || 
            name->equals(Html::globals->HTML_details.get()) || 
            name->equals(Html::globals->HTML_dir.get()) || 
            name->equals(Html::globals->HTML_div.get()) || 
            name->equals(Html::globals->HTML_dl.get()) || 
            name->equals(Html::globals->HTML_dt.get()) || 
            name->equals(Html::globals->HTML_embed.get()) || 
            name->equals(Html::globals->HTML_fieldset.get()) || 
            name->equals(Html::globals->HTML_figcaption.get()) || 
            name->equals(Html::globals->HTML_figure.get()) || 
            name->equals(Html::globals->HTML_footer.get()) || 
            name->equals(Html::globals->HTML_form.get()) || 
            name->equals(Html::globals->HTML_frame.get()) || 
            name->equals(Html::globals->HTML_frameset.get()) || 
            name->equals(Html::globals->HTML_h1.get()) || 
            name->equals(Html::globals->HTML_h2.get()) || 
            name->equals(Html::globals->HTML_h3.get()) || 
            name->equals(Html::globals->HTML_h4.get()) || 
            name->equals(Html::globals->HTML_h5.get()) || 
            name->equals(Html::globals->HTML_h6.get()) || 
            name->equals(Html::globals->HTML_head.get()) || 
            name->equals(Html::globals->HTML_header.get()) || 
            name->equals(Html::globals->HTML_hgroup.get()) || 
            name->equals(Html::globals->HTML_hr.get()) || 
            name->equals(Html::globals->HTML_html.get()) || 
            name->equals(Html::globals->HTML_iframe.get()) ||  
            name->equals(Html::globals->HTML_img.get()) || 
            name->equals(Html::globals->input_element_name.get()) || 
            name->equals(Html::globals->HTML_isindex.get()) || 
            name->equals(Html::globals->HTML_li.get()) || 
            name->equals(Html::globals->HTML_link.get()) || 
            name->equals(Html::globals->HTML_listing.get()) || 
            name->equals(Html::globals->HTML_main.get()) || 
            name->equals(Html::globals->HTML_marquee.get()) || 
            name->equals(Html::globals->HTML_menu.get()) || 
            name->equals(Html::globals->HTML_menuitem.get()) || 
            name->equals(Html::globals->HTML_meta.get()) || 
            name->equals(Html::globals->HTML_nav.get()) || 
            name->equals(Html::globals->HTML_noembed.get()) || 
            name->equals(Html::globals->HTML_noframes.get()) || 
            name->equals(Html::globals->HTML_noscript.get()) || 
            name->equals(Html::globals->object_element_name.get()) || 
            name->equals(Html::globals->HTML_ol.get()) || 
            name->equals(Html::globals->HTML_p.get()) || 
            name->equals(Html::globals->HTML_param.get()) || 
            name->equals(Html::globals->HTML_plaintext.get()) || 
            name->equals(Html::globals->HTML_pre.get()) || 
            name->equals(Html::globals->HTML_script.get()) || 
            name->equals(Html::globals->HTML_section.get()) || 
            name->equals(Html::globals->select_element_name.get()) || 
            name->equals(Html::globals->HTML_source.get()) || 
            name->equals(Html::globals->HTML_style.get()) || 
            name->equals(Html::globals->HTML_summary.get()) || 
            name->equals(Html::globals->HTML_table.get()) || 
            name->equals(Html::globals->HTML_tbody.get()) || 
            name->equals(Html::globals->HTML_td.get()) || 
            name->equals(Html::globals->HTML_textarea.get()) || 
            name->equals(Html::globals->HTML_tfoot.get()) || 
            name->equals(Html::globals->HTML_th.get()) || 
            name->equals(Html::globals->HTML_thead.get()) || 
            name->equals(Html::globals->HTML_title.get()) || 
            name->equals(Html::globals->HTML_tr.get()) || 
            name->equals(Html::globals->HTML_track.get()) || 
            name->equals(Html::globals->HTML_ul.get()) || 
            name->equals(Html::globals->HTML_wbr.get()) || 
            name->equals(Html::globals->HTML_xmp.get()) ||
            name->equals(Html::globals->MathML_mi.get()) ||
            name->equals(Html::globals->MathML_mo.get()) || 
            name->equals(Html::globals->MathML_mn.get()) || 
            name->equals(Html::globals->MathML_ms.get()) || 
            name->equals(Html::globals->MathML_mtext.get()) ||
            name->equals(Html::globals->MathML_annotation_xml.get()) ||
            name->equals(Html::globals->SVG_foreignObject.get()) ||
            name->equals(Html::globals->SVG_desc.get()) || 
            name->equals(Html::globals->SVG_title.get()))
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
        std::shared_ptr<FormattingElement> formatting_element;
        for (uint32 counter = 0; counter < this->active_formatting_elements.size(); counter++)
        {
            int index = this->active_formatting_elements.size() - counter - 1;
            std::shared_ptr<FormattingElement> last_element = this->active_formatting_elements.at(index);

            if (last_element->IsMarker())
                break;

            if (tag->has_name_of(last_element->element->element_name.get()))
            {
                formatting_element = last_element;
                break;
            }
        }

        // If there is no such node, then abort these steps and instead act as described in the "any other end tag" entry below.
        if (formatting_element.get() == 0)
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
            remove_node_from_the_list_of_active_formatting_elements(formatting_element->element.get());
            return;
        }

        // Otherwise, if there is such a node, and that node is also in the stack of open elements, but the element is not in scope,
        // then this is a parse error; ignore the token, and abort these steps.
        if (!has_element_in_scope(formatting_element->element.get()))
        {
            ParseError(tag);
            (*ignored) = true;
            return;
        }

        // Otherwise, there is a formatting element and that element is in the stack and is in scope. If the element is not the current node, 
        // this is a parse error. In any case, proceed with the algorithm as written in the following steps.
        if (formatting_element->element != this->CurrentNode())
            ParseError(tag);

        // 5. Let the furthest block be the topmost node in the stack of open elements that is lower in the stack than the formatting element, 
        // and is an element in the special category. There might not be one.
        std::shared_ptr<ElementNode> furthest_block;
        std::shared_ptr<ElementNode> immediately_above;
        for (int counter = 0; counter != this->open_elements.size(); counter++)
        {
            int index = this->open_elements.size() - counter - 1;
            if (this->open_elements.at(index) == formatting_element->element)
                break;

            if (is_special(this->open_elements.at(index)->element_name.get()))
            {
                furthest_block = this->open_elements.at(index);
                immediately_above = this->open_elements.at(index - 1);
            }
        }

        // 6. If there is no furthest block, then the UA must first pop all the nodes from the bottom of the stack of open elements,
        // from the current node up to and including the formatting element, then remove the formatting element from the list of active
        // formatting elements, and finally abort these steps.
        if (furthest_block.get() == 0)
        {
            while (true)
            {
                std::shared_ptr<ElementNode> node = std::static_pointer_cast<ElementNode>(this->CurrentNode());
                pop_the_current_node_off_the_stack_of_open_elements();

                if (node == formatting_element->element)
                    break;
            }

            remove_node_from_the_list_of_active_formatting_elements(formatting_element->element.get());

            return;
        }

        // 7. Let the common ancestor be the element immediately above the formatting element in the stack of open elements.
        std::shared_ptr<ElementNode> common_ancestor = this->open_elements.at(stack_index - 1);

        // 8. Let a bookmark note the position of the formatting element in the list of active formatting elements relative to the elements
        // on either side of it in the list.
        std::shared_ptr<FormattingElement> bookmark_after;
        for (uint32 index = 0; index < this->active_formatting_elements.size(); index++)
        {
            if (this->active_formatting_elements.at(index) == formatting_element)
            {
                bookmark_after = active_formatting_elements.at(index - 1);
                break;
            }
        }

        // 9. Let node and last node be the furthest block. Follow these steps:
        std::shared_ptr<ElementNode> node = furthest_block;
        std::shared_ptr<ElementNode> last_node = furthest_block;

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
        if (!is_in_the_list_of_active_formatting_elements(node.get()))
        {
            remove_node_from_the_stack_of_open_elements(node.get());
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
            std::shared_ptr<FormattingElement> list_entry = this->active_formatting_elements.at(index);

            if (list_entry->element == node)
            {
                std::shared_ptr<ElementNode> new_element;
                create_an_element_for_a_token(list_entry->token.get(), node->element_name->name_space, &new_element);

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
        if (common_ancestor->has_element_name(Html::globals->HTML_table.get()) ||
            common_ancestor->has_element_name(Html::globals->HTML_tbody.get()) ||
            common_ancestor->has_element_name(Html::globals->HTML_tfoot.get()) ||
            common_ancestor->has_element_name(Html::globals->HTML_thead.get()) ||
            common_ancestor->has_element_name(Html::globals->HTML_tr.get()))
        {
            last_node->remove_from_parent();
            foster_parent(last_node.get());
        }
        else
        {
            // Otherwise, append whatever last node ended up being in the previous step to the common ancestor node, first removing it from its
            // previous parent node if any.
            last_node->remove_from_parent();
            common_ancestor->Append(last_node);
        }

        // 11. Create an element for the token for which the formatting element was created.
        std::shared_ptr<ElementNode> new_element;
        create_an_element_for_a_token(formatting_element->token.get(), formatting_element->element->element_name->name_space, &new_element);

        // 12. Take all of the child nodes of the furthest block and append them to the element created in the last step.
        new_element->take_all_child_nodes_of(furthest_block.get());

        // 13. Append that new element to the furthest block.
        furthest_block->Append(new_element);

        // 14. Remove the formatting element from the list of active formatting elements, and insert the new element into the list of 
        // active formatting elements at the position of the aforementioned bookmark.
        remove_node_from_the_list_of_active_formatting_elements(formatting_element->element.get());

        std::shared_ptr<FormattingElement> new_formatting_element = std::make_shared<FormattingElement>();
        new_formatting_element->Initialize(new_element, formatting_element->token);

        for (uint32 index = 0; index < this->active_formatting_elements.size(); index++)
        {
            if (this->active_formatting_elements.at(index) == bookmark_after)
                this->active_formatting_elements.insert(this->active_formatting_elements.begin() + index + 1, new_formatting_element);
        }

        // 15. Remove the formatting element from the stack of open elements, and insert the new element into the stack of open elements 
        // immediately below the position of the furthest block in that stack.
        remove_node_from_the_stack_of_open_elements(formatting_element->element.get());

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
        std::shared_ptr<ElementNode> node = this->open_elements.at(node_index);
        if (tag->has_name_of(node->element_name.get()))
        {
            // 2.1. Generate implied end tags, except for elements with the same tag name as the token.
            generate_implied_end_tags(tag);

            // 2.2. If the tag name of the end tag token does not match the tag name of the current node, this is a parse error.
            if (!tag->has_name_of(node->element_name.get()))
                ParseError(tag);

            // 2.3. Pop all the nodes from the current node up to node, including node, then stop these steps.
            while (this->open_elements.size() != node_index)
                pop_the_current_node_off_the_stack_of_open_elements();

            return;
        }

        // 3. Otherwise, if node is in the special category, then this is a parse error; ignore the token, and abort these steps.
        if (is_special(node->element_name.get()))
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
                !doctype_token->has_name_of(Html::globals->HTML_html.get()) ||
                doctype_token->public_identifier.get() != 0 ||
                (doctype_token->system_identifier.get() != 0 && !equals<UnicodeString, true>(doctype_token->system_identifier.get(), Html::globals->DOCTYPE_legacy_compat.get()))
            )
            &&
            (
                !(equals<UnicodeString, true>(doctype_token->public_identifier.get(), Html::globals->DOCTYPE_html_4_0_public_identifier.get()) &&
                    (doctype_token->system_identifier.get() == 0 || equals<UnicodeString, true>(doctype_token->system_identifier.get(), Html::globals->DOCTYPE_html_4_0_system_identifier.get())))
                    &&
                !(equals<UnicodeString, true>(doctype_token->public_identifier.get(), Html::globals->DOCTYPE_html_4_01_public_identifier.get()) &&
                    (doctype_token->system_identifier.get() == 0 || equals<UnicodeString, true>(doctype_token->system_identifier.get(), Html::globals->DOCTYPE_html_4_01_system_identifier.get())))
                    &&
                !(equals<UnicodeString, true>(doctype_token->public_identifier.get(), Html::globals->DOCTYPE_xhtml_1_0_public_identifier.get()) &&
                    (doctype_token->system_identifier.get() == 0 || equals<UnicodeString, true>(doctype_token->system_identifier.get(), Html::globals->DOCTYPE_xhtml_1_0_system_identifier.get())))
                    &&
                !(equals<UnicodeString, true>(doctype_token->public_identifier.get(), Html::globals->DOCTYPE_xhtml_1_1_public_identifier.get()) &&
                    (doctype_token->system_identifier.get() == 0 || equals<UnicodeString, true>(doctype_token->system_identifier.get(), Html::globals->DOCTYPE_xhtml_1_1_system_identifier.get())))
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
        if (doctype_token->name.get() != 0 && !doctype_token->has_name_of(Html::globals->HTML_html.get()))
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

    void TreeConstruction::ParseError(const Token* token)
    {
        char token_string[0x40];
        token->GetDebugString(token_string, _countof(token_string));

        char full_error[0x100];
        sprintf_s(full_error, "token=%s", token_string);

        ParseError(full_error);
    }

    void TreeConstruction::ParseError(const Token* token, const char* error)
    {
        char token_string[0x40];
        token->GetDebugString(token_string, _countof(token_string));

        char full_error[0x100];
        sprintf_s(full_error, "%s; token=%s", error, token_string);

        ParseError(full_error);
    }

    void TreeConstruction::HandleNyi(const Token* token, bool log)
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
        std::shared_ptr<StartTagToken> tag = std::make_shared<StartTagToken>();
        tag->name = name->name;

        tree_construction_dispatcher(tag, 0);
    }

    void TreeConstruction::act_as_if_an_end_tag_token_had_been_seen(ElementName* name, bool* ignored)
    {
        std::shared_ptr<EndTagToken> tag = std::make_shared<EndTagToken>();
        tag->name = name->name;

        tree_construction_dispatcher(tag, ignored);
    }

    void TreeConstruction::close_a_p_element()
    {
        generate_implied_end_tags(Html::globals->HTML_p.get());

        if (this->CurrentNode()->element_name->equals(Html::globals->HTML_p.get()) == false)
            ParseError("close_a_p_element");

        while (true)
        {
            std::shared_ptr<ElementNode> popped = std::static_pointer_cast<ElementNode>(this->CurrentNode());
            pop_the_current_node_off_the_stack_of_open_elements();

            if (this->CurrentNode()->element_name->equals(Html::globals->HTML_p.get()))
                break;
        }
    }

    void TreeConstruction::write_element(TokenRef element)
    {
        tree_construction_dispatcher(element, 0);
    }

    void TreeConstruction::tree_construction_dispatcher(TokenRef token, bool* ignored)
    {
        bool reprocess;

        if (InForeignContent(token.get()))
            apply_the_rules_for_parsing_tokens_in_foreign_content(token, ignored, &reprocess);
        else
            apply_the_rules_for(this->insertion_mode, token, ignored, &reprocess);

        if (reprocess)
            tree_construction_dispatcher(token, ignored);
    }

    void TreeConstruction::apply_the_rules_for(InsertionMode insertion_mode, TokenRef token, bool* ignored, bool* reprocess)
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
                CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    {
                        DocTypeToken* doctype_token = (DocTypeToken*)token.get();

                        if (!IsValidDocType(doctype_token))
                            ParseError("!IsValidDocType(doctype_token)");

                        std::shared_ptr<DocumentTypeNode> doctype_node = std::make_shared<DocumentTypeNode>();
                        doctype_node->name = doctype_token->name.get() == 0 ? std::make_shared<UnicodeString>() : doctype_token->name;
                        doctype_node->publicId = doctype_token->public_identifier.get() == 0 ? std::make_shared<UnicodeString>() : doctype_token->public_identifier;
                        doctype_node->systemId = doctype_token->system_identifier.get() == 0 ? std::make_shared<UnicodeString>() : doctype_token->system_identifier;

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
                    HandleNyi(token.get(), true); // $ NYI

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
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::comment_token:
                    {
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            std::shared_ptr<ElementNode> element;
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
                            HandleNyi(token.get(), false); // $ NYI

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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_head.get()) ||
                            tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_br.get()))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token.get(), "not head, body, html, or br");
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
                    std::shared_ptr<ElementNode> element = std::make_shared<ElementNode>();
                    element->Initialize(Html::globals->HTML_html, std::make_shared<StringMap>());

                    push_onto_the_stack_of_open_elements(element);

                    // If the Document is being loaded as part of navigation of a browsing context, then: run the application 
                    // cache selection algorithm with no manifest, passing it the Document object.
                    HandleNyi(token.get(), true); // $ NYI

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_head.get()))
                        {
                            std::shared_ptr<ElementNode> element;
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_head.get()) ||
                            tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_br.get()))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token.get());
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
                    act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_head.get());

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_base.get()) ||
                            tag->has_name_of(Html::globals->HTML_basefont.get()) ||
                            tag->has_name_of(Html::globals->HTML_bgsound.get()) ||
                            tag->has_name_of(Html::globals->HTML_link.get()))
                        {
                            std::shared_ptr<ElementNode> element;
                            insert_an_HTML_element(tag, &element);
                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_meta.get()))
                        {
                            std::shared_ptr<ElementNode> element;
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
                            HandleNyi(token.get(), false); // $ NYI
                        }
                        else if (tag->has_name_of(Html::globals->HTML_title.get()))
                        {
                            generic_RCDATA_element_parsing_algorithm(tag);
                        }
                        else if ((tag->has_name_of(Html::globals->HTML_noscript.get()) && this->scripting_flag) ||
                            tag->has_name_of(Html::globals->HTML_noframes.get()) ||
                            tag->has_name_of(Html::globals->HTML_style.get()))
                        {
                            generic_raw_text_element_parsing_algorithm(tag);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_noscript.get()) && !this->scripting_flag)
                        {
                            insert_an_HTML_element(tag, 0);
                            switch_the_insertion_mode(InsertionMode::in_head_noscript_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_script.get()))
                        {
                            std::shared_ptr<ElementNode> element;
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
                            HandleNyi(token.get(), false); // $ NYI

                            this->CurrentNode()->Append(element);
                            push_onto_the_stack_of_open_elements(element);

                            this->parser->tokenizer->SwitchToState(TokenizerState::script_data_state);
                            this->original_insertion_mode = this->insertion_mode;
                            this->insertion_mode = InsertionMode::text_insertion_mode;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_head.get()))
                        {
                            ParseError(token.get());
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_head.get()))
                        {
                            pop_the_current_node_off_the_stack_of_open_elements();
                            switch_the_insertion_mode(InsertionMode::after_head_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_br.get()))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token.get());
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
                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_head.get(), 0);

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
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_basefont.get()) ||
                            tag->has_name_of(Html::globals->HTML_bgsound.get()) ||
                            tag->has_name_of(Html::globals->HTML_link.get()) ||
                            tag->has_name_of(Html::globals->HTML_meta.get()) ||
                            tag->has_name_of(Html::globals->HTML_noframes.get()) ||
                            tag->has_name_of(Html::globals->HTML_style.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_head.get()) ||
                            tag->has_name_of(Html::globals->HTML_noscript.get()))
                        {
                            ParseError(token.get());
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_noscript.get()))
                        {
                            pop_the_current_node_off_the_stack_of_open_elements();
                            switch_the_insertion_mode(InsertionMode::in_head_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_br.get()))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token.get());
                            (*ignored) = true;
                        }
                    }
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_noscript.get(), 0);

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()))
                        {
                            std::shared_ptr<ElementNode> element;
                            insert_an_HTML_element(tag, &element);

                            this->frameset_ok = false;

                            switch_the_insertion_mode(InsertionMode::in_body_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_frameset.get()))
                        {
                            std::shared_ptr<ElementNode> element;
                            insert_an_HTML_element(tag, &element);

                            switch_the_insertion_mode(InsertionMode::in_frameset_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_base.get()) ||
                            tag->has_name_of(Html::globals->HTML_basefont.get()) ||
                            tag->has_name_of(Html::globals->HTML_bgsound.get()) ||
                            tag->has_name_of(Html::globals->HTML_link.get()) ||
                            tag->has_name_of(Html::globals->HTML_meta.get()) ||
                            tag->has_name_of(Html::globals->HTML_noframes.get()) ||
                            tag->has_name_of(Html::globals->HTML_script.get()) ||
                            tag->has_name_of(Html::globals->HTML_style.get()) ||
                            tag->has_name_of(Html::globals->HTML_title.get()))
                        {
                            ParseError(token.get());

                            push_onto_the_stack_of_open_elements(this->head_element);

                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);

                            remove_node_from_the_stack_of_open_elements(this->head_element.get());
                        }
                        else if (tag->has_name_of(Html::globals->HTML_head.get()))
                        {
                            ParseError(token.get());
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_br.get()))
                        {
                            anything_else = true;
                        }
                        else
                        {
                            ParseError(token.get());
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
                    act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_body.get());

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

                        switch (character_token->data)
                        {
                        case 0x0000:
                            ParseError(token.get());
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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        std::shared_ptr<StartTagToken> tag = std::static_pointer_cast<StartTagToken>(token);

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            ParseError(token.get());

                            // For each attribute on the token, check to see if the attribute is already present on the top element of 
                            // the stack of open elements. If it is not, add the attribute and its corresponding value to that element.
                            HandleNyi(token.get(), true); // $ NYI
                        }
                        else if (tag->has_name_of(Html::globals->HTML_base.get()) ||
                            tag->has_name_of(Html::globals->HTML_basefont.get()) ||
                            tag->has_name_of(Html::globals->HTML_bgsound.get()) ||
                            tag->has_name_of(Html::globals->HTML_link.get()) ||
                            tag->has_name_of(Html::globals->HTML_meta.get()) ||
                            tag->has_name_of(Html::globals->HTML_noframes.get()) ||
                            tag->has_name_of(Html::globals->HTML_script.get()) ||
                            tag->has_name_of(Html::globals->HTML_style.get()) ||
                            tag->has_name_of(Html::globals->HTML_title.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()))
                        {
                            ParseError(token.get());

                            if (this->open_elements.size() == 1 || !this->open_elements[1]->has_element_name(Html::globals->HTML_body.get()))
                            {
                                // fragment case
                                (*ignored) = true;
                            }
                            else
                            {
                                this->frameset_ok = false;

                                std::shared_ptr<ElementNode> element = this->open_elements[1];

                                // for each attribute on the token, check to see if the attribute is already present on the body 
                                // (the second element) on the stack of open elements, and if it is not, add the attribute and its 
                                // corresponding value to that element.
                                HandleNyi(token.get(), false); // $ NYI
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_frameset.get()))
                        {
                            ParseError(token.get());

                            if (this->open_elements.size() == 1 || !this->open_elements[1]->has_element_name(Html::globals->HTML_body.get()))
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
                                HandleNyi(token.get(), true); // $ NYI
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_address.get()) ||
                            tag->has_name_of(Html::globals->HTML_article.get()) ||
                            tag->has_name_of(Html::globals->HTML_aside.get()) ||
                            tag->has_name_of(Html::globals->HTML_blockquote.get()) ||
                            tag->has_name_of(Html::globals->HTML_center.get()) ||
                            tag->has_name_of(Html::globals->HTML_details.get()) ||
                            tag->has_name_of(Html::globals->HTML_dialog.get()) ||
                            tag->has_name_of(Html::globals->HTML_dir.get()) ||
                            tag->has_name_of(Html::globals->HTML_div.get()) ||
                            tag->has_name_of(Html::globals->HTML_dl.get()) ||
                            tag->has_name_of(Html::globals->HTML_fieldset.get()) ||
                            tag->has_name_of(Html::globals->HTML_figcaption.get()) ||
                            tag->has_name_of(Html::globals->HTML_figure.get()) ||
                            tag->has_name_of(Html::globals->HTML_footer.get()) ||
                            tag->has_name_of(Html::globals->HTML_header.get()) ||
                            tag->has_name_of(Html::globals->HTML_hgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_main.get()) ||
                            tag->has_name_of(Html::globals->HTML_menu.get()) ||
                            tag->has_name_of(Html::globals->HTML_nav.get()) ||
                            tag->has_name_of(Html::globals->HTML_ol.get()) ||
                            tag->has_name_of(Html::globals->HTML_p.get()) ||
                            tag->has_name_of(Html::globals->HTML_section.get()) ||
                            tag->has_name_of(Html::globals->HTML_summary.get()) ||
                            tag->has_name_of(Html::globals->HTML_ul.get()))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            insert_an_HTML_element(tag.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_h1.get()) ||
                            tag->has_name_of(Html::globals->HTML_h2.get()) ||
                            tag->has_name_of(Html::globals->HTML_h3.get()) ||
                            tag->has_name_of(Html::globals->HTML_h4.get()) ||
                            tag->has_name_of(Html::globals->HTML_h5.get()) ||
                            tag->has_name_of(Html::globals->HTML_h6.get()))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_h1.get()) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h2.get()) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h3.get()) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h4.get()) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h5.get()) ||
                                this->CurrentNode()->has_element_name(Html::globals->HTML_h6.get()))
                            {
                                ParseError(token.get());
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }

                            insert_an_HTML_element(tag.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_pre.get()) ||
                            tag->has_name_of(Html::globals->HTML_listing.get()))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            insert_an_HTML_element(tag.get(), 0);

                            // If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to the
                            // next one. (Newlines at the start of pre blocks are ignore as an authoring convenience.)
                            this->ignore_line_feed = true;

                            this->frameset_ok = false;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_form.get()))
                        {
                            if (this->form_element.get() != 0)
                            {
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                            else
                            {
                                if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                                {
                                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                                }

                                std::shared_ptr<ElementNode> element;
                                insert_an_HTML_element(tag.get(), &element);

                                this->form_element = element;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_li.get()))
                        {
                            // 1. Set the frameset-ok flag to "not ok".
                            this->frameset_ok = false;

                            // 2. Initialize node to be the current node (the bottommost node of the stack).
                            for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
                            {
                                std::shared_ptr<ElementNode> node = (*it);

                                // 3. Loop: If node is an li element, then act as if an end tag with the tag name "li" had been seen, then jump to the last step.
                                if (node->has_element_name(Html::globals->HTML_li.get()))
                                {
                                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_li.get(), 0);
                                    break;
                                }

                                if (is_special(node->element_name.get()) &&
                                    !(node->has_element_name(Html::globals->HTML_address.get()) ||
                                    node->has_element_name(Html::globals->HTML_div.get()) ||
                                    node->has_element_name(Html::globals->HTML_p.get())))
                                {
                                    // 4. If node is in the special category, but is not an address, div, or p element, then jump to the last step.
                                    break;
                                }

                                // 5. Otherwise, set node to the previous entry in the stack of open elements and return to the step labeled loop.                            }
                            }

                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            insert_an_HTML_element(tag.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_dd.get()) ||
                            tag->has_name_of(Html::globals->HTML_dt.get()))
                        {
                            this->frameset_ok = false;

                            for (ElementList::reverse_iterator it = this->open_elements.rbegin(); it != this->open_elements.rend(); it++)
                            {
                                std::shared_ptr<ElementNode> node = (*it);

                                if (node->has_element_name(Html::globals->HTML_dd.get()))
                                {
                                    generate_implied_end_tags(Html::globals->HTML_dd.get());

                                    if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dd.get()) == false)
                                        ParseError(tag.get());

                                    while (true)
                                    {
                                        std::shared_ptr<ElementNode> popped = std::static_pointer_cast<ElementNode>(this->CurrentNode());
                                        pop_the_current_node_off_the_stack_of_open_elements();

                                        if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dd.get()))
                                            break;
                                    }

                                    break;
                                }

                                if (node->has_element_name(Html::globals->HTML_dt.get()))
                                {
                                    generate_implied_end_tags(Html::globals->HTML_dt.get());

                                    if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dt.get()) == false)
                                        ParseError(tag.get());

                                    while (true)
                                    {
                                        std::shared_ptr<ElementNode> popped = std::static_pointer_cast<ElementNode>(this->CurrentNode());
                                        pop_the_current_node_off_the_stack_of_open_elements();

                                        if (this->CurrentNode()->element_name->equals(Html::globals->HTML_dt.get()))
                                            break;
                                    }

                                    break;
                                }

                                // 4. If node is in the special category, but is not an address, div, or p element, then jump to the last step.
                                if (is_special(node->element_name.get()) &&
                                    !node->element_name->equals(Html::globals->HTML_address.get()) &&
                                    !node->element_name->equals(Html::globals->HTML_div.get()) &&
                                    !node->element_name->equals(Html::globals->HTML_p.get()))
                                {
                                    break;
                                }
                            }

                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                close_a_p_element();
                            }

                            insert_an_HTML_element(tag.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_plaintext.get()))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            insert_an_HTML_element(tag.get(), 0);

                            this->parser->tokenizer->SwitchToState(TokenizerState::PLAINTEXT_state);

                            // Note: Once a start tag with the tag name "plaintext" has been seen, that will be the last token ever
                            // seen other than character tokens (and the end-of-file token), because there is no way to switch out of
                            // the PLAINTEXT state.
                        }
                        else if (tag->has_name_of(Html::globals->button_element_name.get()))
                        {
                            if (has_element_in_scope(Html::globals->button_element_name.get()))
                            {
                                ParseError(token.get());

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->button_element_name.get(), 0);

                                (*reprocess = true);
                            }
                            else
                            {
                                reconstruct_the_active_formatting_elements();
                                insert_an_HTML_element(tag.get(), 0);
                                this->frameset_ok = false;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_a.get()))
                        {
                            // If the list of active formatting elements contains an element whose tag name is "a" between the end of 
                            // the list and the last marker on the list (or the start of the list if there is no marker on the list), 
                            // then this is a parse error; act as if an end tag with the tag name "a" had been seen, then remove that 
                            // element from the list of active formatting elements and the stack of open elements if the end tag didn't 
                            // already remove it (it might not have if the element is not in table scope).

                            int formatting_element_index = this->active_formatting_elements.size() - 1;
                            std::shared_ptr<FormattingElement> formatting_element;

                            while (true)
                            {
                                if (formatting_element_index == -1)
                                    break;

                                std::shared_ptr<FormattingElement> item = this->active_formatting_elements.at(formatting_element_index);

                                if (item->IsMarker())
                                    break;

                                if (item->element->has_element_name(Html::globals->HTML_a.get()))
                                {
                                    formatting_element = item;
                                    break;
                                }

                                formatting_element_index--;
                            }

                            if (formatting_element.get() != 0)
                            {
                                ParseError(tag.get());

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_a.get(), 0);

                                remove_node_from_the_list_of_active_formatting_elements(formatting_element->element.get());
                                remove_node_from_the_stack_of_open_elements(formatting_element->element.get());
                            }

                            reconstruct_the_active_formatting_elements();

                            std::shared_ptr<ElementNode> element;
                            insert_an_HTML_element(tag.get(), &element);

                            push_onto_the_list_of_active_formatting_elements(element, tag);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_b.get()) ||
                            tag->has_name_of(Html::globals->HTML_big.get()) ||
                            tag->has_name_of(Html::globals->HTML_code.get()) ||
                            tag->has_name_of(Html::globals->HTML_em.get()) ||
                            tag->has_name_of(Html::globals->HTML_font.get()) ||
                            tag->has_name_of(Html::globals->HTML_i.get()) ||
                            tag->has_name_of(Html::globals->HTML_s.get()) ||
                            tag->has_name_of(Html::globals->HTML_small.get()) ||
                            tag->has_name_of(Html::globals->HTML_strike.get()) ||
                            tag->has_name_of(Html::globals->HTML_strong.get()) ||
                            tag->has_name_of(Html::globals->HTML_tt.get()) ||
                            tag->has_name_of(Html::globals->HTML_u.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            std::shared_ptr<ElementNode> element;
                            insert_an_HTML_element(tag.get(), &element);

                            push_onto_the_list_of_active_formatting_elements(element, tag);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_nobr.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            if (has_element_in_scope(Html::globals->HTML_nobr.get()))
                            {
                                ParseError(token.get());

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_nobr.get(), 0);

                                reconstruct_the_active_formatting_elements();
                            }

                            std::shared_ptr<ElementNode> element;
                            insert_an_HTML_element(tag.get(), &element);

                            push_onto_the_list_of_active_formatting_elements(element, tag);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_applet.get()) ||
                            tag->has_name_of(Html::globals->HTML_marquee.get()) ||
                            tag->has_name_of(Html::globals->object_element_name.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);

                            insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();

                            this->frameset_ok = false;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_table.get()))
                        {
                            if (this->document->mode != Document::quirks_mode &&
                                has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            insert_an_HTML_element(tag.get(), 0);

                            this->frameset_ok = false;

                            switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_area.get()) ||
                            tag->has_name_of(Html::globals->HTML_br.get()) ||
                            tag->has_name_of(Html::globals->HTML_embed.get()) ||
                            tag->has_name_of(Html::globals->HTML_img.get()) ||
                            tag->has_name_of(Html::globals->HTML_keygen.get()) ||
                            tag->has_name_of(Html::globals->HTML_wbr.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            this->frameset_ok = false;
                        }
                        else if (tag->has_name_of(Html::globals->input_element_name.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            StringMap::iterator type_attribute_iterator = tag->attributes.find(Html::globals->type_attribute_name);
                            if (type_attribute_iterator == tag->attributes.end()
                                || equals<UnicodeString, false>(type_attribute_iterator->second.get(), Html::globals->hidden_type.get()))
                            {
                                this->frameset_ok = false;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_menuitem.get()) ||
                            tag->has_name_of(Html::globals->HTML_param.get()) ||
                            tag->has_name_of(Html::globals->HTML_source.get()) ||
                            tag->has_name_of(Html::globals->HTML_track.get()))
                        {
                            insert_an_HTML_element(tag.get(), 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_hr.get()))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            insert_an_HTML_element(tag.get(), 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }

                            this->frameset_ok = false;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_image.get()))
                        {
                            ParseError(token.get());

                            tag->name = Html::globals->HTML_img->name;

                            (*reprocess) = true;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_isindex.get()))
                        {
                            ParseError(token.get());

                            if (this->form_element.get() != 0)
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

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_form.get());

                                // If the token has an attribute called "action", set the action attribute on the resulting form
                                // element to the value of the "action" attribute of the token.
                                HandleNyi(token.get(), true); // $ NYI

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_hr.get());
                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_label.get());

                                // Act as if a stream of character tokens had been seen (see below for what they should say).
                                HandleNyi(token.get(), true); // $ NYI

                                // Act as if a start tag token with the tag name "input" had been seen, with all the attributes 
                                // from the "isindex" token except "name", "action", and "prompt". Set the name attribute of the 
                                // resulting input element to the value "isindex".
                                HandleNyi(token.get(), true); // $ NYI

                                // Act as if a stream of character tokens had been seen (see below for what they should say).
                                HandleNyi(token.get(), true); // $ NYI

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_label.get(), 0);

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_hr.get());

                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_form.get(), 0);

                                // If the token has an attribute with the name "prompt", then the first stream of characters must 
                                // be the same string as given in that attribute, and the second stream of characters must be empty. 
                                // Otherwise, the two streams of character tokens together should, together with the input element, 
                                // express the equivalent of "This is a searchable index. Enter search keywords: (input field)" in 
                                // the user's preferred language.
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_textarea.get()))
                        {
                            insert_an_HTML_element(tag.get(), 0);

                            // 2. If the next token is a U+000A LINE FEED (LF) character token, then ignore that token and move on to
                            // the next one. (Newlines at the start of textarea elements are ignored as an authoring convenience.)
                            this->ignore_line_feed = true;

                            this->parser->tokenizer->SwitchToState(TokenizerState::RCDATA_state);

                            this->original_insertion_mode = this->insertion_mode;

                            this->frameset_ok = false;

                            this->insertion_mode = InsertionMode::text_insertion_mode;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_xmp.get()))
                        {
                            if (has_element_in_button_scope(Html::globals->HTML_p.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_p.get(), 0);
                            }

                            reconstruct_the_active_formatting_elements();

                            this->frameset_ok = false;

                            generic_raw_text_element_parsing_algorithm(tag.get());
                        }
                        else if (tag->has_name_of(Html::globals->HTML_iframe.get()))
                        {
                            this->frameset_ok = false;

                            generic_raw_text_element_parsing_algorithm(tag.get());
                        }
                        else if (tag->has_name_of(Html::globals->HTML_noembed.get()) ||
                            (tag->has_name_of(Html::globals->HTML_noscript.get()) && this->scripting_flag))
                        {
                            generic_raw_text_element_parsing_algorithm(tag.get());
                        }
                        else if (tag->has_name_of(Html::globals->select_element_name.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);

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
                        else if (tag->has_name_of(Html::globals->HTML_optgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_option.get()))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option.get(), 0);
                            }

                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_rp.get()) ||
                            tag->has_name_of(Html::globals->HTML_rt.get()))
                        {
                            if (has_element_in_scope(Html::globals->HTML_ruby.get()))
                            {
                                generate_implied_end_tags();

                                if (!this->CurrentNode()->has_element_name(Html::globals->HTML_ruby.get()))
                                    ParseError(token.get());
                            }

                            insert_an_HTML_element(tag.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->MathML_math.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            adjust_MathML_attributes(tag.get());

                            adjust_foreign_attributes(tag.get());

                            insert_a_foreign_element(tag.get(), Html::globals->Namespace_MathML.get());

                            if (tag->self_closing)
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();

                                // and acknowledge the token's self-closing flag.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->SVG_svg.get()))
                        {
                            reconstruct_the_active_formatting_elements();

                            adjust_SVG_attributes(tag.get());

                            adjust_foreign_attributes(tag.get());

                            insert_a_foreign_element(tag.get(), Html::globals->Namespace_SVG.get());

                            if (tag->self_closing)
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();

                                // and acknowledge the token's self-closing flag.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_frame.get()) ||
                            tag->has_name_of(Html::globals->HTML_head.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            ParseError(token.get());
                            (*ignored) = true;
                        }
                        else
                        {
                            reconstruct_the_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);

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
                            if ((*it)->has_element_name(Html::globals->HTML_dd.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_dt.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_li.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_p.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_tbody.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_td.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_tfoot.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_th.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_thead.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_tr.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_body.get()) ||
                                (*it)->has_element_name(Html::globals->HTML_html.get()))
                            {
                                continue;
                            }

                            ParseError(token.get());
                            break;
                        }

                        stop_parsing();
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_body.get()))
                        {
                            if (!has_element_in_scope(Html::globals->HTML_body.get()))
                            {
                                ParseError(token.get());
                            }
                            else
                            {
                                // Otherwise, if there is a node in the stack of open elements that is not either a dd element,
                                // a dt element, an li element, an optgroup element, an option element, a p element, an rp element,
                                // an rt element, a tbody element, a td element, a tfoot element, a th element, a thead element, a 
                                // tr element, the body element, or the html element, then this is a parse error.

                                for (ElementList::iterator it = this->open_elements.begin(); it != this->open_elements.end(); it++)
                                {
                                    if ((*it)->has_element_name(Html::globals->HTML_dd.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_dt.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_li.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_optgroup.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_option.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_p.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_rp.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_rt.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_tbody.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_td.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_tfoot.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_th.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_thead.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_tr.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_body.get()) ||
                                        (*it)->has_element_name(Html::globals->HTML_html.get()))
                                    {
                                        continue;
                                    }

                                    ParseError(token.get());
                                    break;
                                }
                            }

                            switch_the_insertion_mode(InsertionMode::after_body_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_body.get(), &ignored);

                            if (!ignored)
                                (*reprocess) = true;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_address.get()) ||
                            tag->has_name_of(Html::globals->HTML_article.get()) ||
                            tag->has_name_of(Html::globals->HTML_aside.get()) ||
                            tag->has_name_of(Html::globals->HTML_blockquote.get()) ||
                            tag->has_name_of(Html::globals->button_element_name.get()) ||
                            tag->has_name_of(Html::globals->HTML_center.get()) ||
                            tag->has_name_of(Html::globals->HTML_details.get()) ||
                            tag->has_name_of(Html::globals->HTML_dialog.get()) ||
                            tag->has_name_of(Html::globals->HTML_dir.get()) ||
                            tag->has_name_of(Html::globals->HTML_div.get()) ||
                            tag->has_name_of(Html::globals->HTML_dl.get()) ||
                            tag->has_name_of(Html::globals->HTML_fieldset.get()) ||
                            tag->has_name_of(Html::globals->HTML_figcaption.get()) ||
                            tag->has_name_of(Html::globals->HTML_figure.get()) ||
                            tag->has_name_of(Html::globals->HTML_footer.get()) ||
                            tag->has_name_of(Html::globals->HTML_header.get()) ||
                            tag->has_name_of(Html::globals->HTML_hgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_listing.get()) ||
                            tag->has_name_of(Html::globals->HTML_main.get()) ||
                            tag->has_name_of(Html::globals->HTML_menu.get()) ||
                            tag->has_name_of(Html::globals->HTML_nav.get()) ||
                            tag->has_name_of(Html::globals->HTML_ol.get()) ||
                            tag->has_name_of(Html::globals->HTML_pre.get()) ||
                            tag->has_name_of(Html::globals->HTML_section.get()) ||
                            tag->has_name_of(Html::globals->HTML_summary.get()) ||
                            tag->has_name_of(Html::globals->HTML_ul.get()))
                        {
                            if (!has_element_in_scope(tag))
                            {
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(token.get());

                                while(true)
                                {
                                    std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(this->CurrentNode());
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->has_name_of(element->element_name.get()))
                                        break;
                                }
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_form.get()))
                        {
                            std::shared_ptr<ElementNode> node = this->form_element;
                            this->form_element = 0;

                            if (node.get() == 0 || !has_element_in_scope(node.get()))
                            {
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (this->CurrentNode() != node)
                                    ParseError(token.get());

                                remove_node_from_the_stack_of_open_elements(node.get());
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_p.get()))
                        {
                            if (!has_element_in_button_scope(tag))
                            {
                                ParseError(token.get());

                                act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_p.get());

                                (*reprocess) = true;
                            }
                            else
                            {
                                // 1. Generate implied end tags, except for elements with the same tag name as the token.
                                generate_implied_end_tags(tag);

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(token.get());

                                while(true)
                                {
                                    std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(this->CurrentNode());
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->has_name_of(element->element_name.get()))
                                        break;
                                }
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_li.get()))
                        {
                            if (!has_element_in_list_item_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;
                            }
                            else
                            {
                                // 1. Generate implied end tags, except for elements with the same tag name as the token.
                                generate_implied_end_tags(tag);

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(token.get());

                                while(true)
                                {
                                    std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(this->CurrentNode());
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->has_name_of(element->element_name.get()))
                                        break;
                                }
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_dd.get()) ||
                            tag->has_name_of(Html::globals->HTML_dt.get()))
                        {
                            if (!has_element_in_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;
                            }
                            else
                            {
                                // 1. Generate implied end tags, except for elements with the same tag name as the token.
                                generate_implied_end_tags(tag);

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(token.get());

                                while(true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->has_name_of(element->element_name.get()))
                                        break;
                                }
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_h1.get()) ||
                            tag->has_name_of(Html::globals->HTML_h2.get()) ||
                            tag->has_name_of(Html::globals->HTML_h3.get()) ||
                            tag->has_name_of(Html::globals->HTML_h4.get()) ||
                            tag->has_name_of(Html::globals->HTML_h5.get()) ||
                            tag->has_name_of(Html::globals->HTML_h6.get()))
                        {
                            if (!(has_element_in_scope(Html::globals->HTML_h1.get()) ||
                                has_element_in_scope(Html::globals->HTML_h2.get()) ||
                                has_element_in_scope(Html::globals->HTML_h3.get()) ||
                                has_element_in_scope(Html::globals->HTML_h4.get()) ||
                                has_element_in_scope(Html::globals->HTML_h5.get()) ||
                                has_element_in_scope(Html::globals->HTML_h6.get())))
                            {
                                ParseError(token.get());

                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(token.get());

                                while(true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->HTML_h1.get()) ||
                                        element->has_element_name(Html::globals->HTML_h2.get()) ||
                                        element->has_element_name(Html::globals->HTML_h3.get()) ||
                                        element->has_element_name(Html::globals->HTML_h4.get()) ||
                                        element->has_element_name(Html::globals->HTML_h5.get()) ||
                                        element->has_element_name(Html::globals->HTML_h6.get()))
                                        break;
                                }
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_a.get()) ||
                            tag->has_name_of(Html::globals->HTML_b.get()) ||
                            tag->has_name_of(Html::globals->HTML_big.get()) ||
                            tag->has_name_of(Html::globals->HTML_code.get()) ||
                            tag->has_name_of(Html::globals->HTML_em.get()) ||
                            tag->has_name_of(Html::globals->HTML_font.get()) ||
                            tag->has_name_of(Html::globals->HTML_i.get()) ||
                            tag->has_name_of(Html::globals->HTML_nobr.get()) ||
                            tag->has_name_of(Html::globals->HTML_s.get()) ||
                            tag->has_name_of(Html::globals->HTML_small.get()) ||
                            tag->has_name_of(Html::globals->HTML_strike.get()) ||
                            tag->has_name_of(Html::globals->HTML_strong.get()) ||
                            tag->has_name_of(Html::globals->HTML_tt.get()) ||
                            tag->has_name_of(Html::globals->HTML_u.get()))
                        {
                            adoption_agency_algorithm(tag, ignored);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_applet.get()) ||
                            tag->has_name_of(Html::globals->HTML_marquee.get()) ||
                            tag->has_name_of(Html::globals->object_element_name.get()))
                        {
                            if (!has_element_in_scope(tag))
                            {
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(token.get());

                                while(true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->has_name_of(element->element_name.get()))
                                        break;
                                }

                                clear_the_list_of_active_formatting_elements_up_to_the_last_marker();
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_br.get()))
                        {
                            ParseError(token.get());

                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_br.get());

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
                        CharacterToken* character_token = (CharacterToken*)token.get();
                        this->CurrentNode()->Insert(character_token->data);
                    }
                    break;

                case Token::Type::end_of_file_token:
                    {
                        ParseError(token.get());

                        if (this->CurrentNode()->has_element_name(Html::globals->HTML_script.get()))
                        {
                            // mark the script element as "already started".
                            HandleNyi(token.get(), true); // $ NYI
                        }

                        pop_the_current_node_off_the_stack_of_open_elements();

                        switch_the_insertion_mode(this->original_insertion_mode);

                        (*reprocess) = true;
                    }
                    break;

                case Token::Type::end_tag_token:
                    {
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_script.get()))
                        {
                            perform_a_microtask_checkpoint();

                            provide_a_stable_state();

                            std::shared_ptr<ElementNode> script = this->CurrentNode();

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
                        if (this->CurrentNode()->has_element_name(Html::globals->HTML_table.get()) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_tbody.get()) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_tfoot.get()) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_thead.get()) ||
                            this->CurrentNode()->has_element_name(Html::globals->HTML_tr.get()))
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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        std::shared_ptr<StartTagToken> tag = std::static_pointer_cast<StartTagToken>(token);

                        if (tag->has_name_of(Html::globals->HTML_caption.get()))
                        {
                            clear_the_stack_back_to_a_table_context();

                            insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();

                            insert_an_HTML_element(tag.get(), 0);

                            switch_the_insertion_mode(InsertionMode::in_caption_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_colgroup.get()))
                        {
                            clear_the_stack_back_to_a_table_context();

                            insert_an_HTML_element(tag.get(), 0);

                            switch_the_insertion_mode(InsertionMode::in_column_group_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_col.get()))
                        {
                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_colgroup.get());

                            (*reprocess) = true;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()))
                        {
                            clear_the_stack_back_to_a_table_context();

                            insert_an_HTML_element(tag.get(), 0);

                            switch_the_insertion_mode(InsertionMode::in_table_body_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_tbody.get());

                            (*reprocess) = true;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_table.get()))
                        {
                            ParseError(token.get());

                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_table.get(), &ignored);

                            if (!ignored)
                            {
                                // Note: The fake end tag token here can only be ignored in the fragment case.
                                (*reprocess) = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_style.get()) ||
                            tag->has_name_of(Html::globals->HTML_script.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_head_insertion_mode, tag, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->input_element_name.get()))
                        {
                            // If the token does not have an attribute with the name "type", or if it does, but that attribute's 
                            // value is not an ASCII case-insensitive match for the string "hidden", then: act as described in 
                            // the "anything else" entry below.
                            HandleNyi(token.get(), true); // $ NYI

                            if (false) // $
                            {
                            }
                            else
                            {
                                ParseError(token.get());

                                insert_an_HTML_element(tag.get(), 0);

                                pop_the_current_node_off_the_stack_of_open_elements();

                                if (tag->self_closing)
                                {
                                    // Acknowledge the token's self-closing flag, if it is set.
                                    tag->acknowledged = true;
                                }
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_form.get()))
                        {
                            ParseError(token.get());

                            if (this->form_element.get() != 0)
                            {
                                (*ignored) = true;
                            }
                            else
                            {
                                insert_an_HTML_element(tag.get(), &this->form_element);

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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_table.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                // fragment case
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                            else
                            {
                                while (true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->HTML_table.get()))
                                        break;
                                }

                                reset_the_insertion_mode_appropriately();
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            ParseError(token.get());
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
                            ParseError(token.get());

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
                    in_table_insertion_mode_anything_else(token.get());
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
                        std::shared_ptr<CharacterToken> character_token = std::static_pointer_cast<CharacterToken>(token);

                        switch (character_token->data)
                        {
                        case 0x0000:
                            ParseError(token.get());
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

                    for (std::vector<std::shared_ptr<CharacterToken> >::iterator it = this->pending_table_character_tokens.begin(); it != this->pending_table_character_tokens.end(); it++)
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
                        for (std::vector<std::shared_ptr<CharacterToken> >::iterator it = this->pending_table_character_tokens.begin(); it != this->pending_table_character_tokens.end(); it++)
                        {
                            in_table_insertion_mode_anything_else(it->get());
                        }
                    }
                    else
                    {
                        for (std::vector<std::shared_ptr<CharacterToken> >::iterator it = this->pending_table_character_tokens.begin(); it != this->pending_table_character_tokens.end(); it++)
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_caption.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!this->CurrentNode()->has_element_name(Html::globals->HTML_caption.get()))
                                    ParseError(token.get());

                                while (true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->HTML_caption.get()))
                                        break;
                                }

                                clear_the_list_of_active_formatting_elements_up_to_the_last_marker();

                                switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_table.get()))
                        {
                            ParseError(token.get());

                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_caption.get(), &ignored);

                            if (!ignored)
                                (*reprocess) = true;

                            // Note: The fake end tag token here can only be ignored in the fragment case.
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            ParseError(token.get());

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            ParseError(token.get());

                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_caption.get(), &ignored);

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_col.get()))
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_colgroup.get()))
                        {
                            if (this->CurrentNode() == this->open_elements.front())
                            {
                                ParseError(token.get());

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();

                                switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_col.get()))
                        {
                            ParseError(token.get());

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
                        if (this->CurrentNode() == this->open_elements.front())
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

                    act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_colgroup.get(), &ignored);

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            clear_the_stack_back_to_a_table_body_context();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_row_insertion_mode);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()))
                        {
                            ParseError(token.get());

                            act_as_if_a_start_tag_token_had_been_seen(Html::globals->HTML_tr.get());

                            (*reprocess) = true;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()))
                        {
                            if (!(has_element_in_table_scope(Html::globals->HTML_tbody.get()) ||
                                has_element_in_table_scope(Html::globals->HTML_thead.get()) ||
                                has_element_in_table_scope(Html::globals->HTML_tfoot.get())))
                            {
                                ParseError(token.get());

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_body_context();

                                act_as_if_an_end_tag_token_had_been_seen(this->CurrentNode()->element_name.get(), 0);

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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_body_context();

                                pop_the_current_node_off_the_stack_of_open_elements();

                                switch_the_insertion_mode(InsertionMode::in_table_insertion_mode);
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_table.get()))
                        {
                            if (!(has_element_in_table_scope(Html::globals->HTML_tbody.get()) ||
                                has_element_in_table_scope(Html::globals->HTML_thead.get()) ||
                                has_element_in_table_scope(Html::globals->HTML_tfoot.get())))
                            {
                                ParseError(token.get());

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                clear_the_stack_back_to_a_table_body_context();

                                act_as_if_an_end_tag_token_had_been_seen(this->CurrentNode()->element_name.get(), 0);

                                (*reprocess) = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            ParseError(token.get());

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()))
                        {
                            clear_the_stack_back_to_a_table_row_context();

                            insert_an_HTML_element(tag, 0);

                            switch_the_insertion_mode(InsertionMode::in_cell_insertion_mode);

                            insert_a_marker_at_the_end_of_the_list_of_active_formatting_elements();
                        }
                        else if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_tr.get(), &ignored);

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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token.get());

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
                        else if (tag->has_name_of(Html::globals->HTML_table.get()))
                        {
                            bool ignored = false;

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_tr.get(), &ignored);

                            if (!ignored)
                                (*reprocess) = true;

                            // Note: The fake end tag token here can only be ignored in the fragment case.
                        }
                        else if (tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;
                            }
                            else
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_tr.get(), 0);

                                (*reprocess) = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()))
                        {
                            ParseError(token.get());

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            if (!(has_element_in_table_scope(Html::globals->HTML_td.get()) ||
                                has_element_in_table_scope(Html::globals->HTML_th.get())))
                            {
                                ParseError(token.get());

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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_td.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;
                            }
                            else
                            {
                                generate_implied_end_tags();

                                if (!tag->has_name_of(this->CurrentNode()->element_name.get()))
                                    ParseError(tag);

                                while (true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (tag->has_name_of(element->element_name.get()))
                                        break;
                                }

                                clear_the_list_of_active_formatting_elements_up_to_the_last_marker();

                                switch_the_insertion_mode(InsertionMode::in_row_insertion_mode);
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_body.get()) ||
                            tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_col.get()) ||
                            tag->has_name_of(Html::globals->HTML_colgroup.get()) ||
                            tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            ParseError(token.get());

                            (*ignored) = true;
                        }
                        else if (tag->has_name_of(Html::globals->HTML_table.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()))
                        {
                            if (!has_element_in_table_scope(tag))
                            {
                                ParseError(token.get());

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

                        switch (character_token->data)
                        {
                        case 0x0000:
                            ParseError(token.get());
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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_option.get()))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()))
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option.get(), 0);

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_optgroup.get()))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()))
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option.get(), 0);

                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup.get()))
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_optgroup.get(), 0);

                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->has_name_of(Html::globals->select_element_name.get()))
                        {
                            ParseError(token.get());

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name.get(), 0);
                        }
                        else if (tag->has_name_of(Html::globals->input_element_name.get()) ||
                            tag->has_name_of(Html::globals->HTML_keygen.get()) ||
                            tag->has_name_of(Html::globals->HTML_textarea.get()))
                        {
                            ParseError(token.get());

                            if (!has_element_in_select_scope(Html::globals->select_element_name.get()))
                            {
                                (*ignored) = true;
                                // fragment case
                            }
                            else
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name.get(), 0);
                                (*reprocess) = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_script.get()))
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_optgroup.get()))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()) &&
                                this->open_elements[this->open_elements.size() - 2]->has_element_name(Html::globals->HTML_optgroup.get()))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->HTML_option.get(), 0);
                            }

                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_optgroup.get()))
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }
                            else
                            {
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_option.get()))
                        {
                            if (this->CurrentNode()->has_element_name(Html::globals->HTML_option.get()))
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }
                            else
                            {
                                ParseError(token.get());
                                (*ignored) = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->select_element_name.get()))
                        {
                            if (!has_element_in_select_scope(tag))
                            {
                                ParseError(token.get());

                                (*ignored) = true;

                                // fragment case;
                            }
                            else
                            {
                                while (true)
                                {
                                    std::shared_ptr<ElementNode> element = this->CurrentNode();
                                    pop_the_current_node_off_the_stack_of_open_elements();

                                    if (element->has_element_name(Html::globals->select_element_name.get()))
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
                            ParseError(token.get());

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
                    ParseError(token.get());
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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_table.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()))
                        {
                            ParseError(token.get());

                            act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name.get(), 0);

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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_caption.get()) ||
                            tag->has_name_of(Html::globals->HTML_table.get()) ||
                            tag->has_name_of(Html::globals->HTML_tbody.get()) ||
                            tag->has_name_of(Html::globals->HTML_tfoot.get()) ||
                            tag->has_name_of(Html::globals->HTML_thead.get()) ||
                            tag->has_name_of(Html::globals->HTML_tr.get()) ||
                            tag->has_name_of(Html::globals->HTML_th.get()) ||
                            tag->has_name_of(Html::globals->HTML_td.get()))
                        {
                            ParseError(token.get());

                            if (has_element_in_table_scope(tag))
                            {
                                act_as_if_an_end_tag_token_had_been_seen(Html::globals->select_element_name.get(), 0);
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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            if (this->fragment_context.get() != 0)
                            {
                                ParseError(token.get());

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
                    ParseError(token.get());

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_frameset.get()))
                        {
                            insert_an_HTML_element(tag, 0);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_frame.get()))
                        {
                            insert_an_HTML_element(tag, 0);

                            pop_the_current_node_off_the_stack_of_open_elements();

                            if (tag->self_closing)
                            {
                                // Acknowledge the token's self-closing flag, if it is set.
                                tag->acknowledged = true;
                            }
                        }
                        else if (tag->has_name_of(Html::globals->HTML_noframes.get()))
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_frameset.get()))
                        {
                            if (this->CurrentNode() == this->open_elements.front())
                            {
                                ParseError(token.get());

                                (*ignored) = true;

                                // fragment case
                            }
                            else
                            {
                                pop_the_current_node_off_the_stack_of_open_elements();
                            }

                            if (this->fragment_context.get() == 0 &&
                                !this->CurrentNode()->has_element_name(Html::globals->HTML_frameset.get()))
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
                            ParseError(token.get());

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
                    ParseError(token.get());

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
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->CurrentNode()->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    ParseError(token.get());
                    (*ignored) = true;
                    break;

                case Token::Type::start_tag_token:
                    {
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_noframes.get()))
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
                        EndTagToken* tag = (EndTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
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
                    ParseError(token.get());

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
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
                    ParseError(token.get());

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
                        CommentToken* comment_token = (CommentToken*)token.get();

                        std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                        comment_node->data = comment_token->data;

                        this->document->Append(comment_node);
                    }
                    break;

                case Token::Type::DOCTYPE_token:
                    apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                    break;

                case Token::Type::character_token:
                    {
                        CharacterToken* character_token = (CharacterToken*)token.get();

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
                        StartTagToken* tag = (StartTagToken*)token.get();

                        if (tag->has_name_of(Html::globals->HTML_html.get()))
                        {
                            apply_the_rules_for(InsertionMode::in_body_insertion_mode, token, 0, reprocess);
                        }
                        else if (tag->has_name_of(Html::globals->HTML_noframes.get()))
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
                    ParseError(token.get());

                    (*ignored) = true;
                }
            }
            break;

        default:
            HandleError("unexpected insertion mode");
            break;
        }
    }

    void TreeConstruction::apply_the_rules_for_parsing_tokens_in_foreign_content(TokenRef token, bool* ignored, bool* reprocess)
    {
        switch (token->type)
        {
        case Token::Type::character_token:
            {
                CharacterToken* character_token = (CharacterToken*)token.get();

                switch (character_token->data)
                {
                case 0x0000:
                    ParseError(token.get());
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
                CommentToken* comment_token = (CommentToken*)token.get();

                std::shared_ptr<CommentNode> comment_node = std::make_shared<CommentNode>();
                comment_node->data = comment_token->data;

                this->CurrentNode()->Append(comment_node);
            }
            break;

        case Token::Type::DOCTYPE_token:
            ParseError(token.get());
            (*ignored) = true;
            break;

        case Token::Type::start_tag_token:
            {
                StartTagToken* tag = (StartTagToken*)token.get();

                //if (tag->has_name_of())
                //{
                //}
                //else if (tag->has_name_of())
                //{
                //}
                //else
                //{
                //}

                HandleNyi(token.get(), true); // $ NYI
            }
            break;

        case Token::Type::end_tag_token:
            {
                EndTagToken* tag = (EndTagToken*)token.get();

                //if (tag->has_name_of())
                //{
                //}
                //else
                //{
                //}

                HandleNyi(token.get(), true); // $ NYI
            }
            break;
        }
    }
}