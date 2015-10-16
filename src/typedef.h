#pragma once

#define Packet_ptr std::shared_ptr<Packet<>>
#define Packetisp_ptr std::shared_ptr<Packet<IScenePeer>>
#define SystemRequest_ptr std::shared_ptr<SystemRequest>
#define Route_ptr std::shared_ptr<Route>
#define Scene_ptr std::shared_ptr<Scene>
#define Scene_wptr std::weak_ptr<Scene>
#define Subscription_ptr std::shared_ptr<Subscription>
#define Client_ptr std::shared_ptr<Client>
#define Client_wptr std::weak_ptr<Client>

#define PacketObservable rxcpp::observable<Packet_ptr>

#define handlerFunction std::function<bool(Packet_ptr)>
#define processorFunction std::function<bool(byte, Packet_ptr)>

#define RpcRequest_ptr std::shared_ptr<RpcRequest>
#define RpcRequestContex_ptr std::shared_ptr<RpcRequestContext<IScenePeer>>

namespace Stormancer
{
	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;

	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;

	using byte = uint8;

	using stringMap = std::map < std::string, std::string >;
	using anyMap = std::map < std::string, void* >;
};
