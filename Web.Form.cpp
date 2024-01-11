// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Form.h"
#include "Html.Document.h"
#include "Html.Globals.h"
#include "Http.Globals.h"
#include "Basic.TextWriter.h"
#include "Basic.Globals.h"

namespace Web
{
    void Form::Initialize(std::shared_ptr<ElementNode> element)
    {
        this->form_element = element;

        find_associated_submittable_elements(element->html_document.lock(), &this->controls);
    }

    void Form::find_associated_submittable_elements(std::shared_ptr<Node> node, Html::ElementList* controls)
    {
        if (node->type == Html::NodeType::ELEMENT_NODE)
        {
            std::shared_ptr<Html::ElementNode> element = std::static_pointer_cast<Html::ElementNode>(node);

            if (element->form_owner.lock().get() == this->form_element.get() && element->IsSubmittable())
            {
                controls->push_back(element);
            }
        }

        for (Html::Node::NodeList::iterator it = node->children.begin(); it != node->children.end(); it++)
        {
            find_associated_submittable_elements(*it, controls);
        }
    }

    void Form::construct_form_data_set(Html::ElementList* controls, FormDataSet* form_data_set)
    {
        //4.10.22.4 Constructing the form data set
        //
        //The algorithm to construct the form data set for a form form optionally in the context of a submitter submitter
        //is as follows. If not specified otherwise, submitter is null.
        //1. Let controls be a list of all the submittable elements whose form owner is form, in tree order.
        // (input param)

        //2. Let the form data set be a list of name-value-type tuples, initially empty.
        form_data_set->clear();

        for (uint32 i = 0; i < controls->size(); i++)
        {
            //3. Loop: For each element field in controls, in tree order, run the following substeps:
            std::shared_ptr<Html::ElementNode> field = controls->at(i);

            //1. If any of the following conditions are met, then skip these substeps for this element:
            //◾The field element has a datalist element ancestor.
            if (field->has_ancestor(Html::globals->datalist_element_name.get()))
                continue;

            //◾The field element is disabled.
            if (field->is_disabled())
                continue;

            //◾The field element is a button but it is not submitter.
            if (field->has_element_name(Html::globals->button_element_name.get()))
                continue;

            //◾The field element is an input element whose type attribute is in the Checkbox state and whose checkedness is false.
            if (field->has_element_name(Html::globals->input_element_name.get())
                && field->attribute_equals(Html::globals->type_attribute_name, Html::globals->checkbox_type)
                && field->checkedness() == false)
                continue;

            //◾The field element is an input element whose type attribute is in the Radio Button state and whose checkedness is false.
            if (field->has_element_name(Html::globals->input_element_name.get())
                && field->attribute_equals(Html::globals->type_attribute_name, Html::globals->radio_type)
                && field->checkedness() == false)
                continue;

            //◾The field element is not an input element whose type attribute is in the Image Button state, and either the
            //field element does not have a name attribute specified, or its name attribute's value is the empty string.
            if (!(field->has_element_name(Html::globals->input_element_name.get()) && field->attribute_equals(Html::globals->type_attribute_name, Html::globals->image_type))
                && field->attribute_missing_or_empty(Html::globals->name_attribute_name))
                continue;

            //◾The field element is an object element that is not using a plugin.
            // $ "objects" are entirely NYI
            if (field->has_element_name(Html::globals->object_element_name.get()))
                continue;

            FormDataSetEntry item;

            //Otherwise, process field as follows:
            //
            //2. Let type be the value of the type IDL attribute of field.
            field->get_attribute(Html::globals->type_attribute_name, &item.type);

            //3. If the field element is an input element whose type attribute is in the Image Button state, then run these further nested substeps:
            if (field->has_element_name(Html::globals->input_element_name.get())
                && field->attribute_equals(Html::globals->type_attribute_name, Html::globals->image_type))
            {
                //    1. If the field element has a name attribute specified and its value is not the empty string, let name be that
                //     value followed by a single U+002E FULL STOP character (.). Otherwise, let name be the empty string.
                //    2. Let namex be the string consisting of the concatenation of name and a single U+0078 LATIN SMALL LETTER X character (x).
                //    3. Let namey be the string consisting of the concatenation of name and a single U+0079 LATIN SMALL LETTER Y character (y).
                //    4. The field element is submitter, and before this algorithm was invoked the user indicated a coordinate. Let x be
                //     the x-component of the coordinate selected by the user, and let y be the y-component of the coordinate selected by the user.
                //    5. Append an entry to the form data set with the name namex, the value x, and the type type.
                //    6. Append an entry to the form data set with the name namey and the value y, and the type type.
                // $ NYI

                //    7. Skip the remaining substeps for this element: if there are any more elements in controls, return to the top of the
                //     loop step, otherwise, jump to the end step below.
                continue;
            }

            //4. Let name be the value of the field element's name attribute.
            field->get_attribute(Html::globals->name_attribute_name, &item.name);

            //5. If the field element is a select element, then for each option element in the select element's list of options whose
            //   selectedness is true and that is not disabled, append an entry to the form data set with the name as the name, the
            //   value of the option element as the value, and type as the type.
            if (field->has_element_name(Html::globals->select_element_name.get()))
            {
                // $ NYI
            }

            //6. Otherwise, if the field element is an input element whose type attribute is in the Checkbox state or the Radio Button
            //   state, then run these further nested substeps:
            else if (field->has_element_name(Html::globals->input_element_name.get())
                && (field->attribute_equals(Html::globals->type_attribute_name, Html::globals->checkbox_type) || field->attribute_equals(Html::globals->type_attribute_name, Html::globals->radio_type)))
            {
                //1. If the field element has a value attribute specified, then let value be the value of that attribute; otherwise,
                //   let value be the string "on".
                if (!field->attribute_missing_or_empty(Html::globals->value_attribute_name))
                {
                    field->get_attribute(Html::globals->value_attribute_name, &item.value);
                }
                else
                {
                    item.value = Html::globals->on_value;
                }

                //2. Append an entry to the form data set with name as the name, value as the value, and type as the type.
                form_data_set->push_back(item);
            }

            //7. Otherwise, if the field element is an input element whose type attribute is in the File Upload state, then
            //   for each file selected in the input element, append an entry to the form data set with the name as the name,
            //   the file (consisting of the name, the type, and the body) as the value, and type as the type. If there are
            //   no selected files, then append an entry to the form data set with the name as the name, the empty string as
            //   the value, and application/octet-stream as the type.
            else if (field->has_element_name(Html::globals->input_element_name.get())
                && field->attribute_equals(Html::globals->type_attribute_name, Html::globals->file_type))
            {
                // $ NYI
            }

            //8. Otherwise, if the field element is an object element: try to obtain a form submission value from the plugin,
            //   and if that is successful, append an entry to the form data set with name as the name, the returned form 
            //   submission value as the value, and the string "object" as the type.
            else if (field->has_element_name(Html::globals->input_element_name.get())
                && field->attribute_equals(Html::globals->type_attribute_name, Html::globals->object_type))
            {
                // $ NYI
            }

            //9. Otherwise, append an entry to the form data set with name as the name, the value of the field element as the
            //   value, and type as the type.
            else
            {
                field->get_attribute(Html::globals->value_attribute_name, &item.value);
                form_data_set->push_back(item);
            }

            //10. If the element has a dirname attribute, and that attribute's value is not the empty string, then run these substeps:
            if (field->has_attribute(Html::globals->dirname_attribute_name))
            {
                UnicodeStringRef dirname_value;
                field->get_attribute(Html::globals->dirname_attribute_name, &dirname_value);
                if (dirname_value->size() > 0)
                {
                    //1. Let dirname be the value of the element's dirname attribute.
                    //2. Let dir be the string "ltr" if the directionality of the element is 'ltr', and "rtl" otherwise (i.e. when
                    //   the directionality of the element is 'rtl').
                    //3. Append an entry to the form data set with dirname as the name, dir as the value, and the string "direction" as the type.
                    //Note: 
                    //An element can only have a dirname attribute if it is a textarea element or an input element whose type attribute
                    //is in either the Text state or the Search state.
                    // $ NYI
                }
            }
        }

        //4. End: For the name of each entry in the form data set, and for the value of each entry in the form data set whose type
        //   is not "file" or "textarea", replace every occurrence of a U+000D CARRIAGE RETURN (CR) character not followed by a
        //   U+000A LINE FEED (LF) character, and every occurrence of a U+000A LINE FEED (LF) character not preceded by a
        //   U+000D CARRIAGE RETURN (CR) character, by a two-character string consisting of a U+000D CARRIAGE RETURN U+000A LINE FEED (CRLF)
        //   character pair.
        //
        //Note: 
        //In the case of the value of textarea elements, this newline normalization is already performed during the conversion of the
        //control's raw value into the control's value (which also performs any necessary line wrapping). In the case of input elements
        //type attributes in the File Upload state, the value is not normalized.
        // $ NYI

        //5. Return the form data set.
    }

