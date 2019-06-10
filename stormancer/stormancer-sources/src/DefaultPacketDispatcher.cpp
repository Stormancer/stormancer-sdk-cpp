#include "stormancer/stdafx.h"
#include "stormancer/DefaultPacketDispatcher.h"
#include "stormancer/Logger/ILogger.h"
#include "stormancer/Helpers.h"

namespace Stormancer
{
	DefaultPacketDispatcher::DefaultPacketDispatcher(ILogger_ptr logger, bool asyncDispatch)
		: _asyncDispatch(asyncDispatch)
		, _logger(logger)
		, _maxLayerCount(40)
	{
	}

	DefaultPacketDispatcher::~DefaultPacketDispatcher()
	{
		_handlers.clear();
		_defaultProcessors.clear();
	}

	void DefaultPacketDispatcher::dispatchPacket(Packet_ptr packet)
	{
		if (_asyncDispatch)
		{
			pplx::create_task([this, packet]()
			{
				dispatchImpl(packet);
			})
				.then([this](pplx::task<void> t)
			{
				try
				{
					t.wait();
				}
				catch (const std::exception& ex)
				{
					_logger->log(LogLevel::Error, "client.dispatchPacket", "Unhandled exception in dispatchPacketImpl" + std::string(ex.what()));
				}
			});
		}
		else
		{
			try
			{
				dispatchImpl(packet);
			}
			catch (const std::exception& ex)
			{
				_logger->log(LogLevel::Error, "client.dispatchPacket", "Unhandled exception in dispatchPacketImpl" + std::string(ex.what()));
			}
		}
	}

	void DefaultPacketDispatcher::dispatchImpl(Packet_ptr packet)
	{
		bool processed = false;
		int count = 0;
		byte msgType = 0;

		while (!processed && count < _maxLayerCount)
		{
			packet->stream >> msgType;
			auto it = _handlers.find(msgType);

			if (it != _handlers.end())
			{
				processed = it->second(packet);
				count++;
			}
			else
			{
				break;
			}
		}

		for (auto& processor : _defaultProcessors)
		{
			if (processor(msgType, packet))
			{
				processed = true;
				break;
			}
		}

		if (!processed)
		{
			_logger->log(LogLevel::Warn, "DefaultPacketDispatcher", "Couldn't process message", "msgId: " + std::to_string(msgType));
			auto bytes = stringifyBytesArray(packet->stream.bytes(), true, true);
			_logger->log(LogLevel::Trace, "DefaultPacketDispatcher", "Message contents: ", bytes.c_str());
		}
	}

	void DefaultPacketDispatcher::addProcessor(std::shared_ptr<IPacketProcessor> processor)
	{
		PacketProcessorConfig config(_handlers, _defaultProcessors);
		processor->registerProcessor(config);
	}
}
