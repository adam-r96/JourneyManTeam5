// Code by Jack Adams - Created 28/01/26
// Lasted editied - 30/01/26

#include "RewindCog.h"
#include "RewindManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerController.h"

// Sets default values
ARewindCog::ARewindCog()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create root
	cogRoot = CreateDefaultSubobject<USceneComponent>(TEXT("CogRoot"));
	RootComponent = cogRoot;

	// Create cog mesh
	cogMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CogMesh"));
	cogMesh->SetupAttachment(cogRoot);

	// Load a default cylinder mesh - can be replaced with actual cog mesh in editor
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		cogMesh->SetStaticMesh(CylinderMesh.Object);
		cogMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 0.2f));
		cogMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	}

	// Create interaction zone
	interactionZone = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionZone"));
	interactionZone->SetupAttachment(cogRoot);
	interactionZone->SetSphereRadius(200.0f);
	interactionZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	interactionZone->SetGenerateOverlapEvents(true);
}

void ARewindCog::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	interactionZone->OnComponentBeginOverlap.AddDynamic(this, &ARewindCog::OnInteractionBeginOverlap);
	interactionZone->OnComponentEndOverlap.AddDynamic(this, &ARewindCog::OnInteractionEndOverlap);
}

void ARewindCog::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check for player input if they're in range
	if (bPlayerInRange && playerInZone)
	{
		APlayerController* PC = Cast<APlayerController>(playerInZone->GetController());
		if (PC)
		{
			bool bKeyHeld = PC->IsInputKeyDown(interactionKey);

			// Start rewind if key just pressed
			if (bKeyHeld && !bIsRewindingActive)
			{
				URewindManager* manager = GetWorld()->GetSubsystem<URewindManager>();
				if (manager)
				{
					UE_LOG(LogTemp, Display, TEXT("Hello"));
					manager->StartRewind();
					bIsRewindingActive = true;
				}
			}
			// Stop rewind if key released
			else if (!bKeyHeld && bIsRewindingActive)
			{
				URewindManager* Manager = GetWorld()->GetSubsystem<URewindManager>();
				if (Manager)
				{
					Manager->StopRewind();
					bIsRewindingActive = false;
				}
			}
		}
	}

	// Spin the cog while rewinding (visual feedback)
	if (bIsRewindingActive)
	{
		float SpinAmount = cogSpinSpeed * DeltaTime;
		cogMesh->AddRelativeRotation(FRotator(0.0f, 0.0f, SpinAmount));
	}
}

void ARewindCog::OnInteractionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if it's a player pawn
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		bPlayerInRange = true;
		playerInZone = Pawn;

		// Debug feedback
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
				FString::Printf(TEXT("Press %s to rewind"), *interactionKey.ToString()));
		}
	}
}

void ARewindCog::OnInteractionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn == playerInZone)
	{
		// Stop rewinding if player leaves while holding
		if (bIsRewindingActive)
		{
			URewindManager* Manager = GetWorld()->GetSubsystem<URewindManager>();
			if (Manager)
			{
				Manager->StopRewind();
				bIsRewindingActive = false;
			}
		}

		bPlayerInRange = false;
		playerInZone = nullptr;
	}
}
