#include "stdafx.h"
#include "PersistenceService.h"

pplx::task<bool> Stormancer::PersistenceService::remove(std::string type, std::string id)
{
	auto rpc = _scene->dependencyResolver()->template resolve<RpcService>();
	return rpc->rpc<bool>("persistence.delete", type, id);
}

pplx::task<Stormancer::SearchResponse> Stormancer::PersistenceService::search(SearchRequest rq)
{
	auto rpc = _scene->dependencyResolver()->template resolve<RpcService>();
	return rpc->rpc<SearchResponse>("persistence.search",rq);
}

Stormancer::PersistenceService::PersistenceService(Scene * scene) :
	_scene(scene)
{
}

pplx::task < Stormancer::Document > Stormancer::PersistenceService::get(std::string type, std::string id)
{

	std::shared_ptr<RpcService> rpc = _scene->dependencyResolver()->template resolve<RpcService>();
	return rpc->rpc <Document>("persistence.get", type, id);

}

pplx::task<Stormancer::PutResult> Stormancer::PersistenceService::put(Document & doc)
{
	auto rpc = _scene->dependencyResolver()->template resolve<RpcService>();
	return rpc->rpc<PutResult>("persistence.put", doc);
}
