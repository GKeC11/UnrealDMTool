#include "DMUILibrary.h"
#include "DMToolBox/Framework/Config/DMToolBoxDeveloperSetting.h"
#include "DMToolBox/Framework/UI/DMUIScreen.h"
#include "DMToolBox/Framework/UI/DMWidgetConfig.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"

namespace DMUILibraryPrivate
{
	static TSubclassOf<UCommonActivatableWidget> ResolveWidgetClassByTag(FGameplayTag InWidgetTag)
	{
		if (!InWidgetTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveWidgetClassByTag failed: WidgetTag is invalid."));
			return nullptr;
		}

		const UDMToolBoxDeveloperSetting* DMToolBoxDeveloperSettings = GetDefault<UDMToolBoxDeveloperSetting>();
		if (!DMToolBoxDeveloperSettings)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveWidgetClassByTag failed: DMToolBoxDeveloperSettings is null. WidgetTag=%s"),
				*InWidgetTag.ToString());
			return nullptr;
		}

		UDataTable* WidgetDataTable = DMToolBoxDeveloperSettings->WidgetConfigDataTable.LoadSynchronous();
		if (!WidgetDataTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveWidgetClassByTag failed: WidgetConfigDataTable load failed. WidgetTag=%s"),
				*InWidgetTag.ToString());
			return nullptr;
		}

		const TMap<FName, uint8*>& RowMap = WidgetDataTable->GetRowMap();
		for (const TPair<FName, uint8*>& RowPair : RowMap)
		{
			const FDMWidgetConfig* Row = reinterpret_cast<const FDMWidgetConfig*>(RowPair.Value);
			if (!Row || Row->WidgetTag != InWidgetTag)
			{
				continue;
			}

			UClass* WidgetClass = Row->WidgetClass.LoadSynchronous();
			if (WidgetClass && WidgetClass->IsChildOf(UCommonActivatableWidget::StaticClass()))
			{
				UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] ResolveWidgetClassByTag success: WidgetTag=%s, WidgetClass=%s"),
					*InWidgetTag.ToString(),
					*WidgetClass->GetName());
				return WidgetClass;
			}

			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveWidgetClassByTag failed: WidgetClass is invalid or not a UCommonActivatableWidget. WidgetTag=%s, RowName=%s"),
				*InWidgetTag.ToString(),
				*RowPair.Key.ToString());
			return nullptr;
		}

		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveWidgetClassByTag failed: No row matched WidgetTag=%s"),
			*InWidgetTag.ToString());
		return nullptr;
	}

	static UCommonActivatableWidgetContainerBase* ResolveLayerForLocalPlayer(APlayerController* PlayerController, FGameplayTag InLayerTag)
	{
		if (!IsValid(PlayerController) || !PlayerController->GetLocalPlayer())
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveLayerForLocalPlayer failed: PlayerController or LocalPlayer is invalid. PC=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*InLayerTag.ToString());
			return nullptr;
		}

		UDMUIScreen* Screen = UDMUIScreen::GetUIScreen(PlayerController);
		if (!Screen)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveLayerForLocalPlayer failed: Screen is invalid. PC=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*InLayerTag.ToString());
			return nullptr;
		}

		UDMUILayout* Layout = Screen->GetLayoutFromLocalPlayer(PlayerController->GetLocalPlayer());
		if (!Layout)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] ResolveLayerForLocalPlayer failed: Layout is invalid. PC=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*InLayerTag.ToString());
			return nullptr;
		}

		return Layout->GetLayerFromGameplayTag(InLayerTag);
	}
}

void UDMUILibrary::CreateWidgetToLayer(APlayerController* InPlayerController, TSubclassOf<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerTag)
{
	if (!IsValid(InPlayerController) || !InWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetToLayer skipped: Invalid PlayerController or WidgetClass. LayerTag=%s"),
			*InLayerTag.ToString());
		return;
	}

	UDMUIScreen* Screen = UDMUIScreen::GetUIScreen(InPlayerController);
	if (!Screen || !InPlayerController->GetLocalPlayer())
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetToLayer failed: Screen or LocalPlayer is invalid. PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(InPlayerController),
			*GetNameSafe(InWidgetClass.Get()),
			*InLayerTag.ToString());
		return;
	}

	UDMUILayout* Layout = Screen->GetLayoutFromLocalPlayer(InPlayerController->GetLocalPlayer());
	if (!Layout)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetToLayer failed: Layout is invalid. PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(InPlayerController),
			*GetNameSafe(InWidgetClass.Get()),
			*InLayerTag.ToString());
		return;
	}

	UCommonActivatableWidgetContainerBase* Layer = Layout->GetLayerFromGameplayTag(InLayerTag);
	if (!Layer)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetToLayer failed: Layer not found. PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(InPlayerController),
			*GetNameSafe(InWidgetClass.Get()),
			*InLayerTag.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] CreateWidgetToLayer add widget: PC=%s, WidgetClass=%s, LayerTag=%s"),
		*GetNameSafe(InPlayerController),
		*GetNameSafe(InWidgetClass.Get()),
		*InLayerTag.ToString());
	Layer->AddWidget(InWidgetClass);
}