    void Form::encode_value(UnicodeStringRef charset, UnicodeStringRef value, UnicodeStringRef* result)
    {
        std::shared_ptr<Basic::IEncoder> encoder;
        Basic::globals->GetEncoder(charset, &encoder);

        //3. For each character in the entry's name and value that cannot be expressed using the selected character
        //   encoding, replace the character by a string consisting of a U+0026 AMPERSAND character (&), a U+0023
        //   NUMBER SIGN character (#), one or more ASCII digits representing the Unicode code point of the character
        //   in base ten, and finally a U+003B SEMICOLON character (;).
        // $ NYI

        //4. Encode the entry's name and value using the encoder for the selected character encoding. The entry's name
        //   and value are now byte strings.
        ByteString encoded_value;
        encoder->set_destination(&encoded_value);
        value->write_to_stream(encoder.get());

        //5. For each byte in the entry's name and value, apply the appropriate subsubsteps from the following list:
        for (uint32 byte_index = 0; byte_index != encoded_value.size(); byte_index++)
        {
            byte b = encoded_value.at(byte_index);

            //↪If the byte is 0x20 (U+0020 SPACE if interpreted as ASCII)
            //Replace the byte with a single 0x2B byte (U+002B PLUS SIGN character (+) if interpreted as ASCII).
            if (b == 0x20)
            {
                b = 0x2B;
                encoded_value.replace(byte_index, 1, &b, 1);
            }

            //↪If the byte is in the range 0x2A, 0x2D, 0x2E, 0x30 to 0x39, 0x41 to 0x5A, 0x5F, 0x61 to 0x7A
            //Leave the byte as is.
            else if (b == 0x2A || b == 0x2D || b == 0x2E || (b >= 0x30 && b <= 0x39) || (b >= 0x41 && b <= 0x5A) || b == 0x5F || (b >= 0x61 && b <= 0x7A))
            {
            }

            //↪Otherwise
            else
            {
                //1. Let s be a string consisting of a U+0025 PERCENT SIGN character (%) followed by uppercase ASCII hex digits representing 
                //   the hexadecimal value of the byte in question (zero-padded if necessary).
                UnicodeString s;
                TextWriter writer(&s);
                writer.WriteFormat<0x10>("%%%02X", b);

                //2. Encode the string s as US-ASCII, so that it is now a byte string.
                ByteString encoded_s;
                ascii_encode(&s, &encoded_s);

                //3. Replace the byte in question in the name or value being processed by the bytes in s, preserving their relative order.
                encoded_value.replace(byte_index, 1, encoded_s);
            }
        }

        //6. Interpret the entry's name and value as Unicode strings encoded in US-ASCII. (All of the bytes in the string will be in the range 0x00 to 0x7F; the high bit will be zero throughout.) The entry's name and value are now Unicode strings again.
        UnicodeStringRef string_value = std::make_shared<UnicodeString>();
        ascii_decode(&encoded_value, string_value.get());

        (*result) = string_value;
    }

