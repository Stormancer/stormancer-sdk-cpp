#pragma once

#include "stormancer/BuildConfig.h"


#include "stormancer/IPacketProcessor.h"
#include "stormancer/IActionDispatcher.h"
#include "stormancer/SceneImpl.h"

namespace Stormancer
{
	/// Dispatch the network messages to the right scene.
	class SceneDispatcher : public IPacketProcessor
	{
		friend class Client;

	public:

#pragma region public_methods

		SceneDispatcher(std::shared_ptr<IActionDispatcher> evDispatcher);
		virtual ~SceneDispatcher();

		void registerProcessor(PacketProcessorConfig& config);
		void addScene(std::shared_ptr<IConnection> connection, Scene_ptr scene);
		void removeScene(std::shared_ptr<IConnection> connection, uint8 sceneHandle);

#pragma endregion

	private:

#pragma region private_methods

		bool handler_impl(uint8 sceneHandle, Packet_ptr packet);

#pragma endregion

#pragma region private_members

		std::shared_ptr<IActionDispatcher> _eventDispatcher;
		processorFunction* handler = nullptr;


#pragma endregion
	};
};
