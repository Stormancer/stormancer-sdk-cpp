#include "StormancerPluginPrivatePCH.h"
#include "StormancerTickDispatcher.h"


FStormancerTickDispatcher::FStormancerTickDispatcher() :
	MainThreadActionDispatcher(),
	_statId()
{
}

void FStormancerTickDispatcher::stop()
{
	MainThreadActionDispatcher::stop();
	// MainThreadActionDispatcher needs a last call tu update() before stopping
	MainThreadActionDispatcher::update(std::chrono::milliseconds(1));
}

void FStormancerTickDispatcher::Tick(float DeltaTime)
{
	MainThreadActionDispatcher::update(std::chrono::milliseconds(1));
}

bool FStormancerTickDispatcher::IsTickable() const
{
	return true;
}

TStatId FStormancerTickDispatcher::GetStatId() const
{
	return _statId;
}

bool FStormancerTickDispatcher::IsTickableInEditor() const
{
	return false;
}
