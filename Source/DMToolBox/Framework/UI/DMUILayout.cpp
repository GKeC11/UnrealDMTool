#include "DMUILayout.h"
#include "DMActivatableWidgetContainer.h"
#include "DMToolBox/Framework/Library/DMSystemLibrary.h"

UCommonActivatableWidgetContainerBase* UDMUILayout::GetLayerFromGameplayTag(FGameplayTag InTag)
{
	return LayerMap[InTag];
}

void UDMUILayout::NativeConstruct()
{
	Super::NativeConstruct();

	TArray<UWidget*> Widgets = Overlay_Root->GetAllChildren();
	for (UWidget* Widget : Widgets)
	{
		if (IDMActivatableWidgetContainer* ActivatableWidgetContainer = Cast<IDMActivatableWidgetContainer>(Widget))
		{
			RegisterLayer(ActivatableWidgetContainer->GetLayerTag(), ActivatableWidgetContainer->GetContainer());
		}
	}

	InitializeFromWorldSetting();
}

void UDMUILayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	LayerMap.Add(LayerTag, LayerWidget);
}

void UDMUILayout::InitializeFromWorldSetting()
{
	if (UDMLevelInitializationSetting* LevelInitializationSetting = UDMSystemLibrary::GetLevelInitializationSetting(GetWorld()))
	{
		TArray<FLevelInitializationSetting_WidgetConfig> WidgetConfigs = LevelInitializationSetting->WidgetConfigs;
		for (FLevelInitializationSetting_WidgetConfig& WidgetConfig : WidgetConfigs)
		{
			UCommonActivatableWidgetContainerBase* Layer = GetLayerFromGameplayTag(WidgetConfig.Layer);
			Layer->AddWidget(WidgetConfig.WidgetClass);
		}
	}
}
