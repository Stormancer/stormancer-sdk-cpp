#pragma once


namespace Stormancer
{
	/// Forward declare
	struct RecoverableGame;

	class GameRecovery
	{
	public:
		virtual pplx::task<std::shared_ptr<RecoverableGame>> getCurrent() = 0;

		virtual pplx::task<void> cancelCurrent() = 0;
	};
}