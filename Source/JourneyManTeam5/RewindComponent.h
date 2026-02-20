// Code by Jack Adams - Created 23/01/26
// Lasted editied - 25/01/26

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RewindableState.h"
#include "RewindComponent.generated.h"

USTRUCT(BlueprintType)
struct FRewindSnapshot
{
    GENERATED_BODY()

    //When this snapshot was taken 
    UPROPERTY()
    float timeStamp = 0.0f;

    //Where the actor was at the time of snapshot
    UPROPERTY()
    FTransform transform;

    //Custom state tracking for actors via IRewindableState
    UPROPERTY()
    TMap<FName, float> customState;
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class JOURNEYMANTEAM5_API URewindComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URewindComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Control Functions (called by manager) ---

    // Freeze in place, keep recording position
    void StartPause();

    // Start rewinding from current position
    void StartRewind();

    // Return to normal operation
    void StopTimeManipulation();

    // Step the rewind playback by a time delta (negative = backwards)
    void UpdateRewind(float TimeDelta);

    // Update function to keep the object set to the frame that it was paused on
    void UpdatePause();

    bool IsTimeFrozen() const { return bIsTimeFrozen; }
    // Check if this component is currently being rewound
    bool IsRewinding() const { return bIsRewinding; }

    // --- Configuration ---

    // How many seconds of history to keep
    // At 60fps with a frameskip of 4, we are recording every 5th frame so 12 times per second
    // This means we are taking 720 snapshots every minute can be tweaked relative to performance and how it plays
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
    float maxRecordDuration = 60.0f;

    // How many frames to skip between recordings (0 = record every frame)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewind")
    int32 frameSkip = 4;

private:
    // --- Runtime Data ---

    // Our recorded history
    TArray<FRewindSnapshot> snapshotHistory;

    // Counts frames since last recording
    int32 frameCounter = 0;

    bool bIsTimeFrozen = false;   // True when paused OR rewinding
    bool bIsRewinding = false;    // True only when actively rewinding

    float rewindPlaybackTime = 0.0f;

    // Physics state before we froze it (so we can restore)
    bool bWasSimulatingPhysics = false;

    // Cache the primitive component for physics operations
    UPROPERTY()
    UPrimitiveComponent* CachedPrimitiveComponent = nullptr;

    // --- Helper Functions ---
    void FreezePhysics();
    void UnfreezePhysics();
};

