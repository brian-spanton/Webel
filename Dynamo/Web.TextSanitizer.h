#pragma once

#include "Basic.IStream.h"

namespace Web
{
	using namespace Basic;

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
		Basic::Ref<IStream<Codepoint> > destination; // $$$

	public:
		typedef Basic::Ref<TextSanitizer> Ref;

		void Initialize(IStream<Codepoint>* destination);

		virtual void IStream<Codepoint>::Write(const Codepoint* elements, uint32 count);
		virtual void IStream<Codepoint>::WriteEOF();
	};
}