    void Form::strictly_split_a_string(UnicodeStringRef input, Codepoint delimiter, std::vector<UnicodeStringRef>* tokens)
    {
        UnicodeStringRef token = std::make_shared<UnicodeString>();

        for (int position = 0; position != input->size(); position++)
        {
            Codepoint codepoint = input->at(position);

            if (codepoint == delimiter)
            {
                tokens->push_back(token);
                token = std::make_shared<UnicodeString>();
            }
            else
            {
                token->push_back(codepoint);
            }
        }

        if (token->size() > 0)
            tokens->push_back(token);
    }

    void Form::decode_value(UnicodeString* value, IStream<Codepoint>* result)
    {
        for (uint32 codepoint_index = 0; codepoint_index != value->size(); codepoint_index++)
        {
            Codepoint codepoint = value->at(codepoint_index);

            if (codepoint == 0x002B)
            {
                codepoint = 0x0020;
            }
            else if (codepoint == 0x0025 
                && codepoint_index + 2 < value->size()
                && Uri::is_ascii_hex_digit(value->at(codepoint_index + 1))
                && Uri::is_ascii_hex_digit(value->at(codepoint_index + 2)))
            {
                UnicodeString code;
                code.insert(code.end(), value->begin() + codepoint_index + 1, value->begin() + codepoint_index + 3);

                codepoint = code.as_base_16<Codepoint>(0);

                codepoint_index += 2;
            }

            result->write_element(codepoint);
        }
    }

