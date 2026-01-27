// Fill out your copyright notice in the Description page of Project Settings.


#include "RewindableDoor.h"
#include "RewindComponent.h"

// Sets default values
ARewindableDoor::ARewindableDoor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// create root comp and default mesh
	doorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	RootComponent = doorRoot;

	doorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	doorMesh->SetupAttachment(doorRoot);

	// Load a default cube mesh - we can change this in the editor
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		doorMesh->SetStaticMesh(CubeMesh.Object);
		// Scale it to look more door-like (thin and tall)
		doorMesh->SetRelativeScale3D(FVector(0.1f, 1.0f, 2.0f));
		// Offset so it rotates from the edge, not the centre
		doorMesh->SetRelativeLocation(FVector(0.0f, 50.0f, 100.0f));
	}

	// Create rewind component
	rewindComponent = CreateDefaultSubobject<URewindComponent>(TEXT("RewindComponent"));
}

// Called when the game starts or when spawned
void ARewindableDoor::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ARewindableDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (rewindComponent && rewindComponent->IsRewinding()) {
		return;
	}

	// Automatic swing - opens and closes on its own for testing
	if (bIsOpening)
	{
		doorOpenAmount += doorSpeed * DeltaTime;
		if (doorOpenAmount >= 1.0f)
		{
			doorOpenAmount = 1.0f;
			bIsOpening = false;
		}
	}
	else
	{
		doorOpenAmount -= doorSpeed * DeltaTime;
		if (doorOpenAmount <= 0.0f)
		{
			doorOpenAmount = 0.0f;
			bIsOpening = true;
		}
	}

	// Apply the rotation based on open amount
	float CurrentAngle = doorOpenAmount * maxOpenAngle;
	doorMesh->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));
}

void ARewindableDoor::CaptureRewindState(TMap<FName, float>& OutState)
{
	OutState.Add(FName("DoorOpenAmount"), doorOpenAmount);
	OutState.Add(FName("bIsOpening"), bIsOpening ? 1.0f : 0.0f);
}

void ARewindableDoor::ApplyRewindState(const TMap<FName, float>& InState)
{
	if (InState.Contains(FName("DoorOpenAmount")))
	{
		doorOpenAmount = InState[FName("DoorOpenAmount")];
	}

	if (InState.Contains(FName("bIsOpening")))
	{
		bIsOpening = InState[FName("bIsOpening")] > 0.5f;
	}

	// Apply the visual rotation
	float CurrentAngle = doorOpenAmount * maxOpenAngle;
	doorMesh->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));
}
