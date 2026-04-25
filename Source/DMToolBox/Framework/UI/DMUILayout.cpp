#include "DMUILayout.h"

#include "DMActivatableWidgetContainer.h"
#include "DMToolBox/Framework/Gameplay/Core/DMGameInstance.h"
#include "DMToolBox/Framework/Library/DMSystemLibrary.h"
#include "DMToolBox/Framework/UI/DMUISubsystem.h"

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
	RemoveReadinessDelegates();

	Super::NativeDestruct();
}

void UDMUILayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	LayerMap.Add(LayerTag, LayerWidget);
}

void UDMUILayout::InitializeWhenReady()
{
	UDMGameInstance* GameInstance = Cast<UDMGameInstance>(GetGameInstance());
	UDMUISubsystem* UISubsystem = GameInstance ? GameInstance->GetSubsystem<UDMUISubsystem>() : nullptr;
	if (!GameInstance || !UISubsystem)
	{
		return;
	}

	if (GameInstance->IsScriptInitialized() && UISubsystem->IsLevelTravelCompleted())
	{
		InitializeFromWorldSetting();
		return;
	}

	// Initial widgets rely on both world settings and Puerts mixins, so wait for both lifecycle gates.
	if (!GameInstance->IsScriptInitialized() && !ScriptInitializedDelegateHandle.IsValid())
	{
		ScriptInitializedDelegateHandle = GameInstance->OnScriptInitialized.AddUObject(this, &ThisClass::InitializeWhenReady);
	}

	if (!UISubsystem->IsLevelTravelCompleted() && !LevelTravelCompletedDelegateHandle.IsValid())
	{
		LevelTravelCompletedDelegateHandle = UISubsystem->OnLevelTravelCompleted.AddUObject(this, &ThisClass::InitializeWhenReady);
	}
}

void UDMUILayout::RemoveReadinessDelegates()
{
	if (UDMGameInstance* GameInstance = Cast<UDMGameInstance>(GetGameInstance()))
	{
		GameInstance->OnScriptInitialized.Remove(ScriptInitializedDelegateHandle);

		if (UDMUISubsystem* UISubsystem = GameInstance->GetSubsystem<UDMUISubsystem>())
		{
			UISubsystem->OnLevelTravelCompleted.Remove(LevelTravelCompletedDelegateHandle);
		}
	}
	ScriptInitializedDelegateHandle.Reset();
	LevelTravelCompletedDelegateHandle.Reset();
}

void UDMUILayout::InitializeFromWorldSetting()
{
	if (bInitializedFromWorldSetting)
	{
		return;
	}

	bInitializedFromWorldSetting = true;
	RemoveReadinessDelegates();

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
