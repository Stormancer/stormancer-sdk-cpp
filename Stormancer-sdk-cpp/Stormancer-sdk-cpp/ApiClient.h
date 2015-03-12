#pragma once

#include <iostream>

class Configuration{};
class ITokenHandler{};

class ApiClient
{
public:
	ApiClient(Configuration config, ITokenHandler tokenHandler);
	~ApiClient();

private:
	Configuration _config;
	const std::string _createTokenUri;
	const ITokenHandler _tokenHandler;
};
