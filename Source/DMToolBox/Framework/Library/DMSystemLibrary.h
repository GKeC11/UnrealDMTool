#pragma once

#include "DMToolBox/Framework/Gameplay/Data/DMLevelInitializationSetting.h"

#include "DMSystemLibrary.generated.h"

UCLASS()
class UDMSystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	static UDMLevelInitializationSetting* GetLevelInitializationSetting(UWorld* InWorld);
};
