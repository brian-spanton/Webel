#include "stdafx.h"
#include "Json.Types.h"
#include "Basic.FrameStream.h"
#include "Json.Globals.h"

namespace Json
{
	using namespace Basic;

	void Token::GetDebugString(Type type, char* debug_string, int count)
	{
		switch (type)
		{
#define CASE(e) \
		case e: \
			strcpy_s(debug_string, count, #e); \
			break

			CASE(begin_array_token);
			CASE(begin_object_token);
			CASE(end_array_token);
			CASE(end_object_token);
			CASE(name_separator_token);
			CASE(value_separator_token);
			CASE(string_token);
			CASE(number_token);
			CASE(bool_token);
			CASE(null_token);

#undef CASE

		default:
			throw new Exception("Token::GetDebugString unhandled token type");
		}
	}

	uint32 ReadyForReadTokenPointerEvent::get_type()
	{
		return Json::EventType::ready_for_read_token_pointer_event;
	}

	void ReadyForReadTokenPointerEvent::Initialize(IElementSource<Token::Ref>* element_source)
	{
		this->element_source = element_source;
	}

	bool ReadyForReadTokenPointerEvent::ReadNext(IEvent* event, Token::Ref* element, bool* yield)
	{
		if (event->get_type() == Json::EventType::ready_for_read_token_pointer_event)
		{
			ReadyForReadTokenPointerEvent* read_event = (ReadyForReadTokenPointerEvent*)event;
			return read_event->element_source->ReadNext(element, yield);
		}

		(*yield) = true;
		return false;
	}

	void ReadyForReadTokenPointerEvent::UndoReadNext(IEvent* event)
	{
		if (event->get_type() == Json::EventType::ready_for_read_token_pointer_event)
		{
			ReadyForReadTokenPointerEvent* read_event = (ReadyForReadTokenPointerEvent*)event;
			return read_event->element_source->UndoReadNext();
		}

		throw new Exception("ReadyForReadTokenPointerEvent::UndoReadNext unexpected undo during non-read event");
	}

	void TokenVector::Write(const Token::Ref* elements, uint32 count)
	{
		this->insert(this->end(), elements, elements + count);
	}

	void TokenVector::WriteEOF()
	{
	}

	void TokenVector::write_to(IStream<Token::Ref>* dest)
	{
		if (this->size() > 0)
		{
			dest->Write(&this->front(), this->size());
		}
	}

	void Array::write_to(Basic::IStream<Codepoint>* stream)
	{
		TextWriter writer(stream);

		bool has_complex = false;

		for (ValueList::iterator it = this->elements.begin(); it != this->elements.end(); it++)
		{
			if ((*it)->type == Value::Type::array_value
				|| (*it)->type == Value::Type::object_value)
			{
				has_complex = true;
			}
		}

		stream->Write(&Json::globals->begin_array, 1);

		if (this->elements.size() > 0)
		{
			if (!has_complex)
			{
				writer.Write(" ");

				for (ValueList::iterator it = this->elements.begin(); it != this->elements.end(); it++)
				{
					if (it != this->elements.begin())
					{
						stream->Write(&Json::globals->value_separator, 1);
						writer.Write(" ");
					}

					(*it)->write_to(stream);
				}

				writer.Write(" ");
			}
			else
			{
				// $ stream->Indent();
				writer.WriteLine();

				for (ValueList::iterator it = this->elements.begin(); it != this->elements.end(); it++)
				{
					if (it != this->elements.begin())
					{
						stream->Write(&Json::globals->value_separator, 1);
						writer.WriteLine();
					}

					(*it)->write_to(stream);
				}

				// $ stream->Unindent();
			}
		}

		stream->Write(&Json::globals->end_array, 1);
	}

	void Object::write_to(Basic::IStream<Codepoint>* stream)
	{
		TextWriter writer(stream);

		bool has_complex = false;

		for (MemberList::iterator it = this->members.begin(); it != this->members.end(); it++)
		{
			if (it->second->type == Value::Type::array_value
				|| it->second->type == Value::Type::object_value)
			{
				has_complex = true;
			}
		}

		stream->Write(&Json::globals->begin_object, 1);

		if (this->members.size() > 0)
		{
			if (!has_complex)
			{
				writer.Write(" ");

				for (MemberList::iterator it = this->members.begin(); it != this->members.end(); it++)
				{
					if (it != this->members.begin())
					{
						stream->Write(&Json::globals->value_separator, 1);
						writer.Write(" ");
					}

					Json::String::write_value(it->first, stream);
					writer.Write(" ");
					stream->Write(&Json::globals->name_separator, 1);
					writer.Write(" ");
					it->second->write_to(stream);
				}

				writer.Write(" ");
			}
			else
			{
				// $ stream->Indent();
				writer.WriteLine();

				for (MemberList::iterator it = this->members.begin(); it != this->members.end(); it++)
				{
					if (it != this->members.begin())
					{
						stream->Write(&Json::globals->value_separator, 1);
						writer.WriteLine();
					}

					Json::String::write_value(it->first, stream);
					writer.Write(" ");
					stream->Write(&Json::globals->name_separator, 1);
					writer.Write(" ");
					it->second->write_to(stream);
				}

				// $ stream->Unindent();
			}
		}

		stream->Write(&Json::globals->end_object, 1);
	}

	void Number::write_to(Basic::IStream<Codepoint>* stream)
	{
		TextWriter writer(stream);

		// $$ handle exponents and fractions
		writer.WriteFormat<64>("%d", (uint64)this->value);
	}

	void String::write_to(Basic::IStream<Codepoint>* stream)
	{
		write_value(this->value, stream);
	}

	void String::write_value(Basic::UnicodeString::Ref value, Basic::IStream<Codepoint>* stream)
	{
		Codepoint quote = '\"';
		stream->Write(&quote, 1);
		// $$ escape special chars (for instance " )
		value->write_to(stream);
		stream->Write(&quote, 1);
	}

	void Bool::write_to(Basic::IStream<Codepoint>* stream)
	{
		UnicodeString::Ref string = this->value ? Json::globals->json_true : Json::globals->json_false;
		string->write_to(stream);
	}

	void Null::write_to(Basic::IStream<Codepoint>* stream)
	{
		Json::globals->json_null->write_to(stream);
	}
}

namespace Basic
{
	void FrameStream<Json::Token::Ref>::Write(const Json::Token::Ref* elements, uint32 count)
	{
		this->element_source.Initialize(elements, count);

		Json::ReadyForReadTokenPointerEvent event;
		event.Initialize(&this->element_source);

		this->frame->Process(&event);
	}
}