    void Form::url_decode(UnicodeStringRef payload, UnicodeStringRef encoding, bool isindex, StringMap* pairs)
    {
        pairs->clear();

        std::vector<UnicodeStringRef> strings;
        strictly_split_a_string(payload, 0x0026, &strings);

        if (strings.size() > 0)
        {
            UnicodeStringRef first_string = strings.at(0);
            bool encoding_override = false;

            int equals_index = first_string->find(0x003D);
            if (isindex && equals_index == UnicodeString::npos)
            {
                first_string->insert(first_string->begin(), 0x003D);
            }

            for (int string_index = 0; string_index != strings.size(); string_index++)
            {
                UnicodeStringRef string = strings.at(string_index);

                UnicodeStringRef name = std::make_shared<UnicodeString>();
                UnicodeStringRef value = std::make_shared<UnicodeString>();

                equals_index = string->find(0x003D);
                if (equals_index != UnicodeString::npos)
                {
                    UnicodeString encoded;
                    encoded.insert(encoded.end(), string->begin(), string->begin() + equals_index);
                    decode_value(&encoded, name.get());

                    encoded.clear();
                    encoded.insert(encoded.end(), string->begin() + equals_index + 1, string->end());
                    decode_value(&encoded, value.get());
                }
                else
                {
                    decode_value(string.get(), name.get());
                }

                if (encoding_override == false && equals<UnicodeString, true>(name.get(), Html::globals->_charset__name.get()))
                {
                    encoding = value;
                }

                ByteString name_bytes;
                ascii_encode(name.get(), &name_bytes);

                ByteString value_bytes;
                ascii_encode(value.get(), &value_bytes);

                if (is_null_or_empty(encoding.get()))
                    encoding = Basic::globals->us_ascii_label;
                    
                std::shared_ptr<IDecoder> decoder;
                Basic::globals->GetDecoder(encoding, &decoder);

                name->clear();
                decoder->set_destination(name.get());
                decoder->write_elements(name_bytes.address(), name_bytes.size());

                value->clear();
                decoder->set_destination(value.get());
                decoder->write_elements(value_bytes.address(), value_bytes.size());

                pairs->insert(StringMap::value_type(name, value));
            }
        }
    }

