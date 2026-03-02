// Code by Jack Adams - Created 25/01/26
// Lasted editied - 03/02/26

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

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        return;
    }

    // Update what we're looking at
    UpdateLookedAtComponent();

    // Handle E key for targeting
    HandleTargetingInput(PC, DeltaTime);

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

    // Handle rewind playback for targeted components
    if (CurrentState == ETimeManipulationState::Rewinding)
    {
        float TimeDelta = -DeltaTime * RewindSpeed;

        for (URewindComponent* Component : TargetedComponents)
        {
            if (Component)
            {
                Component->UpdateRewind(TimeDelta);
            }
        }
    }
    else if (CurrentState == ETimeManipulationState::Paused)
    {
        for (URewindComponent* Component : TargetedComponents)
        {
            if (Component)
            {
                Component->UpdatePause();
            }
        }
    }

    // Debug display
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

        FString LookingAt = LookedAtComponent ? LookedAtComponent->GetOwner()->GetName() : TEXT("Nothing");

        GEngine->AddOnScreenDebugMessage(0, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Ability: %s | State: %s | Targets: %d/%d"),
                *AbilityName, *StateName, TargetedComponents.Num(), MaxTargets));

        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Looking at: %s | E to target, Q to switch, R to activate"),
                *LookingAt));

        // Show clear progress if holding
        if (bIsHoldingTargetKey && TargetKeyHoldTime > TapThreshold)
        {
            float ClearProgress = GetClearTargetsProgress();
            GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Red,
                FString::Printf(TEXT("Clearing targets: %.0f%%"), ClearProgress * 100.0f));
        }
    }
}

void URewindManager::UpdateLookedAtComponent()
{
    UWorld* World = GetWorld();
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        LookedAtComponent = nullptr;
        return;
    }

    // Get camera location and direction
    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * MaxTargetRange);

    // Perform line trace
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(PC->GetPawn());

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        CameraLocation,
        TraceEnd,
        ECC_Visibility,
        QueryParams
    );

    if (bHit && HitResult.GetActor())
    {
        // Check if hit actor has a rewind component
        URewindComponent* RewindComp = HitResult.GetActor()->FindComponentByClass<URewindComponent>();
        LookedAtComponent = RewindComp;
    }
    else
    {
        LookedAtComponent = nullptr;
    }
}

void URewindManager::HandleTargetingInput(APlayerController* PC, float DeltaTime)
{
    bool bKeyDown = PC->IsInputKeyDown(EKeys::E);

    if (bKeyDown)
    {
        if (!bIsHoldingTargetKey)
        {
            // Just started holding
            bIsHoldingTargetKey = true;
            TargetKeyHoldTime = 0.0f;
        }
        else
        {
            // Continue holding
            TargetKeyHoldTime += DeltaTime;
        }
    }
    else
    {
        // Key released
        if (bIsHoldingTargetKey)
        {
            if (TargetKeyHoldTime < TapThreshold)
            {
                // Tap - target or untarget
                if (LookedAtComponent)
                {
                    if (IsComponentTargeted(LookedAtComponent))
                    {
                        UntargetComponent(LookedAtComponent);
                    }
                    else
                    {
                        TargetComponent(LookedAtComponent);
                    }
                }
            }
            else if (TargetKeyHoldTime >= ClearHoldThreshold)
            {
                // Long hold - clear all
                ClearAllTargets();
            }
            // Between tap and clear threshold = nothing happens

            bIsHoldingTargetKey = false;
            TargetKeyHoldTime = 0.0f;
        }
    }
}

void URewindManager::TargetComponent(URewindComponent* Component)
{
    if (!Component)
    {
        return;
    }

    if (TargetedComponents.Num() >= MaxTargets)
    {
        UE_LOG(LogTemp, Warning, TEXT("Max targets reached (%d)"), MaxTargets);
        return;
    }

    if (TargetedComponents.Contains(Component))
    {
        return;
    }

    TargetedComponents.Add(Component);
    UE_LOG(LogTemp, Log, TEXT("Targeted: %s"), *Component->GetOwner()->GetName());

    // If ability is already active, apply it to this new target
    if (CurrentState != ETimeManipulationState::Normal)
    {
        ApplyAbilityToComponent(Component);
    }
}

void URewindManager::UntargetComponent(URewindComponent* Component)
{
    if (!Component)
    {
        return;
    }

    if (!TargetedComponents.Contains(Component))
    {
        return;
    }

    // If ability is active, remove it from this component first
    if (CurrentState != ETimeManipulationState::Normal)
    {
        RemoveAbilityFromComponent(Component);
    }

    TargetedComponents.Remove(Component);
    UE_LOG(LogTemp, Log, TEXT("Untargeted: %s"), *Component->GetOwner()->GetName());
}

void URewindManager::ClearAllTargets()
{
    // Remove ability from all targeted components if active
    if (CurrentState != ETimeManipulationState::Normal)
    {
        for (URewindComponent* Component : TargetedComponents)
        {
            if (Component)
            {
                RemoveAbilityFromComponent(Component);
            }
        }
    }

    TargetedComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("Cleared all targets"));
}

bool URewindManager::IsComponentTargeted(URewindComponent* Component) const
{
    return TargetedComponents.Contains(Component);
}

float URewindManager::GetClearTargetsProgress() const
{
    if (!bIsHoldingTargetKey || TargetKeyHoldTime < TapThreshold)
    {
        return 0.0f;
    }

    float ProgressTime = TargetKeyHoldTime - TapThreshold;
    float TotalTime = ClearHoldThreshold - TapThreshold;

    return FMath::Clamp(ProgressTime / TotalTime, 0.0f, 1.0f);
}

void URewindManager::ApplyAbilityToComponent(URewindComponent* Component)
{
    if (!Component)
    {
        return;
    }

    if (CurrentState == ETimeManipulationState::Paused)
    {
        Component->StartPause();
    }
    else if (CurrentState == ETimeManipulationState::Rewinding)
    {
        Component->StartRewind();
    }
}

void URewindManager::RemoveAbilityFromComponent(URewindComponent* Component)
{
    if (!Component)
    {
        return;
    }

    Component->StopTimeManipulation();
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

    if (TargetedComponents.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No targets selected"));
        return;
    }

    if (SelectedAbility == ETimeAbility::Pause)
    {
        CurrentState = ETimeManipulationState::Paused;

        for (URewindComponent* Component : TargetedComponents)
        {
            if (Component)
            {
                Component->StartPause();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("PAUSE activated on %d targets"), TargetedComponents.Num());
    }
    else
    {
        CurrentState = ETimeManipulationState::Rewinding;

        for (URewindComponent* Component : TargetedComponents)
        {
            if (Component)
            {
                Component->StartRewind();
            }
        }

        UE_LOG(LogTemp, Log, TEXT("REWIND activated on %d targets"), TargetedComponents.Num());
    }
}

void URewindManager::DeactivateAbility()
{
    if (CurrentState == ETimeManipulationState::Normal)
    {
        return;
    }

    CurrentState = ETimeManipulationState::Normal;

    for (URewindComponent* Component : TargetedComponents)
    {
        if (Component)
        {
            Component->StopTimeManipulation();
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Ability deactivated"));
}