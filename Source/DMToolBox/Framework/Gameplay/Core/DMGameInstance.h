#pragma once

#include "Engine/GameInstance.h"
#include "DMGameInstance.generated.h"

class UClass;

DECLARE_MULTICAST_DELEGATE(FDMScriptInitializedDelegate);

UCLASS()
class DMTOOLBOX_API UDMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	bool AddPuertsLoadedClassReference(UClass* InClass);

	bool IsScriptInitialized() const { return bScriptInitialized; }

	UFUNCTION(BlueprintCallable)
	void NotifyScriptInitialized();

	FDMScriptInitializedDelegate OnScriptInitialized;

protected:
	// Override
	virtual void OnStart() override;

	// Puerts
	UFUNCTION(BlueprintImplementableEvent)
	void TS_Init();

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UClass>> PuertsLoadedClassReferences;

	bool bScriptInitialized = false;
};
