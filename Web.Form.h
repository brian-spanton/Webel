// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.ElementNode.h"
#include "Web.Client.h"
#include "Basic.IProcess.h"

namespace Web
{
    using namespace Basic;
    using namespace Html;

    struct FormDataSetEntry
    {
        UnicodeStringRef name;
        UnicodeStringRef value;
        UnicodeStringRef type;
    };

    typedef std::vector<FormDataSetEntry> FormDataSet;
    
    class Form
    {
    public:
        static void encode_value(UnicodeStringRef charset, UnicodeStringRef value, UnicodeStringRef* result);
        static void set_control_name(Html::ElementNode* control, UnicodeStringRef value);
        static void set_control_value(Html::ElementNode* control, UnicodeStringRef value);
        static void url_decode(UnicodeStringRef payload, UnicodeStringRef encoding, bool isindex, StringMap* form_data_set);
        static void strictly_split_a_string(UnicodeStringRef input, Codepoint delimiter, std::vector<UnicodeStringRef>* tokens);
        static void decode_value(UnicodeString* value, IStream<Codepoint>* result);

        std::shared_ptr<ElementNode> form_element;
        Html::ElementList controls;

        void Initialize(std::shared_ptr<ElementNode> element);
        bool Submit(Web::Client* client, std::shared_ptr<IProcess> client_completion, ByteStringRef client_cookie);

        bool find_control(UnicodeStringRef pattern, std::shared_ptr<Html::ElementNode>* result);
        void find_associated_submittable_elements(std::shared_ptr<Node> node, Html::ElementList* controls);
        void construct_form_data_set(Html::ElementList* controls, FormDataSet* form_data_set);
        bool construct_request(FormDataSet* form_data_set, std::shared_ptr<Http::Request>* request_result);
        void url_encode(FormDataSet* form_data_set, ByteStringRef* result);
    };

    typedef std::vector<std::shared_ptr<Form> > FormList;
}
