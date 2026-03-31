#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "DMToolBox/Framework/Gameplay/DMCameraActor.h"

#include "DMLevelInitializationSetting.generated.h"

USTRUCT(BlueprintType)
struct FLevelInitializationSetting_WidgetConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Layer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCommonActivatableWidget> WidgetClass;
};

UCLASS()
class UDMLevelInitializationSetting : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TArray<FLevelInitializationSetting_WidgetConfig> WidgetConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	TSubclassOf<ADMCameraActor> CameraActorClass;
};
