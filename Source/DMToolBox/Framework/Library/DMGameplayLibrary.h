#pragma once

#include "InputMappingContext.h"

#include "DMGameplayLibrary.generated.h"

UCLASS()
class UDMGameplayLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void RegisterInputMappingContext(APlayerController* InPlayerController, UInputMappingContext* InputMappingContextToRegister);

	static void UnregisterInputMappingContext(APlayerController* InPlayerController, UInputMappingContext* InputMappingContextToRegister);
};
