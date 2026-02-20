// Code by Jack Adams - Created 23/01/26
// Lasted editied - 25/01/26

#include "RewindComponent.h"
#include "RewindManager.h"
#include "Components/PrimitiveComponent.h"

// Sets default values for this component's properties
URewindComponent::URewindComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void URewindComponent::BeginPlay()
{
	Super::BeginPlay();

	URewindManager* manager = GetWorld()->GetSubsystem<URewindManager>();

	// register self with manager so that the component can be told WHEN to rewind
	if (manager)
		manager->RegisterComponent(this);

}

void URewindComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	URewindManager* manager = GetWorld()->GetSubsystem<URewindManager>();

	// Unregister from the manager to prevent dangling pointers
	if (manager)
		manager->UnregisterComponent(this);

	Super::EndPlay(EndPlayReason);
}


// Called every frame
void URewindComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Don't record while time is frozen (paused or rewinding)
	if (bIsTimeFrozen)
	{
		return;
	}

	frameCounter++;

	// Only want to be recording once every frameskip+1 frames
	// so if frameskip is 4 we record at 5,10,15...
	if (frameCounter <= frameSkip)
		return;

	// reset counter now that we are recording this frame
	frameCounter = 0;

	// create snapshot
	FRewindSnapshot newSnapshot;
	newSnapshot.timeStamp = GetWorld()->GetTimeSeconds();
	newSnapshot.transform = GetOwner()->GetActorTransform();

	if (GetOwner()->Implements<URewindableState>()) {
		IRewindableState* rewindable = Cast<IRewindableState>(GetOwner());
		if (rewindable) {
			rewindable->CaptureRewindState(newSnapshot.customState);
		}
	}

	// Add to the history
	snapshotHistory.Add(newSnapshot);

	// Remove old snapshots that are beyond our max duration
	float oldestAllowedTime = newSnapshot.timeStamp - maxRecordDuration;

	// remove from the front while snapshots are too old
	while (snapshotHistory.Num() > 0 && snapshotHistory[0].timeStamp < oldestAllowedTime) {
		snapshotHistory.RemoveAt(0);
	}

	// Debug output - remove later
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT("%s: %d snapshots stored (%.1fs of history)"),
			*GetOwner()->GetName(),
			snapshotHistory.Num(),
			snapshotHistory.Num() > 0 ? newSnapshot.timeStamp - snapshotHistory[0].timeStamp : 0.0f);

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, DebugMsg);
	}
}

void URewindComponent::StartPause()
{
	bIsTimeFrozen = true;
	bIsRewinding = false;

	FreezePhysics();

	// Set playback time to current position so toggling to rewind starts from here
	if (snapshotHistory.Num() > 0)
	{
		rewindPlaybackTime = snapshotHistory.Last().timeStamp;
	}
}

void URewindComponent::StartRewind()
{
	//Can't rewind without history
	if (snapshotHistory.Num() == 0)
		return;

	bIsTimeFrozen = true;  // Add this line
	bIsRewinding = true;

	FreezePhysics();

	// Only reset playback time if we're starting fresh
	if (rewindPlaybackTime == 0.0f)
	{
		rewindPlaybackTime = snapshotHistory.Last().timeStamp;
	}
}

void URewindComponent::StopTimeManipulation()
{
	bIsTimeFrozen = false;
	bIsRewinding = false;

	UnfreezePhysics();

	snapshotHistory.Empty();
	frameCounter = 0;
	rewindPlaybackTime = 0.0f;
}

void URewindComponent::UpdateRewind(float TimeDelta)
{
	if (!bIsRewinding || snapshotHistory.Num() == 0)
	{
		return;
	}

	rewindPlaybackTime += TimeDelta;

	float oldestTime = snapshotHistory[0].timeStamp;
	float newestTime = snapshotHistory.Last().timeStamp;
	rewindPlaybackTime = FMath::Clamp(rewindPlaybackTime, oldestTime, newestTime);

	int32 afterIndex = 0;
	for (int32 i = 0; i < snapshotHistory.Num(); i++)
	{
		if (snapshotHistory[i].timeStamp >= rewindPlaybackTime)
		{
			afterIndex = i;
			break;
		}
	}

	if (afterIndex == 0)
	{
		GetOwner()->SetActorTransform(snapshotHistory[0].transform);

		// Apply custom state for first snapshot too
		if (GetOwner()->Implements<URewindableState>())
		{
			IRewindableState* rewindable = Cast<IRewindableState>(GetOwner());
			if (rewindable)
			{
				rewindable->ApplyRewindState(snapshotHistory[0].customState);
			}
		}
		return;
	}

	int32 beforeIndex = afterIndex - 1;

	FRewindSnapshot& before = snapshotHistory[beforeIndex];
	FRewindSnapshot& after = snapshotHistory[afterIndex];

	float timeBetween = after.timeStamp - before.timeStamp;
	float alpha = (rewindPlaybackTime - before.timeStamp) / timeBetween;

	FVector InterpolatedLocation = FMath::Lerp(before.transform.GetLocation(), after.transform.GetLocation(), alpha);
	FQuat InterpolatedRotation = FQuat::Slerp(before.transform.GetRotation(), after.transform.GetRotation(), alpha);
	FVector InterpolatedScale = FMath::Lerp(before.transform.GetScale3D(), after.transform.GetScale3D(), alpha);

	FTransform InterpolatedTransform(InterpolatedRotation, InterpolatedLocation, InterpolatedScale);
	GetOwner()->SetActorTransform(InterpolatedTransform);

	// Apply custom state if the owner implements the interface
	if (GetOwner()->Implements<URewindableState>())
	{
		IRewindableState* rewindable = Cast<IRewindableState>(GetOwner());
		if (rewindable)
		{
			TMap<FName, float> interpolatedState;

			TArray<FName> allKeys;
			before.customState.GetKeys(allKeys);
			for (const auto& pair : after.customState)
			{
				allKeys.AddUnique(pair.Key);
			}

			for (const FName& key : allKeys)
			{
				float BeforeVal = before.customState.Contains(key) ? before.customState[key] : 0.0f;
				float AfterVal = after.customState.Contains(key) ? after.customState[key] : 0.0f;
				interpolatedState.Add(key, FMath::Lerp(BeforeVal, AfterVal, alpha));
			}

			rewindable->ApplyRewindState(interpolatedState);
		}
	}
}

void URewindComponent::UpdatePause()
{
	if (!bIsTimeFrozen || snapshotHistory.Num() == 0)
	{
		return;
	}

	GetOwner()->SetActorTransform(snapshotHistory.Last().transform);
}

void URewindComponent::FreezePhysics()
{
	// Cache the primitive component if we haven't already
	if (!CachedPrimitiveComponent)
	{
		CachedPrimitiveComponent = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	}

	if (CachedPrimitiveComponent && CachedPrimitiveComponent->IsSimulatingPhysics())
	{
		// Remember that physics was on
		bWasSimulatingPhysics = true;

		// Zero out velocity
		//CachedPrimitiveComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		//CachedPrimitiveComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		// Disable physics simulation entirely
		CachedPrimitiveComponent->SetSimulatePhysics(false);
	}
}

void URewindComponent::UnfreezePhysics()
{
	if (CachedPrimitiveComponent && bWasSimulatingPhysics)
	{
		// Re-enable physics
		CachedPrimitiveComponent->SetSimulatePhysics(true);

		// Zero velocity so it doesn't continue with old momentum
		//CachedPrimitiveComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		//CachedPrimitiveComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

		bWasSimulatingPhysics = false;
	}
}