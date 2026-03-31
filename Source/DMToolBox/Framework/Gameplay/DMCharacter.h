#pragma once

#include "DMToolBox/Framework/Input/EnhancedInputHelperComponent.h"
#include "GameFramework/Character.h"

#include "DMCharacter.generated.h"

UCLASS()
class ADMCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Character
	UEnhancedInputHelperComponent* GetEnhancedInputHelper() { return EnhancedInputHelper; }

protected:
	// Character
	ADMCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION()
	void OnMoveTriggered(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime, const UInputAction* SourceAction);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UEnhancedInputHelperComponent* EnhancedInputHelper;
};
