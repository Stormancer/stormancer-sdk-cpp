#include "TestRunner.h"
#include "TestPlayerDataPlugin.h"

TestRunner::TestRunner(Stormancer::ILogger_ptr logger)
	: _logger(logger)
{
	// I cannot list-initialize the vector as it requires a copy ctor for elements
	_tests.emplace_back(new TestPlayerDataPlugin);
}

bool TestRunner::run_tests()
{
	for (const auto& test : _tests)
	{
		_logger->log(Stormancer::LogLevel::Info, "Tester", "Running test", test->get_name());

		// TODO this is not flexible enough
		test->set_up(TestParameters{});
		if (test->run())
		{
			_logger->log(Stormancer::LogLevel::Info, "Tester", "Test PASSED", test->get_name());
		}
		else
		{
			_logger->log(Stormancer::LogLevel::Error, "Tester", "Test FAILED", test->get_name());
			_logger->log(Stormancer::LogLevel::Error, "Tester", "Reason", test->get_error());
			_errors[test->get_name()] = test->get_error();
		}
		test->tear_down();
	}

	size_t num_tests = _tests.size();
	size_t num_failed = _errors.size();
	size_t num_passed = num_tests - num_failed;
	_logger->log(Stormancer::LogLevel::Info, "Tester", "SUMMARY");
	_logger->log(Stormancer::LogLevel::Info, "Tetser", "PASSED", std::to_string(num_passed) + "/" + std::to_string(num_tests));

	if (num_failed > 0)
	{
		_logger->log(Stormancer::LogLevel::Info, "Tester", "FAILED", std::to_string(num_failed));
		for (const auto& error : _errors)
		{
			_logger->log(Stormancer::LogLevel::Error, "Tester", error.first, error.second);
		}
		return false;
	}

	return true;
}