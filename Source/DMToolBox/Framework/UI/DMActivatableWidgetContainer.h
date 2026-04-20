#pragma once

#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include "DMActivatableWidgetContainer.generated.h"

UINTERFACE()
class UDMActivatableWidgetContainer : public UInterface
{
	GENERATED_BODY()
};

class IDMActivatableWidgetContainer
{
	GENERATED_BODY()
	
public:
	virtual FGameplayTag GetLayerTag() = 0;

	virtual UCommonActivatableWidgetContainerBase* GetContainer() = 0;
};


UCLASS()
class UDMActivatableWidgetStack : public UCommonActivatableWidgetStack, public IDMActivatableWidgetContainer
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetLayerTag() override { return LayerTag; }

	virtual UCommonActivatableWidgetContainerBase* GetContainer() override { return this; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FGameplayTag LayerTag;
};

UCLASS()
class UDMActivatableWidgetQueue : public UCommonActivatableWidgetQueue, public IDMActivatableWidgetContainer
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetLayerTag() override { return LayerTag; }

	virtual UCommonActivatableWidgetContainerBase* GetContainer() override { return this; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layer")
	FGameplayTag LayerTag;
};

