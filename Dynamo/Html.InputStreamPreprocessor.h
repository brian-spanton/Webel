#pragma once

#include "Html.Types.h"
#include "Html.Node.h"
#include "Html.Token.h"

namespace Html
{
	using namespace Basic;

	class Parser;

	class InputStreamPreprocessor : public IStream<Codepoint>
	{
	private:
		enum State
		{
			bom_state,
			normal_state,
			ignore_lf_state,
		};

		State state;
		Basic::Ref<IStream<Codepoint> > output; // $$$
		Parser* parser;

		bool IsValid(Codepoint c);

	public:
		typedef Basic::Ref<InputStreamPreprocessor, IStream<Codepoint> > Ref;

		void Initialize(Parser* parser, IStream<Codepoint>* output);

		virtual void IStream<Codepoint>::Write(const Codepoint* elements, uint32 count);
		virtual void IStream<Codepoint>::WriteEOF();
	};
}