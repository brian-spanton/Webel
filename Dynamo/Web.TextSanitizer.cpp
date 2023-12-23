#include "stdafx.h"
#include "Web.TextSanitizer.h"
#include "Html.Globals.h"

namespace Web
{
	using namespace Basic;

	void TextSanitizer::Initialize(IStream<Codepoint>* destination)
	{
		this->state = State::before_first_word_state;
		this->destination = destination;
	}

	void TextSanitizer::Write(const Codepoint* elements, uint32 count)
	{
		for (uint32 i = 0; i < count; i++)
		{
			Codepoint c = elements[i];

			switch (this->state)
			{
			case State::before_first_word_state:
				if (!Html::globals->WSP(c))
				{
					this->destination->Write(&c, 1);
					this->state = State::in_word_state;
				}
				break;

			case State::in_word_state:
				if (Html::globals->WSP(c))
				{
					this->state = State::before_next_word_state;
				}
				else
				{
					this->destination->Write(&c, 1);
				}
				break;

			case State::before_next_word_state:
				if (!Html::globals->WSP(c))
				{
					Codepoint space = 0x0020;
					this->destination->Write(&space, 1);
					this->destination->Write(&c, 1);
					this->state = State::in_word_state;
				}
				break;
			}
		}
	}

	void TextSanitizer::WriteEOF()
	{
	}
}