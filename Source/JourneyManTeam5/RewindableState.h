// Code by Jack Adams - Created 25/01/26
// Lasted editied - 25/01/26

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RewindableState.generated.h"

// This class does not need to be modified
// Its a dummy UObject for unreals reflection system to use
// No need to use this directly
UINTERFACE(MinimalAPI)
class URewindableState : public UInterface
{
    GENERATED_BODY()
};

// Implement this interface on actors that need custom state tracked during rewind
class IRewindableState
{
    GENERATED_BODY()

public:
    // Called when recording a snapshot - fill the map with your custom state
    virtual void CaptureRewindState(TMap<FName, float>& OutState) = 0;

    // Called during rewind playback - apply your saved state
    virtual void ApplyRewindState(const TMap<FName, float>& InState) = 0;
};