#pragma once

#include <string>

class StormancerResultBase
{
public:
	StormancerResultBase(bool Success) :
		_success(Success)
	{
	}

	virtual ~StormancerResultBase() = 0;

	bool Success() const
	{
		return _success;
	}

	void SetError(const char* reason)
	{
		_success = false;
		_reason = reason;
	}

	std::string Reason() const
	{
		return _reason;
	}

protected:
	bool _success = false;
	std::string _reason;
};

inline StormancerResultBase::~StormancerResultBase()
{
}

template<typename T = void>
class StormancerResult : public StormancerResultBase
{
public:
	StormancerResult() :
		StormancerResultBase(false)
	{
	}

	StormancerResult(T data) :
		StormancerResultBase(true)
	{
		Set(data);
	}

	void Set(T data)
	{
		_data = data;
		_success = true;
	}

	T Get() const
	{
		// Cannot call Get() on an unset result
		if (!_success)
		{
			std::terminate();
		}
		return _data;
	}

private:
	T _data;
};

template<>
class StormancerResult<void> : public StormancerResultBase
{
public:
	StormancerResult(bool Success = false) :
		StormancerResultBase(Success)
	{
	}

	void Set()
	{
		_success = true;
	}
};
