#pragma once

#include "TestCase.h"
#include "stormancer/stormancer.h"

class TestRunner
{
public:
	TestRunner(Stormancer::ILogger_ptr logger);

	bool run_tests();
	
private:
	std::vector<std::unique_ptr<TestCase>> _tests;
	std::unordered_map<std::string, std::string> _errors;

	Stormancer::ILogger_ptr _logger;
};
