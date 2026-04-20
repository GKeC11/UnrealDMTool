#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "DMUILibrary.generated.h"

UCLASS()
class DMTOOLBOX_API UDMUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI", meta = (WorldContext = "WorldContextObject"))
	static bool CreateWidgetByTagToLayer(UObject* WorldContextObject, FGameplayTag InWidgetTag, FGameplayTag InLayerTag);

	UFUNCTION(BlueprintCallable, Category = "DMToolBox|UI", meta = (WorldContext = "WorldContextObject"))
	static bool RemoveWidgetByTag(UObject* WorldContextObject, FGameplayTag InWidgetTag);

	static void CreateWidgetToLayer(APlayerController* InPlayerController, TSubclassOf<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerTag);
};
