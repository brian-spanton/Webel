#include "stdafx.h"

namespace Basic
{
	typedef std::unordered_set<IRefCounted*> Objects;
	typedef std::unordered_map<IRefCounted*, std::string> Names;

	Lock lock;
	Objects objects;
	Names names;
	Objects baseline;

	void register_object(IRefCounted* object)
	{
		Hold hold(lock);

		Objects::_Pairib result = objects.insert(object);

		if (result.second == false)
			throw new Exception("tried to register an already registered object");
	}

	void set_object_name(IRefCounted* object, const char* name)
	{
		Hold hold(lock);

		names.insert(Names::value_type(object, name));
	}

	void unregister_object(IRefCounted* object)
	{
		Hold hold(lock);

		int count = objects.erase(object);

		if (count == 0)
			throw new Exception("tried to unregister an unregistered object");

		if (count > 1)
			throw new Exception("unregistered more than one object");

		names.erase(object);
	}

	void set_objects_baseline()
	{
		Hold hold(lock);

		baseline = objects;
	}

	void check_new_objects()
	{
		Hold hold(lock);

		Objects leaked = objects;

		for (Objects::iterator baseline_it = baseline.begin(); baseline_it != baseline.end(); baseline_it++)
		{
			leaked.erase(*baseline_it);
		}

		for (Objects::iterator leaked_it = leaked.begin(); leaked_it != leaked.end(); leaked_it++)
		{
			IRefCounted* object = (*leaked_it);

			Names::iterator names_it = names.find(object);
		}
	}
}