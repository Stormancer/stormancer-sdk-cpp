#pragma once
#include "headers.h"

namespace Stormancer
{
	class IMetadata
	{
	public:
		IMetadata();
		virtual ~IMetadata();
	};


	class StringMetadata : public IMetadata
	{
	public:
		StringMetadata();
		StringMetadata(std::string);
		~StringMetadata();

	public:
		std::string data;
	};


	class PtrMetadata : public IMetadata
	{
	public:
		PtrMetadata();
		PtrMetadata(void* dataPtr);
		~PtrMetadata();

	public:
		void* data = nullptr;
	};


	template<typename T>
	class OwningPtrMetadata : public IMetadata
	{
	public:
		OwningPtrMetadata()
		{
		}

		OwningPtrMetadata(T* dataPtr)
			: data(dataPtr)
		{
		}

		OwningPtrMetadata(OwningPtrMetadata& right)
			: data()
		{

		}

		~OwningPtrMetadata()
		{
			delete data;
		}

	public:
		T* data = nullptr;

	private:
		uint32 _count = 1;
	};
};
