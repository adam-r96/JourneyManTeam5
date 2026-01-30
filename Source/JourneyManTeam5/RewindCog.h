// Code by Jack Adams - Created 28/01/26
// Lasted editied - 30/01/26

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RewindCog.generated.h"

UCLASS()
class JOURNEYMANTEAM5_API ARewindCog : public AActor
{
	GENERATED_BODY()

public:
	ARewindCog();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cog")
	USceneComponent* cogRoot;

	// Visual mesh for the cog
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cog")
	UStaticMeshComponent* cogMesh;

	// Trigger zone for player interaction
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cog")
	class USphereComponent* interactionZone;

	// --- Overlap Events ---

	UFUNCTION()
	void OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// --- State ---

	// Is a player currently in range?
	bool bPlayerInRange = false;

	// Is rewind currently active from this cog?
	bool bIsRewindingActive = false;

	// Reference to the player in range (if any)
	UPROPERTY()
	APawn* playerInZone = nullptr;

	// --- Configuration ---

	// Which key to hold for rewinding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cog")
	FKey interactionKey = EKeys::E;

	// How fast the cog spins while rewinding (visual only)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cog")
	float cogSpinSpeed = 180.0f;
};