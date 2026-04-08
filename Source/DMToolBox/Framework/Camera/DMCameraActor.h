#pragma once

#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "DMCameraActor.generated.h"

class APlayerController;

UCLASS()
class DMTOOLBOX_API ADMCameraActor : public AActor
{
	GENERATED_BODY()
	
public:
	ADMCameraActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Camera")
	bool InitializeFromPlayerController(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetActorToFollow(AActor* InActorToFollow);

	UFUNCTION(BlueprintPure, Category = "Camera")
	AActor* GetActorToFollow() const { return ActorToFollow; }

protected:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(Transient)
	TObjectPtr<AActor> ActorToFollow;
};
