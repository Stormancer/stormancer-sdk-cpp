#include "stdafx.h"
#include "P2P/EndpointCandidate.h"

namespace Stormancer
{
	EndpointCandidateType EndpointCandidate::type()
	{
		return (EndpointCandidateType)_type;
	}
};
