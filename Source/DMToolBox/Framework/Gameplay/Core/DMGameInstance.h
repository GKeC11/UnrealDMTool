#pragma once

#include "Engine/GameInstance.h"
#include "DMGameInstance.generated.h"

class UClass;

UCLASS()
class DMTOOLBOX_API UDMGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	bool AddPuertsLoadedClassReference(UClass* InClass);

protected:
	// Override
	virtual void OnStart() override;

	// Puerts
	UFUNCTION(BlueprintImplementableEvent)
	void TS_Init();

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UClass>> PuertsLoadedClassReferences;
};
