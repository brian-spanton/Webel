#include "stdafx.h"
#include "Dynamo.HeapStorage.h"
#include "Basic.Event.h"
#include "Basic.ICompletion.h"
#include "Dynamo.Globals.h"

namespace Dynamo
{
	using namespace Basic;

	void HeapStorage::GetNamespace(UnicodeString::Ref name_space, Basic::Ref<Namespace>* item)
	{
		Basic::Ref<Namespace> name_space_map = New<Namespace>();

		Namespaces::iterator it = this->name_spaces.find(name_space);
		if (it == this->name_spaces.end())
		{
			name_space_map = New<Namespace>();
			this->name_spaces.insert(Namespaces::value_type(name_space, name_space_map));
		}
		else
		{
			name_space_map = it->second;
		}

		(*item) = name_space_map;
	}

	void HeapStorage::Store(UnicodeString::Ref name_space, UnicodeString::Ref name, Basic::Ref<IStream<byte> >* stream)
	{
		Basic::Ref<Namespace> name_space_map = New<Namespace>();
		GetNamespace(name_space, &name_space_map);

		ByteString::Ref data;

		Namespace::iterator it2 = name_space_map->find(name);
		if (it2 == name_space_map->end())
		{
			data = New<ByteString>();
			name_space_map->insert(Namespace::value_type(name, data));
		}
		else
		{
			data = it2->second;
			data->clear();
		}

		(*stream) = data;
	}

	class HeapStorageFind : public IEvent, public ICompletion
	{
	public:
		typedef Basic::Ref<HeapStorageFind> Ref;

		Basic::Ref<Namespace> name_space;
		UnicodeString::Ref query;
		Basic::Ref<IProcess> results;
		ByteString::Ref result;

		virtual uint32 get_type()
		{
			return 0x1000;
		}

		virtual void ICompletion::CompleteAsync(OVERLAPPED_ENTRY& entry)
		{
			Namespace::iterator it2 = name_space->lower_bound(query);
			if (it2 != name_space->end())
			{
				while (true)
				{
					if (!it2->first->starts_with<true>(query))
						break;

					this->result = it2->second;
					results->Process(this);
				};
			}

			ElementStreamEndingEvent event;
			results->Process(&event);
		}
	};

	void HeapStorage::Find(UnicodeString::Ref name_space, UnicodeString::Ref query, Basic::Ref<IProcess> results)
	{
		Basic::Ref<Namespace> name_space_map = New<Namespace>();
		GetNamespace(name_space, &name_space_map);

		HeapStorageFind::Ref result = New<HeapStorageFind>();
		result->name_space = name_space_map;
		result->query = query;
		result->results = results;

		Dynamo::globals->PostCompletion(result, 0);
	}
}

namespace Basic
{
	template <>
	bool Event::ReadNext(IEvent* event, ByteString::Ref* element, bool* yield)
	{
		if (event->get_type() == 0x1000)
		{
			Dynamo::HeapStorageFind* read_event = (Dynamo::HeapStorageFind*)event;
			(*element) = read_event->result;

			(*yield) = true;
			return true;
		}

		(*yield) = true;
		return false;
	}
}
