#pragma once

#include "GameplayTagContainer.h"

#include "DMWidgetConfig.generated.h"

USTRUCT()
struct FDMWidgetConfig : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag WidgetTag;
	
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UUserWidget> WidgetClass;
};
