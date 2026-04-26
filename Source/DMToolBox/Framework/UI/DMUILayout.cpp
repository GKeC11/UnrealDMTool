#include "DMUILayout.h"

#include "DMActivatableWidgetContainer.h"
#include "DMToolBox/Framework/Gameplay/Core/DMGameInstance.h"
#include "DMToolBox/Framework/Library/DMSystemLibrary.h"

UCommonActivatableWidgetContainerBase* UDMUILayout::GetLayerFromGameplayTag(FGameplayTag InTag)
{
	return LayerMap.FindRef(InTag);
}

void UDMUILayout::NativeConstruct()
{
	Super::NativeConstruct();

	LayerMap.Reset();

	TArray<UWidget*> Widgets = Overlay_Root->GetAllChildren();
	for (UWidget* Widget : Widgets)
	{
		if (IDMActivatableWidgetContainer* ActivatableWidgetContainer = Cast<IDMActivatableWidgetContainer>(Widget))
		{
			RegisterLayer(ActivatableWidgetContainer->GetLayerTag(), ActivatableWidgetContainer->GetContainer());
		}
	}

	InitializeWhenReady();
}

void UDMUILayout::NativeDestruct()
{
	RemoveScriptInitializedDelegate();
	bInitializedFromWorldSetting = false;

	Super::NativeDestruct();
}

void UDMUILayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	LayerMap.Add(LayerTag, LayerWidget);
}

void UDMUILayout::InitializeWhenReady()
{
	UDMGameInstance* GameInstance = Cast<UDMGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	if (GameInstance->IsScriptInitialized())
	{
		InitializeFromWorldSetting();
		return;
	}

	// Initial widgets rely on Puerts mixins, so wait until the script layer is ready.
	if (!GameInstance->IsScriptInitialized() && !ScriptInitializedDelegateHandle.IsValid())
	{
		ScriptInitializedDelegateHandle = GameInstance->OnScriptInitialized.AddUObject(this, &ThisClass::InitializeWhenReady);
	}
}

void UDMUILayout::RemoveScriptInitializedDelegate()
{
	if (UDMGameInstance* GameInstance = Cast<UDMGameInstance>(GetGameInstance()))
	{
		GameInstance->OnScriptInitialized.Remove(ScriptInitializedDelegateHandle);
	}
	ScriptInitializedDelegateHandle.Reset();
}

void UDMUILayout::InitializeFromWorldSetting()
{
	if (bInitializedFromWorldSetting)
	{
		return;
	}

	bInitializedFromWorldSetting = true;
	RemoveScriptInitializedDelegate();

	if (UDMLevelInitializationSetting* LevelInitializationSetting = UDMSystemLibrary::GetLevelInitializationSetting(GetWorld()))
	{
		TArray<FLevelInitializationSetting_WidgetConfig> WidgetConfigs = LevelInitializationSetting->WidgetConfigs;
		for (FLevelInitializationSetting_WidgetConfig& WidgetConfig : WidgetConfigs)
		{
			UCommonActivatableWidgetContainerBase* Layer = GetLayerFromGameplayTag(WidgetConfig.Layer);
			if (Layer)
			{
				Layer->AddWidget(WidgetConfig.WidgetClass);
			}
		}
	}
}
