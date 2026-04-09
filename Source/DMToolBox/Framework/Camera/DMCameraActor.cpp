#include "DMCameraActor.h"

#include "Components/SceneComponent.h"
#include "GameFramework/PlayerController.h"

ADMCameraActor::ADMCameraActor(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootScene);

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

	if (bFollowPlayerControllerRotation && IsValid(PlayerController))
	{
		SetActorRotation(PlayerController->GetControlRotation());
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
	if (bFollowPlayerControllerRotation)
	{
		SetActorRotation(PlayerController->GetControlRotation());
	}
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

	if (bFollowPlayerControllerRotation && IsValid(PlayerController))
	{
		SetActorRotation(PlayerController->GetControlRotation());
	}
}
