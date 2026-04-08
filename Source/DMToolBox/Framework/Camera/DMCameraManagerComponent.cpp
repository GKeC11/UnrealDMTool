#include "DMCameraManagerComponent.h"

#include "DMCameraActor.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UDMCameraManagerComponent::UDMCameraManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CameraActorClass = ADMCameraActor::StaticClass();
	PlayerIndex = 0;
	bAutoSetViewTarget = true;
}

void UDMCameraManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->ReceiveControllerChangedDelegate.AddDynamic(this, &ThisClass::HandleOwnerPawnControllerChanged);
	}

	RefreshCamera();
}

void UDMCameraManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerPawn->ReceiveControllerChangedDelegate.RemoveDynamic(this, &ThisClass::HandleOwnerPawnControllerChanged);
	}

	BindToPlayerController(nullptr);

	if (IsValid(CameraActor))
	{
		CameraActor->Destroy();
		CameraActor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UDMCameraManagerComponent::SetPlayerController(APlayerController* InPlayerController)
{
	PlayerControllerOverride = InPlayerController;
	RefreshCamera();
}

void UDMCameraManagerComponent::RefreshCamera()
{
	BindToPlayerController(ResolvePlayerController());

	if (IsValid(PlayerController))
	{
		ApplyCameraToPawn(PlayerController->GetPawn());
	}
	else if (IsValid(CameraActor))
	{
		CameraActor->SetActorToFollow(nullptr);
	}
}

void UDMCameraManagerComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	ApplyCameraToPawn(NewPawn);
}

void UDMCameraManagerComponent::HandleOwnerPawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	RefreshCamera();
}

APlayerController* UDMCameraManagerComponent::ResolvePlayerController() const
{
	if (IsValid(PlayerControllerOverride))
	{
		return PlayerControllerOverride;
	}

	if (APlayerController* OwnerPlayerController = Cast<APlayerController>(GetOwner()))
	{
		return OwnerPlayerController;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		return Cast<APlayerController>(OwnerPawn->GetController());
	}

	if (UWorld* World = GetWorld())
	{
		return UGameplayStatics::GetPlayerController(World, PlayerIndex);
	}

	return nullptr;
}

void UDMCameraManagerComponent::BindToPlayerController(APlayerController* InPlayerController)
{
	if (PlayerController == InPlayerController)
	{
		return;
	}

	if (IsValid(PlayerController))
	{
		PlayerController->OnPossessedPawnChanged.RemoveDynamic(this, &ThisClass::HandlePossessedPawnChanged);
	}

	PlayerController = nullptr;
	if (!IsValid(InPlayerController) || !InPlayerController->IsLocalController())
	{
		return;
	}

	PlayerController = InPlayerController;
	PlayerController->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::HandlePossessedPawnChanged);
}

void UDMCameraManagerComponent::EnsureCameraActor()
{
	if (IsValid(CameraActor))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController() || !IsValid(World))
	{
		return;
	}

	TSubclassOf<ADMCameraActor> ClassToSpawn = CameraActorClass;
	if (!ClassToSpawn)
	{
		ClassToSpawn = ADMCameraActor::StaticClass();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = PlayerController->GetPawn();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.ObjectFlags |= RF_Transient;

	CameraActor = World->SpawnActor<ADMCameraActor>(ClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (IsValid(CameraActor))
	{
		CameraActor->InitializeFromPlayerController(PlayerController);
	}
}

void UDMCameraManagerComponent::ApplyCameraToPawn(APawn* NewPawn)
{
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
	{
		return;
	}

	EnsureCameraActor();
	if (!IsValid(CameraActor))
	{
		return;
	}

	CameraActor->SetActorToFollow(NewPawn);
	if (bAutoSetViewTarget)
	{
		PlayerController->SetViewTarget(CameraActor);
	}
}
