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

    // Old debug for rewind to test if it was working, if you want this back uncomment it
    // Take note that it will break any cogs that are in the map from the way this is all setup
    // hold R to manipulate time, Q to toggle pause/rewind
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC)
    {
        // Q - switch between pause and rewind ability
        if (PC->WasInputKeyJustPressed(EKeys::Q))
        {
            SwitchAbility();
        }

        // R - toggle the current ability on/off
        if (PC->WasInputKeyJustPressed(EKeys::R))
        {
            if (CurrentState == ETimeManipulationState::Normal)
            {
                ActivateAbility();
            }
            else
            {
                DeactivateAbility();
            }
        }
    }

    // Debug - show current state
    if (GEngine)
    {
        FString AbilityName = (SelectedAbility == ETimeAbility::Pause) ? TEXT("PAUSE") : TEXT("REWIND");
        FString StateName;
        switch (CurrentState)
        {
        case ETimeManipulationState::Normal: StateName = TEXT("Normal"); break;
        case ETimeManipulationState::Paused: StateName = TEXT("Paused"); break;
        case ETimeManipulationState::Rewinding: StateName = TEXT("Rewinding"); break;
        }

        GEngine->AddOnScreenDebugMessage(0, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Selected: %s | State: %s | Q to switch, R to activate"),
                *AbilityName, *StateName));
    }


    // Handle rewind playback
    if (CurrentState == ETimeManipulationState::Rewinding)
    {
        float TimeDelta = -DeltaTime * RewindSpeed;

        for (URewindComponent* Component : RegisteredComponents)
        {
            if (Component)
            {
                Component->UpdateRewind(TimeDelta);
            }
        }
    }
    else if (CurrentState == ETimeManipulationState::Paused) 
    {
        for (URewindComponent* Component : RegisteredComponents) 
        {
            if (Component) 
            {
                Component->UpdatePause();
            }
        }
    }

    // Paused state doesn't need to do anything - objects are just frozen
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

void URewindManager::SwitchAbility()
{
    // Only allow switching when not actively using an ability
    if (CurrentState != ETimeManipulationState::Normal)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot switch ability while active"));
        return;
    }

    if (SelectedAbility == ETimeAbility::Pause)
    {
        SelectedAbility = ETimeAbility::Rewind;
        UE_LOG(LogTemp, Log, TEXT("Switched to REWIND ability"));
    }
    else
    {
        SelectedAbility = ETimeAbility::Pause;
        UE_LOG(LogTemp, Log, TEXT("Switched to PAUSE ability"));
    }
}

void URewindManager::ActivateAbility()
{
    if (CurrentState != ETimeManipulationState::Normal)
    {
        return;
    }

    if (SelectedAbility == ETimeAbility::Pause)
    {
        CurrentState = ETimeManipulationState::Paused;

        for (URewindComponent* Component : RegisteredComponents)
        {
            if (Component)
            {
                Component->StartPause();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("PAUSE activated"));
    }
    else
    {
        CurrentState = ETimeManipulationState::Rewinding;

        for (URewindComponent* Component : RegisteredComponents)
        {
            if (Component)
            {
                Component->StartRewind();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("REWIND activated"));
    }
}

void URewindManager::DeactivateAbility()
{
    if (CurrentState == ETimeManipulationState::Normal)
    {
        return;
    }

    CurrentState = ETimeManipulationState::Normal;

    for (URewindComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->StopTimeManipulation();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Ability deactivated - returning to NORMAL"));
}