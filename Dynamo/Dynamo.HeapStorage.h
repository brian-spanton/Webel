#pragma once

#include "Dynamo.IStorage.h"

namespace Dynamo
{
	using namespace Basic;

	class AdminProtocol;

	template <typename value_type>
	class DataMap : public std::map<UnicodeString::Ref, value_type>, public IRefCounted
	{
	public:
		// $$ typedef Basic::Ref<DataMap<value_type> > Ref;
	};

	typedef DataMap<ByteString::Ref> Namespace; // $$$

	class HeapStorage : public IStorage
	{
		friend class AdminProtocol;

	public:
		typedef StringMapCaseSensitive<Basic::Ref<Namespace> > Namespaces;

	private:
		Namespaces name_spaces;

		void GetNamespace(UnicodeString::Ref name_space, Basic::Ref<Namespace>* item);

	public:
		virtual void IStorage::Store(UnicodeString::Ref name_space, UnicodeString::Ref name, Basic::Ref<IStream<byte> >* stream);
		virtual void IStorage::Find(UnicodeString::Ref name_space, UnicodeString::Ref query, Basic::Ref<IProcess> results);
	};
}
