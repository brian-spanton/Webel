#pragma once

#include "Basic.INumberStream.h"

namespace Basic
{
	template <class character_type, class number_type>
	class HexNumberStream : public INumberStream<character_type>
	{
	private:
		number_type* value;
		uint8 count;

	public:
		typedef Basic::Ref<HexNumberStream<character_type, number_type>, INumberStream<character_type> > Ref;

		void Initialize(number_type* value)
		{
			this->count = 0;
			this->value = value;
		}

		uint8 get_digit_count()
		{
			return this->count;
		}

		virtual bool INumberStream<character_type>::WriteDigit(character_type digit)
		{
			byte digit_value;

			bool success = base_16<character_type>(digit, &digit_value);
			if (!success)
				return false;

			if (this->count == 0)
			{
				(*this->value) = 0;
			}
			else
			{
				(*this->value) *= 0x10;
			}

			this->count += 1;
			(*this->value) += digit_value;
			return true;
		}
	};
}
