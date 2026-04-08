#include "DMCameraActor.h"

#include "GameFramework/PlayerController.h"

ADMCameraActor::ADMCameraActor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SetRootComponent(SpringArm);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	SpringArm->TargetArmLength = 1000.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetRelativeRotation(FRotator(-45.0f, 90.0f, 0.0f));
}

void ADMCameraActor::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (IsValid(ActorToFollow))
	{
		SetActorLocation(ActorToFollow->GetActorLocation());
	}
}

bool ADMCameraActor::InitializeFromPlayerController(APlayerController* InPlayerController)
{
	PlayerController = InPlayerController;
	if (!IsValid(PlayerController))
	{
		return false;
	}

	SetActorToFollow(PlayerController->GetPawn());
	PlayerController->SetViewTarget(this);

	if (APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager)
	{
		PlayerCameraManager->UpdateCamera(0.0f);
	}

	return true;
}

void ADMCameraActor::SetActorToFollow(AActor* InActorToFollow)
{
	ActorToFollow = InActorToFollow;

	if (IsValid(ActorToFollow))
	{
		SetActorLocation(ActorToFollow->GetActorLocation());
	}
}
