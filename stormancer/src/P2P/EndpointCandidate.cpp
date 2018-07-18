#include "stormancer/stdafx.h"
#include "stormancer/P2P/EndpointCandidate.h"

namespace Stormancer
{
	EndpointCandidateType EndpointCandidate::type()
	{
		return (EndpointCandidateType)_type;
	}
};