    void Form::url_encode(FormDataSet* form_data_set, ByteStringRef* result)
    {
        //1. Let result be the empty string.
        UnicodeStringRef unicode_result;
        unicode_result = std::make_shared<UnicodeString>();

        UnicodeStringRef charset;

        UnicodeStringRef accept_charset;
        this->form_element->get_attribute(Html::globals->accept_charset_attribute_name, &accept_charset);

        // $ this is commented out because it is NYI and defaulting to utf-8 is often adequate

        ////2. If the form element has an accept-charset attribute, let the selected character encoding be the result
        ////   of picking an encoding for the form.
        //if (accept_charset)
        //{
        //    pick_an_encoding_for_the_form(form, &charset);
        //}

        ////Otherwise, if the form element has no accept-charset attribute, but the document's character encoding is an
        ////ASCII-compatible character encoding, then that is the selected character encoding.
        //else if (form_document->encoding->is_ascii_compatible())
        //{
        //    charset = form_document->encoding;
        //}

        ////Otherwise, let the selected character encoding be UTF-8.
        //else
        //{
            charset = Basic::globals->utf_8_label;
        //}

        //3. Let charset be the name of the selected character encoding.

        //4. For each entry in the form data set, perform these substeps:
        for (uint32 entry_index = 0; entry_index < form_data_set->size(); entry_index++)
        {
            FormDataSetEntry& entry = form_data_set->at(entry_index);

            //1. If the entry's name is "_charset_" and its type is "hidden", replace its value with charset.
            if (equals<UnicodeString, true>(entry.name.get(), Html::globals->_charset__name.get())
                && equals<UnicodeString, true>(entry.type.get(), Html::globals->hidden_type.get()))
            {
                entry.value = charset;
            }

            //2. If the entry's type is "file", replace its value with the file's name only.
            if (equals<UnicodeString, true>(entry.type.get(), Html::globals->file_type.get()))
            {
                // $ NYI
            }

            encode_value(charset, entry.name, &entry.name);

            if (entry.value)
                encode_value(charset, entry.value, &entry.value);

            //7. If the entry's name is "isindex", its type is "text", and this is the first entry in the form data set,
            //   then append the value to result and skip the rest of the substeps for this entry, moving on to the next 
            //   entry, if any, or the next step in the overall algorithm otherwise.
            if (equals<UnicodeString, true>(entry.name.get(), Html::globals->isindex_name.get())
                && equals<UnicodeString, true>(entry.type.get(), Html::globals->text_type.get())
                && entry_index == 0)
            {
                unicode_result->append(*entry.value.get());
                continue;
            }

            //8. If this is not the first entry, append a single U+0026 AMPERSAND character (&) to result.
            if (entry_index != 0)
            {
                unicode_result->push_back(0x0026);
            }

            //9. Append the entry's name to result.
            unicode_result->append(*entry.name.get());

            //10. Append a single U+003D EQUALS SIGN character (=) to result.
            unicode_result->push_back(0x003D);

            if (entry.value)
            {
                //11. Append the entry's value to result.
                unicode_result->append(*entry.value.get());
            }
        }

        //5. Encode result as US-ASCII and return the resulting byte stream.
        std::shared_ptr<ByteString> bytes_result = std::make_shared<ByteString>();
        ascii_encode(unicode_result.get(), bytes_result.get());

        (*result) = bytes_result;
    }

    bool Form::Submit(Web::Client* client, std::shared_ptr<IProcess> completion, ByteStringRef completion_cookie)
    {
        FormDataSet form_data_set;
        construct_form_data_set(&this->controls, &form_data_set);

        std::shared_ptr<Http::Request> request;
        bool success = construct_request(&form_data_set, &request);
        if (!success)
            return false;

        client->Get(request, 0, completion, completion_cookie);
        return true;
    }