bool UDMUILibrary::CreateWidgetByTagToLayer(UObject* WorldContextObject, FGameplayTag InWidgetTag, FGameplayTag InLayerTag)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetByTagToLayer failed: World is invalid. WidgetTag=%s, LayerTag=%s, Context=%s"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] CreateWidgetByTagToLayer begin: WidgetTag=%s, LayerTag=%s, Context=%s"),
		*InWidgetTag.ToString(),
		*InLayerTag.ToString(),
		*GetNameSafe(WorldContextObject));

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = DMUILibraryPrivate::ResolveWidgetClassByTag(InWidgetTag);
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetByTagToLayer failed: ResolveWidgetClassByTag returned null. WidgetTag=%s, LayerTag=%s"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
		return false;
	}

	bool bCreatedAnyWidget = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
		{
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] CreateWidgetByTagToLayer process local controller: PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(WidgetClass.Get()),
			*InLayerTag.ToString());
		CreateWidgetToLayer(PlayerController, WidgetClass, InLayerTag);
		bCreatedAnyWidget = true;
	}

	if (bCreatedAnyWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] CreateWidgetByTagToLayer end: WidgetTag=%s, LayerTag=%s, CreatedAnyWidget=true"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] CreateWidgetByTagToLayer end: WidgetTag=%s, LayerTag=%s, CreatedAnyWidget=false"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	return bCreatedAnyWidget;
}

bool UDMUILibrary::RemoveWidgetFromLayer(UObject* WorldContextObject, UCommonActivatableWidget* InWidget, FGameplayTag InLayerTag)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetFromLayer failed: World is invalid. Widget=%s, LayerTag=%s, Context=%s"),
			*GetNameSafe(InWidget),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	if (!IsValid(InWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetFromLayer failed: Widget is invalid. LayerTag=%s, Context=%s"),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	bool bRemovedAnyWidget = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
		{
			continue;
		}

		UCommonActivatableWidgetContainerBase* Layer = DMUILibraryPrivate::ResolveLayerForLocalPlayer(PlayerController, InLayerTag);
		if (!Layer)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetFromLayer skipped: Layer not found. PC=%s, Widget=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*GetNameSafe(InWidget),
				*InLayerTag.ToString());
			continue;
		}

		const bool bContainsWidget = Layer->GetWidgetList().Contains(InWidget);
		if (!bContainsWidget)
		{
			continue;
		}

		Layer->RemoveWidget(*InWidget);
		UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] RemoveWidgetFromLayer removed widget: PC=%s, Widget=%s, LayerTag=%s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(InWidget),
			*InLayerTag.ToString());
		bRemovedAnyWidget = true;
	}

	if (!bRemovedAnyWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetFromLayer end: Widget=%s, LayerTag=%s, RemovedAnyWidget=false"),
			*GetNameSafe(InWidget),
			*InLayerTag.ToString());
	}

	return bRemovedAnyWidget;
}

bool UDMUILibrary::RemoveWidgetByTagFromLayer(UObject* WorldContextObject, FGameplayTag InWidgetTag, FGameplayTag InLayerTag)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetByTagFromLayer failed: World is invalid. WidgetTag=%s, LayerTag=%s, Context=%s"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] RemoveWidgetByTagFromLayer begin: WidgetTag=%s, LayerTag=%s, Context=%s"),
		*InWidgetTag.ToString(),
		*InLayerTag.ToString(),
		*GetNameSafe(WorldContextObject));

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = DMUILibraryPrivate::ResolveWidgetClassByTag(InWidgetTag);
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetByTagFromLayer failed: ResolveWidgetClassByTag returned null. WidgetTag=%s, LayerTag=%s"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
		return false;
	}

	bool bRemovedAnyWidget = false;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		if (!IsValid(PlayerController) || !PlayerController->IsLocalController())
		{
			continue;
		}

		UCommonActivatableWidgetContainerBase* Layer = DMUILibraryPrivate::ResolveLayerForLocalPlayer(PlayerController, InLayerTag);
		if (!Layer)
		{
			UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetByTagFromLayer skipped: Layer not found. PC=%s, WidgetTag=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*InWidgetTag.ToString(),
				*InLayerTag.ToString());
			continue;
		}

		TArray<UCommonActivatableWidget*> WidgetsToRemove;
		for (UCommonActivatableWidget* Widget : Layer->GetWidgetList())
		{
			if (!IsValid(Widget) || !Widget->IsA(WidgetClass))
			{
				continue;
			}

			WidgetsToRemove.Add(Widget);
		}

		for (UCommonActivatableWidget* Widget : WidgetsToRemove)
		{
			bRemovedAnyWidget |= RemoveWidgetFromLayer(WorldContextObject, Widget, InLayerTag);
		}
	}

	if (bRemovedAnyWidget)
	{
		UE_LOG(LogTemp, Log, TEXT("[DMUILibrary] RemoveWidgetByTagFromLayer end: WidgetTag=%s, LayerTag=%s, RemovedAnyWidget=true"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DMUILibrary] RemoveWidgetByTagFromLayer end: WidgetTag=%s, LayerTag=%s, RemovedAnyWidget=false"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	return bRemovedAnyWidget;
}
