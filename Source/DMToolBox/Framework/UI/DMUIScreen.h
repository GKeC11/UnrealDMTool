#pragma once

#include "DMUILayout.h"

#include "DMUIScreen.generated.h"

UCLASS(Blueprintable)
class UDMUIScreen : public UObject
{
	GENERATED_BODY()

public:
	// Static
	static UDMUIScreen* GetUIScreen(UObject* InContext);

	// UI
	void RegisterLayoutForLocalPlayer(ULocalPlayer* InLocalPlayer);

	UDMUILayout* GetLayoutFromLocalPlayer(ULocalPlayer* InLocalPlayer);

protected:
	// UI
	void AddLayoutToViewport(ULocalPlayer* InLocalPlayer, UDMUILayout* Layout);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UDMUILayout> DefaultUILayoutClass;

protected:
	UPROPERTY(Transient)
	TMap<ULocalPlayer*, UDMUILayout*> LayoutEntryMap;
};
