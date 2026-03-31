#pragma once

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "DMCameraActor.generated.h"

UCLASS()
class ADMCameraActor : public AActor
{
	GENERATED_BODY()
	
public:
	// Camera Actaor
	bool InitializeFromPlayerController(const APlayerController* InPlayerController);

protected:
	// Override
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	// Camera Actor
	ADMCameraActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* Camera;

	const APlayerController* PlayerController = nullptr;

	AActor* ActorToFollow = nullptr;;
};
