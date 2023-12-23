#pragma once

#include "Html.ElementNode.h"
#include "Http.Client.h"
#include "Basic.IProcess.h"

namespace Web
{
	using namespace Basic;
	using namespace Html;

	struct FormDataSetEntry
	{
		UnicodeString::Ref name; // $$$
		UnicodeString::Ref value; // $$$
		UnicodeString::Ref type; // $$$
	};

	typedef std::vector<FormDataSetEntry> FormDataSet;
	
	class Form : public IRefCounted
	{
	public:
		typedef Basic::Ref<Form> Ref; // $$$

		static void encode_value(UnicodeString::Ref charset, UnicodeString::Ref value, UnicodeString::Ref* result);
		static void set_control_name(Html::ElementNode* control, UnicodeString::Ref value);
		static void set_control_value(Html::ElementNode* control, UnicodeString::Ref value);
		static void url_decode(UnicodeString::Ref payload, UnicodeString::Ref encoding, bool isindex, StringMap* form_data_set);
		static void strictly_split_a_string(UnicodeString::Ref input, Codepoint delimiter, StringList* tokens);
		static void decode_value(UnicodeString::Ref value, IStream<Codepoint>* result);

		Basic::Ref<ElementNode> form_element; // $$$
		Html::ElementList controls;

		void Initialize(ElementNode* element);
		bool Submit(Http::Client* client, IProcess* client_completion, ByteString* client_cookie);

		bool find_control(UnicodeString::Ref pattern, Html::ElementNode::Ref* result);
		void find_associated_submittable_elements(Node* node, Html::ElementList* controls);
		void construct_form_data_set(Html::ElementList* controls, FormDataSet* form_data_set);
		bool construct_request(FormDataSet* form_data_set, Http::Request::Ref* request_result);
		void url_encode(FormDataSet* form_data_set, ByteString::Ref* result);
	};

	typedef std::vector<Form::Ref> FormList; // $$$
}
