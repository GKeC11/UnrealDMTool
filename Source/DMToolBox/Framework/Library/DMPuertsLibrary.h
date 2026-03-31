#pragma once

#include "DMPuertsLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FConsoleCommandDynamicDelegate, const UWorld*, InWorld, const TArray<FString>&, Args);

UCLASS()
class DMTOOLBOX_API UDMPuertsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void RegisterConsoleCommand(const FString& CommandName, const FString& CommandDesc, const FConsoleCommandDynamicDelegate& CommandDelegate);
};
