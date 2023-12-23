#pragma once

#include "Basic.Ref.h"
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

	class Node : public IRefCounted
	{
	public:
		typedef Basic::Ref<Node> Ref;
		typedef std::vector<Node::Ref> NodeList; // $$$

	public:
		Document* html_document;
		NodeType type;
		Node* parent;
		NodeList children;

	protected:
		Node(NodeType type);

	public:
		void Insert(Codepoint c);
		virtual void Append(Node* node);
		void Remove(Node* node);

		bool has_ancestor(ElementName* element_name);
		bool has_ancestor(Node* node);
		void remove_from_parent();
		void take_all_child_nodes_of(Node* node);
		virtual void write_to_human(IStream<Codepoint>* stream, bool verbose);
		void write_pointer_to_human(IStream<Codepoint>* stream);

		bool find_element(ElementName* name, UnicodeString* attribute_name, UnicodeString* attribute_value, Basic::Ref<ElementNode>* result);
		bool find_element_with_child_count(Node* after, ElementName* name, uint32 child_count, Basic::Ref<ElementNode>* result);
		bool find_element_with_attribute_prefix(Node* after, ElementName* name, UnicodeString* attribute, UnicodeString* value, Basic::Ref<ElementNode>* result);
		bool find_element_with_attribute_value(Node* after, ElementName* name, UnicodeString* attribute, UnicodeString* value, Basic::Ref<ElementNode>* result);
		bool find_element_with_text_value(Node* after, ElementName* name, UnicodeString* value, Basic::Ref<ElementNode>* result);
		void extract_text(IStream<Codepoint>* destination);
		bool first_text(IStream<Codepoint>* destination);
	};
}