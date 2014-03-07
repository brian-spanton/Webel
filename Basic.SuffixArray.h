// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
	template <class value_type>
	class SuffixArray : public IRefCounted
	{
	private:
		struct SortEntry
		{
			Codepoint sort_key[2];
			uint32 key_index;
		};

		struct ArrayEntry
		{
			UnicodeString::Ref key; // REF
			uint32 key_index;
			value_type value;

			ArrayEntry()
			{
			}

			ArrayEntry(const ArrayEntry& entry)
			{
				this->key = entry.key;
				this->key_index = entry.key_index;
				this->value = entry.value;
			}
		};

		typedef std::vector<Codepoint> WorkingArray;
		typedef std::vector<ArrayEntry> ResultArray;

		static int compare(struct SortEntry a, struct SortEntry b)
		{
			if (a.sort_key[0] == b.sort_key[0])
			{
				return (a.sort_key[1] < b.sort_key[1] ? 1 : 0);
			}
			else
			{
				return (a.sort_key[0] < b.sort_key[0] ? 1 : 0);
			}
		}

		uint32 find(uint32 begin, uint32 end, UnicodeString::Ref key, uint32 key_index)
		{
			Hold hold(this->lock);

			while(begin < end)
			{
				uint32 mid = begin + ((end - begin) / 2);

				int result = compare_strings<Codepoint, false>(
					key->c_str() + key_index,
					key->size() - key_index,
					this->results[mid].key->c_str() + this->results[mid].key_index,
					this->results[mid].key->size() - this->results[mid].key_index);

				if (result == 1)
					begin = mid + 1;
				else
					end = mid;
			}

			return begin;
		}

		uint32 insert(uint32 begin, uint32 end, UnicodeString::Ref key, uint32 key_index, value_type value)
		{
			ArrayEntry entry;
			entry.key_index = key_index;
			entry.key = key;
			entry.value = value;

			uint32 before = find(begin, end, key, key_index);

			this->results.insert(this->results.begin() + before, entry);

			return before + 1;
		}

	public:
		typedef Basic::Ref<SuffixArray<value_type> > Ref;

		Lock lock; // $ think about reader/writer or other strategies for better concurrency
		ResultArray results;

		void WriteDebug()
		{
			Hold hold(this->lock);

			for (uint32 i = 0; i < this->results.size(); i++)
			{
				Basic::globals->DebugStream()->Write(
					this->results[i].key->c_str() + this->results[i].key_index,
					this->results[i].key->size() - this->results[i].key_index);
				Basic::globals->DebugWriter()->WriteLine();
			}
		}

		void Search(UnicodeString::Ref term, uint32* begin, uint32* end)
		{
			Hold hold(this->lock);

			uint32 before = find(0, this->results.size(), term, 0);

			(*begin) = before;

			while (before < this->results.size())
			{
				bool result = starts_with<Codepoint, false>(
					this->results[before].key->c_str() + this->results[before].key_index,
					this->results[before].key->size() - this->results[before].key_index,
					term->c_str(),
					term->size());

				if (!result)
					break;

				before++;
			}

			(*end) = before;
		}

		void Add(UnicodeString::Ref key, value_type value)
		{
			uint32 key_count = key->size();

			WorkingArray working_array[2];
			working_array[0].resize(key_count);
			working_array[1].resize(key_count);

			for (uint32 i = 0; i < key_count; i++)
			{
				working_array[0][i] = Basic::lower_case(key->at(i));
			}

			bool previous_storage_at_index_zero = true;
			uint32 sort_key_separation = 1;

			while (true)
			{
				WorkingArray& current_array = working_array[previous_storage_at_index_zero ? 1 : 0];
				WorkingArray& previous_array = working_array[previous_storage_at_index_zero ? 0 : 1];

				std::vector<SortEntry> sort_keys;
				sort_keys.resize(key_count);

				for (uint32 i = 0; i < key_count; i++)
				{
					sort_keys[i].sort_key[0] = previous_array[i];

					uint32 secondary_sort_key = i + sort_key_separation;

					if (secondary_sort_key < key_count)
					{
						sort_keys[i].sort_key[1] = previous_array[secondary_sort_key];
					}
					else
					{
						sort_keys[i].sort_key[1] = EOF;
					}

					sort_keys[i].key_index = i;
				}

				std::sort(sort_keys.begin(), sort_keys.end(), compare);

				if (sort_key_separation >= key_count)
				{
					Hold hold(this->lock);

					this->results.reserve(this->results.size() + key_count);

					uint32 begin = 0;
					uint32 end = this->results.size();

					for (uint32 i = 0; i < key_count; i++)
					{
						begin = insert(begin, end, key, sort_keys[i].key_index, value);
						end++;
					}

					break;
				}

				for (uint32 i = 0; i < key_count; i++)
				{
					if (i > 0
						&& sort_keys[i].sort_key[0] == sort_keys[i - 1].sort_key[0]
						&& sort_keys[i].sort_key[1] == sort_keys[i - 1].sort_key[1])
					{
						// equivalent sort keys must have equivalent rank for next round of sorting
						current_array[sort_keys[i].key_index] = current_array[sort_keys[i - 1].key_index];
					}
					else
					{
						current_array[sort_keys[i].key_index] = i;
					}
				}

				previous_storage_at_index_zero = !previous_storage_at_index_zero;
				sort_key_separation <<= 1;
			}
		}
	};
}