#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"

#include "DMUILibrary.generated.h"

UCLASS()
class UDMUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static void CreateWidgetToLayer(APlayerController* InPlayerController, TSubclassOf<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerTag);
};
