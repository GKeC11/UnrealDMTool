#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DMToolBox/Framework/Gameplay/Data/DMLevelInitializationSetting.h"

#include "DMSystemLibrary.generated.h"

class UDMGameInstance;
class UObject;
class UWorld;

UCLASS()
class UDMSystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UDMGameInstance* ResolveDMGameInstance();
	static UWorld* ResolveWorldFromContext(const UObject* WorldContextObject);
	static int32 ResolveClientIndex(const UWorld* World);

	static UDMLevelInitializationSetting* GetLevelInitializationSetting(UWorld* InWorld);
};