    bool Form::construct_request(FormDataSet* form_data_set, std::shared_ptr<Http::Request>* request_result)
    {
        // 1. Let form document be the form's Document.
        std::shared_ptr<Html::Document> form_document = this->form_element->html_document.lock();

        // 2. If form document has no associated browsing context or its active sandboxing flag set has its
        //    sandboxed forms browsing context flag set, then abort these steps without doing anything.
        // $ NYI

        // 3. Let form browsing context be the browsing context of form document.
        // $ NYI

        // 4. If the submitted from construct_request() method flag is not set, and the submitter element's no-validate
        //    state is false, then interactively validate the constraints of form and examine the result: if
        //    the result is negative (the constraint validation concluded that there were invalid fields and 
        //    probably informed the user of this) then fire a simple event named invalid at the form element 
        //    and then abort these steps.
        // $ NYI

        // 5. If the submitted from construct_request() method flag is not set, then fire a simple event that bubbles and 
        //    is cancelable named construct_request, at form. If the event's default action is prevented (i.e. if the event 
        //    is canceled) then abort these steps. Otherwise, continue (effectively the default action is to 
        //    perform the submission).
        // $ NYI

        // 6. Let form data set be the result of constructing the form data set for form in the context of submitter.

        // 7. Let action be the submitter element's action.
        UnicodeStringRef action;
        this->form_element->get_attribute(Html::globals->action_attribute_name, &action);

        std::shared_ptr<Uri> url;

        //8. If action is the empty string, let action be the document's address of the form document.
        if (is_null_or_empty(action.get()))
        {
            url = form_document->url;
        }
        else
        {
            //9. Resolve the URL action, relative to the submitter element. If this fails, abort these steps.
            url = std::make_shared<Uri>();
            url->Initialize();

            bool success = url->Parse(action.get(), form_document->url.get());
            if (!success)
                return false;
        }

        //10. Let action be the resulting absolute URL.
        //11. Let action components be the resulting parsed URL.
        //12. Let scheme be the scheme of the resulting parsed URL.
        // these come from Uri class

        //13. Let enctype be the submitter element's enctype.
        UnicodeStringRef enctype;
        this->form_element->get_attribute(Html::globals->enctype_attribute_name, &enctype);

        //14. Let method be the submitter element's method.
        UnicodeStringRef method;
        this->form_element->get_attribute(Html::globals->method_attribute_name, &method);

        //15. Let target be the submitter element's target.
        UnicodeStringRef target;
        this->form_element->get_attribute(Html::globals->target_attribute_name, &target);

        //16. If the user indicated a specific browsing context to use when submitting the form, then let target
        //    browsing context be that browsing context. Otherwise, apply the rules for choosing a browsing context given
        //    a browsing context name using target as the name and form browsing context as the context in which the
        //    algorithm is executed, and let target browsing context be the resulting browsing context.
        // this is the "client" input parameter

        //17. If target browsing context was created in the previous step, or, alternatively, if the form document has not
        //yet completely loaded and the submitted from construct_request() method is set, then let replace be true. Otherwise, let it be false.
        bool replace = false;

        //18. If the value of method is dialog then jump to the construct_request dialog steps.
        // $ dialog is not supported

        // $ one particular site's form seems to expect this behavior (or script is implementing it)
        if (is_null_or_empty(method.get()))
            method = Http::globals->get_method;

        bool handled = false;

        if (url->is_http_scheme())
        {
            if (equals<UnicodeString, false>(method.get(), Http::globals->get_method.get()))
            {
                handled = true;

                ByteStringRef encoded;
                url_encode(form_data_set, &encoded);

                url->query = std::make_shared<UnicodeString>();
                ascii_decode(encoded.get(), url->query.get());

                std::shared_ptr<Http::Request> request = std::make_shared<Http::Request>();
                request->Initialize();
                request->resource = url;
                request->method = Http::globals->get_method;

                (*request_result) = request;
            }
            else if (equals<UnicodeString, false>(method.get(), Http::globals->post_method.get()))
            {
                if (!enctype || equals<UnicodeString, false>(enctype.get(), Http::globals->application_x_www_form_urlencoded_media_type.get()))
                {
                    handled = true;

                    ByteStringRef encoded;
                    url_encode(form_data_set, &encoded);

                    std::shared_ptr<Http::Request> request = std::make_shared<Http::Request>();
                    request->Initialize();
                    request->resource = url;
                    request->method = Http::globals->post_method;
                    request->headers->set_string(Http::globals->header_content_type, Http::globals->application_x_www_form_urlencoded_media_type);
                    request->request_body = encoded;

                    (*request_result) = request;
                }
            }
        }

        if (!handled)
            throw FatalError("Web", "Form::construct_request unhandled form submission type");

        return true;
    }

    void Form::set_control_value(Html::ElementNode* control, UnicodeStringRef value)
    {
        control->set_attribute(Html::globals->value_attribute_name, value);
    }

    void Form::set_control_name(Html::ElementNode* control, UnicodeStringRef value)
    {
        control->set_attribute(Html::globals->name_attribute_name, value);
    }

    bool Form::find_control(UnicodeStringRef pattern, std::shared_ptr<Html::ElementNode>* result)
    {
        for (uint16 i = 0; i != this->controls.size(); i++)
        {
            UnicodeStringRef name;
            this->controls[i]->get_attribute(Html::globals->name_attribute_name, &name);

            if (equals<UnicodeString, false>(name.get(), pattern.get()))
            {
                (*result) = this->controls[i];
                return true;
            }
        }

        return false;
    }
}