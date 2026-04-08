#pragma once

#include "Components/ActorComponent.h"

#include "DMCameraManagerComponent.generated.h"

class ADMCameraActor;
class AController;
class APawn;
class APlayerController;

UCLASS(ClassGroup = (DMToolBox), meta = (BlueprintSpawnableComponent))
class DMTOOLBOX_API UDMCameraManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDMCameraManagerComponent();

	UFUNCTION(BlueprintPure, Category = "Camera")
	ADMCameraActor* GetCameraActor() const { return CameraActor; }

	UFUNCTION(BlueprintPure, Category = "Camera")
	APlayerController* GetPlayerController() const { return PlayerController; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPlayerController(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void RefreshCamera();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TSubclassOf<ADMCameraActor> CameraActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	int32 PlayerIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bAutoSetViewTarget;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<APlayerController> PlayerControllerOverride;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<ADMCameraActor> CameraActor;

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<APlayerController> PlayerController;

private:
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleOwnerPawnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController);

	APlayerController* ResolvePlayerController() const;
	void BindToPlayerController(APlayerController* InPlayerController);
	void EnsureCameraActor();
	void ApplyCameraToPawn(APawn* NewPawn);
};
