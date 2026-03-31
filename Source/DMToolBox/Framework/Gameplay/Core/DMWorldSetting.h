#pragma once

#include "DMToolBox/Framework/Gameplay/Data/DMLevelInitializationSetting.h"

#include "DMWorldSetting.generated.h"

UCLASS()
class ADMWorldSetting : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DMToolBox")
	UDMLevelInitializationSetting* LevelInitializationSetting;
};
