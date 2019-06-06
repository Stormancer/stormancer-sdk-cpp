#pragma once

#include <string>
#include <vector>
#include <memory>
#include "stormancer/msgpack_define.h"
#include "stormancer/Streams/bytestream.h"
#include "stormancer/Serializer.h"

namespace Stormancer
{
	class Scene;

	struct Message
	{
		Message(std::shared_ptr<Scene> scene, std::string sourceId, ibytestream& stream);

		std::shared_ptr<Scene> scene;
		std::string sourceId;
		ibytestream& stream;

		/// Read a serialized object
		template<typename TOut>
		TOut readObject()
		{
			Serializer serializer;
			return serializer.deserializeOne<TOut>(stream);
		}

		/// Read many serialized objects
		template<typename... Args>
		void readObjects(Args&... args)
		{
			Serializer serializer;
			serializer.deserialize(stream, args...);
		}
	};
}

