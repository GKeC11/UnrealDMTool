#include "DMCameraActor.h"

void ADMCameraActor::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (ActorToFollow)
	{
		SetActorLocation(ActorToFollow->GetActorLocation());
	}
}

ADMCameraActor::ADMCameraActor(const FObjectInitializer& ObjectInitializer)
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachToComponent(SpringArm, FAttachmentTransformRules::KeepRelativeTransform);

	SpringArm->TargetArmLength = 1000;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetRelativeRotation(FRotator(-45, 90, 0));
}

bool ADMCameraActor::InitializeFromPlayerController(const APlayerController* InPlayerController)
{
	PlayerController = InPlayerController;
	
	if (APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager)
	{
		PlayerCameraManager->SetViewTarget(this);
		PlayerCameraManager->UpdateCamera(0);
	}

	ActorToFollow = PlayerController->GetPawn();

	return true;
}
