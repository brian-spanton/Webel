// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
	class TextSanitizer : public IStream<Codepoint>
	{
	private:
		enum State
		{
			before_first_word_state,
			in_word_state,
			before_next_word_state,
		};

		State state;
		Basic::Ref<IStream<Codepoint> > destination; // REF

		bool white_space(Codepoint c);

	public:
		typedef Basic::Ref<TextSanitizer> Ref;

		void Initialize(IStream<Codepoint>* destination);

		virtual void IStream<Codepoint>::Write(const Codepoint* elements, uint32 count);
		virtual void IStream<Codepoint>::WriteEOF();
	};
}