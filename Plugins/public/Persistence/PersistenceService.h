#pragma once

#include "stormancer/Scene.h"
#include "PersistenceModels.h"

namespace Stormancer
{

	class PersistenceService
	{
	public:

		PersistenceService(std::shared_ptr<Scene> scene);



		pplx::task<Document> get(std::string type, std::string id);
		
		pplx::task<PutResult> put(Document& doc);
		

		pplx::task<bool> remove(std::string type, std::string id);

		pplx::task<SearchResponse> search(SearchRequest rq);

	private:
		std::weak_ptr<Scene> _scene;

	};
}