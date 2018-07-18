#pragma once
#include <stormancer.h>
#include "PersistenceModels.h"

namespace Stormancer
{

	class PersistenceService
	{
	public:

		PersistenceService(Scene* scene);



		pplx::task<Document> get(std::string type, std::string id);
		
		pplx::task<PutResult> put(Document& doc);
		

		pplx::task<bool> remove(std::string type, std::string id);

		pplx::task<SearchResponse> search(SearchRequest rq);

	private:
		Scene* _scene = nullptr;

	};
}