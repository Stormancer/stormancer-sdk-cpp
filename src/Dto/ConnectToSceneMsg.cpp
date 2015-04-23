#include "stormancer.h"

namespace Stormancer
{
	ConnectToSceneMsg::ConnectToSceneMsg()
	{
	}

	ConnectToSceneMsg::ConnectToSceneMsg(bytestream* stream)
	{
		deserialize(stream);
	}

	ConnectToSceneMsg::~ConnectToSceneMsg()
	{
	}

	void ConnectToSceneMsg::serialize(bytestream* stream)
	{
		// TODO
	}

	void ConnectToSceneMsg::deserialize(bytestream* stream)
	{
		// TODO
	}
}
