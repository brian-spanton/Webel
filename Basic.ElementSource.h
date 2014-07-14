// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IElementSource.h"
#include "Basic.Globals.h"
#include "Basic.Frame.h"

namespace Basic
{
    // $$ get rid of this class?
    template <class T>
    class ElementSource : public IElementSource<T>
    {
    private:
        typedef std::shared_ptr<IStream<T> > StreamRef;
        typedef std::map<IStream<T>*, StreamRef> StreamMap; // $$ is a map best with shared_ptr, or can we use some other bag and not need the pointer key?

        const T* elements;
        uint32 count;
        uint32 elements_read;

        StreamMap observers;
        uint32 elements_observed;

        void Observe()
        {
            if (this->elements_observed >= this->elements_read)
                return;

            uint32 observing = this->elements_read - this->elements_observed;

            for (StreamMap::iterator it = this->observers.begin(); it != this->observers.end(); it++)
            {
                it->second->write_elements(this->elements + this->elements_observed, observing);
            }

            this->elements_observed += observing;
        }

    public:
        void Initialize(const T* elements, uint32 count)
        {
            this->elements = elements;
            this->count = count;
            this->elements_read = 0;
            this->elements_observed = 0;
        }

        bool Exhausted()
        {
            return (this->elements_read == this->count);
        }

        void Read(uint32 count, const T** out_address, uint32* out_count)
        {
            if (count == 0)
                throw FatalError("Basic::ElementSource::Read count == 0");

            uint32 elements_remaining = this->count - this->elements_read;

            if (elements_remaining == 0)
                throw Yield("event consumed");

            const T* return_address = this->elements + this->elements_read;
            uint32 return_count = (elements_remaining < count) ? elements_remaining : count;

            this->elements_read += return_count;

            Observe();

            (*out_address) = return_address;
            (*out_count) = return_count;
        }

        void ReadNext(T* element)
        {
            uint32 elements_remaining = this->count - this->elements_read;

            if (elements_remaining == 0)
                throw Yield("event consumed");

            (*element) = this->elements[this->elements_read];
            this->elements_read++;

            Observe();
        }

        void UndoReadNext()
        {
            if (this->elements_read == 0)
                throw FatalError("Basic::ElementSource::UndoReadNext");

            this->elements_read--;
        }

        void AddObserver(std::shared_ptr<IStream<T> > stream)
        {
            this->observers.insert(StreamMap::value_type(stream.get(), stream));
        }

        void RemoveObserver(std::shared_ptr<IStream<T> > stream)
        {
            this->observers.erase(stream.get());
        }
    };
}