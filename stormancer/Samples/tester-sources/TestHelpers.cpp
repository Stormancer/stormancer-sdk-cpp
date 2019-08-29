#include "TestHelpers.h"
#include "stormancer/Tasks.h"
#include "stormancer/Utilities/TaskUtilities.h"

namespace TestHelpers
{
	void failAfterTimeout(pplx::task_completion_event<void> tce, std::string msg, std::chrono::milliseconds timeout)
	{
		failAfterTimeout(tce, [msg] { return msg; }, timeout);
	}

	void failAfterTimeout(pplx::task_completion_event<void> tce, std::function<std::string()> msgBuilder, std::chrono::milliseconds timeout)
	{
		auto timeoutCt = Stormancer::timeout(timeout);
		timeoutCt.register_callback([tce, msgBuilder]
		{
			tce.set_exception(std::runtime_error(msgBuilder().c_str()));
		});
	}
}