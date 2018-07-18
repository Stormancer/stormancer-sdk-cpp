#pragma once
#include "Tickable.h"
#include "stormancer/IActionDispatcher.h"

class FStormancerTickDispatcher : public FTickableGameObject, public Stormancer::MainThreadActionDispatcher
{
public:
	FStormancerTickDispatcher();

	// Inherited via FTickableGameObject
	void Tick(float DeltaTime) override;
	bool IsTickable() const override;
	TStatId GetStatId() const override;
	bool IsTickableInEditor() const override;

	// Inherited via MainThreadActionDispatcher
	virtual pplx::task<void> stop() override;

private:
	// TODO fill up?
	TStatId _statId;
};

