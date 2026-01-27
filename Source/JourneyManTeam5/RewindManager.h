// Code by Jack Adams - Created 25/01/26
// Lasted editied - 25/01/26

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "RewindComponent.h"
#include "RewindManager.generated.h"

UCLASS()
class JOURNEYMANTEAM5_API URewindManager : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    // Called when the subsystem is created
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    // Must return true for Tick to be called
    virtual bool IsTickable() const override { return true; }

    virtual bool IsTickableInEditor() const override { return false; }

    // Required override
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(URewindManager, STATGROUP_Tickables); }

    // --- Component Registration ---

    void RegisterComponent(URewindComponent* Component);
    void UnregisterComponent(URewindComponent* Component);

    // --- Rewind Control ---

    void StartRewind();
    void StopRewind();

    bool IsRewinding() const { return bIsRewinding; }

private:
    // All components currently registered with the manager
    UPROPERTY()
    TArray<URewindComponent*> RegisteredComponents;

    // Current rewind state
    bool bIsRewinding = false;

    // How fast to rewind (1.0 = real time backwards, 2.0 = double speed)
    float RewindSpeed = 1.0f;
};