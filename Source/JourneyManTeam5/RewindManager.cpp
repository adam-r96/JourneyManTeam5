// Code by Jack Adams - Created 25/01/26
// Lasted editied - 25/01/26

#include "RewindManager.h"

void URewindManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("Rewind Manager initialized"));
}

void URewindManager::Tick(float DeltaTime)
{
    // Make sure we have a valid world
    // Otherwise it will try and access the GetFirstPlayerController(); pointer before it even exists (causes crash)
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC) {
        if (PC->IsInputKeyDown(EKeys::R))
        {
            if (!bIsRewinding)
            {
                StartRewind();
            }
        }
        else
        {
            if (bIsRewinding)
            {
                StopRewind();
            }
        }
    }


    // Only do anything if we're rewinding
    if (!bIsRewinding)
    {
        return;
    }

    // Calculate how much time to step backwards
    // Negative because we're going back in time
    float TimeDelta = -DeltaTime * RewindSpeed;

    // Update all registered components
    for (URewindComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->UpdateRewind(TimeDelta);
        }
    }
}

void URewindManager::RegisterComponent(URewindComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        UE_LOG(LogTemp, Log, TEXT("Registered component on %s"), *Component->GetOwner()->GetName());
    }
}

void URewindManager::UnregisterComponent(URewindComponent* Component)
{
    RegisteredComponents.Remove(Component);
    UE_LOG(LogTemp, Log, TEXT("Unregistered rewind component"));
}

void URewindManager::StartRewind()
{
    if (bIsRewinding)
    {
        return;
    }

    bIsRewinding = true;

    // Tell all components to start rewinding
    for (URewindComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->StartRewind();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Rewind started"));
}

void URewindManager::StopRewind()
{
    if (!bIsRewinding)
    {
        return;
    }

    bIsRewinding = false;

    // Tell all components to stop rewinding
    for (URewindComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->StopRewind();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Rewind stopped"));
}