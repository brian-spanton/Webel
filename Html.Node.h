// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.ElementName.h"

namespace Html
{
    using namespace Basic;

    enum NodeType : unsigned short
    {
        ELEMENT_NODE = 1,
        ATTRIBUTE_NODE = 2, // historical
        TEXT_NODE = 3,
        CDATA_SECTION_NODE = 4, // historical
        ENTITY_REFERENCE_NODE = 5, // historical
        ENTITY_NODE = 6, // historical
        PROCESSING_INSTRUCTION_NODE = 7,
        COMMENT_NODE = 8,
        DOCUMENT_NODE = 9,
        DOCUMENT_TYPE_NODE = 10,
        DOCUMENT_FRAGMENT_NODE = 11,
        NOTATION_NODE = 12, // historical
    };

    class Document;
    class ElementNode;

    class Node : public std::enable_shared_from_this<Node>
    {
    public:
        typedef std::vector<std::shared_ptr<Node> > NodeList;

    public:
        std::weak_ptr<Document> html_document;
        NodeType type;
        std::weak_ptr<Node> parent;
        NodeList children;

    protected:
        Node(NodeType type);

    public:
        void Insert(Codepoint codepoint);
        virtual void Append(std::shared_ptr<Node> node);
        void Remove(std::shared_ptr<Node> node);

        bool has_ancestor(ElementName* element_name);
        bool has_ancestor(std::shared_ptr<Node> node);
        void remove_from_parent();
        void take_all_child_nodes_of(Node* node);
        virtual void write_to_human(IStream<Codepoint>* stream, bool verbose);
        void write_pointer_to_human(IStream<Codepoint>* stream);

        bool find_element(ElementName* name, UnicodeStringRef attribute_name, UnicodeStringRef attribute_value, std::shared_ptr<ElementNode>* result);
        bool find_element_with_child_count(Node* after, ElementName* name, uint32 child_count, std::shared_ptr<ElementNode>* result);
        bool find_element_with_attribute_prefix(Node* after, ElementName* name, UnicodeStringRef attribute, UnicodeStringRef value, std::shared_ptr<ElementNode>* result);
        bool find_element_with_attribute_value(Node* after, ElementName* name, UnicodeStringRef attribute, UnicodeStringRef value, std::shared_ptr<ElementNode>* result);
        bool find_element_with_text_value(Node* after, ElementName* name, UnicodeStringRef value, std::shared_ptr<ElementNode>* result);
        void extract_text(IStream<Codepoint>* stream);
        bool first_text(IStream<Codepoint>* stream);
    };
}