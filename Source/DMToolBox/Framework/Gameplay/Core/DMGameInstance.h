#pragma once

#include "DMGameInstance.generated.h"

UCLASS()
class UDMGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
protected:
	// Override
	virtual void Init() override;

	// Puerts
	UFUNCTION(BlueprintImplementableEvent)
	void TS_Init();
};
