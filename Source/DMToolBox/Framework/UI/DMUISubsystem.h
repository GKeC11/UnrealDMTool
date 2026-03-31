#pragma once

#include "DMUIScreen.h"

#include "DMUISubsystem.generated.h"

UCLASS()
class UDMUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UDMUIScreen* GetCurrentScreen() { return CurrentScreen; }
	
protected:
	// Override
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// UI Subsystem
	void OnLocalPlayerAddedEvent(ULocalPlayer* InLocalPlayer);

protected:
	// UI
	UPROPERTY()
	UDMUIScreen* CurrentScreen = nullptr;
};
