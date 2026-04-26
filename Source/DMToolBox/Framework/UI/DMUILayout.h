#pragma once

#include "DMUserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/Overlay.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

#include "DMUILayout.generated.h"

UCLASS()
class UDMUILayout : public UDMUserWidget
{
	GENERATED_BODY()

public:
	UCommonActivatableWidgetContainerBase* GetLayerFromGameplayTag(FGameplayTag InTag);

protected:
	// Override
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI
	UFUNCTION(BlueprintCallable)
	void RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	void InitializeWhenReady();
	void RemoveScriptInitializedDelegate();
	void InitializeFromWorldSetting();

protected:
	// UserWidget
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Root;

	// UI
	TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> LayerMap;

	FDelegateHandle ScriptInitializedDelegateHandle;

	bool bInitializedFromWorldSetting = false;
};
