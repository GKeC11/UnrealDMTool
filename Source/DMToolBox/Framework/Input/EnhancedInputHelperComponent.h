#pragma once

#include "InputMappingContext.h"

#include "EnhancedInputHelperComponent.generated.h"

USTRUCT(BlueprintType)
struct FEnhancedInputHelperInputActionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(GetOptions="GetFunctionNamesForInputAction"))
	FString FunctionToBind;
};

UCLASS(meta=(BlueprintSpawnableComponent))
class UEnhancedInputHelperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Helper
	void RegisterEnhancedInput(APlayerController* InPlayerController);

	void UnregisterEnhancedInput(APlayerController* InPlayerController);

protected:
	// Override
	virtual void BeginPlay() override;

	// Helper
	void RegisterInputMappingContext(APlayerController* InPlayerController);

	void UnregisterInputMappingContext(APlayerController* InPlayerController);
	
	void RegisterInputActions();

	void UnregisterInputActions();

	UFUNCTION()
	TArray<FString> GetFunctionNamesForInputAction();
	
public:
	// Helper
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputMappingContextToRegister;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TArray<FEnhancedInputHelperInputActionConfig> InputActionConfigs;
};

