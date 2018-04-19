#pragma once
#include <string>

class TestCase
{
public:
	virtual ~TestCase() {}

	virtual std::string get_name() = 0;

	virtual void set_up() = 0;

	virtual bool run() = 0;

	virtual void tear_down() = 0;

	const std::string& get_error()
	{
		return _error;
	}

protected:
	void set_error(std::string error)
	{
		_error = error;
	}

private:
	std::string _error;
};