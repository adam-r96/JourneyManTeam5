#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RewindableState.h"
#include "RewindableDoor.generated.h"

UCLASS()
class JOURNEYMANTEAM5_API ARewindableDoor : public AActor, public IRewindableState
{
	GENERATED_BODY()

public:
	ARewindableDoor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// --- IRewindableState Interface ---
	virtual void CaptureRewindState(TMap<FName, float>& OutState) override;
	virtual void ApplyRewindState(const TMap<FName, float>& InState) override;

protected:
	// --- Components ---

	// The door frame (stationary)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	USceneComponent* doorRoot;

	// The door mesh (rotates)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	UStaticMeshComponent* doorMesh;

	// Our rewind component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	class URewindComponent* rewindComponent;

	// --- Door State ---

	// 0 = closed, 1 = fully open
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
	float doorOpenAmount = 0.0f;

	// How far the door rotates when fully open (degrees)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float maxOpenAngle = 90.0f;

	// How fast the door opens/closes (units per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
	float doorSpeed = 0.5f;

	// Is the door currently opening or closing?
	bool bIsOpening = true;
};