#pragma once
#include "headers.h"

namespace Stormancer
{
	//Dispatches client actions to the appropriate thread.
	//The default dispatcher execute client actions on the current network engine thread.
	//Developers should use an instance of the MainThreadActionDispatcher to dispatch client actions to the game main thread.
	class IActionDispatcher
	{
	public:
		virtual void post(const std::function<void(void)> action) = 0;

		virtual void start() = 0;
		virtual void stop() = 0;

		virtual bool isRunning() = 0;
	};

	class SameThreadActionDispatcher : public IActionDispatcher
	{
	public:
		virtual void post(const std::function<void(void)> action);

		virtual void start();
		virtual void stop();
		virtual bool isRunning();

	private:
		bool _isRunning;
	};

	class MainThreadActionDispatcher :public IActionDispatcher
	{
	public:
		virtual void post(const std::function<void(void)> action);

		virtual void start();
		virtual void stop();
		virtual bool isRunning();

	public:
		//Runs the actions in the current thread.
		//@maxDuration: Max duration the update method should run (it can take more time, but as soon as  maxDuration is exceeded, update will return as soon as the
		//current executing action returns.)
		void update(uint32 maxDuration);

	private:
		bool _isRunning;
		bool _stopRequested;
		std::queue<std::function<void(void)>> _actions;
	};
}