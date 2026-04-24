#include "DMUILibrary.h"
#include "DMToolBox/Framework/Common/DMMacros.h"
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
			DM_LOG(nullptr, LogTemp, Warning, TEXT("ResolveWidgetClassByTag failed: WidgetTag is invalid."));
			return nullptr;
		}

		const UDMToolBoxDeveloperSetting* DMToolBoxDeveloperSettings = GetDefault<UDMToolBoxDeveloperSetting>();
		if (!DMToolBoxDeveloperSettings)
		{
			DM_LOG(nullptr, LogTemp, Warning, TEXT("ResolveWidgetClassByTag failed: DMToolBoxDeveloperSettings is null. WidgetTag=%s"),
				*InWidgetTag.ToString());
			return nullptr;
		}

		UDataTable* WidgetDataTable = DMToolBoxDeveloperSettings->WidgetConfigDataTable.LoadSynchronous();
		if (!WidgetDataTable)
		{
			DM_LOG(nullptr, LogTemp, Warning, TEXT("ResolveWidgetClassByTag failed: WidgetConfigDataTable load failed. WidgetTag=%s"),
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
				DM_LOG(nullptr, LogTemp, Log, TEXT("ResolveWidgetClassByTag success: WidgetTag=%s, WidgetClass=%s"),
					*InWidgetTag.ToString(),
					*WidgetClass->GetName());
				return WidgetClass;
			}

			DM_LOG(nullptr, LogTemp, Warning, TEXT("ResolveWidgetClassByTag failed: WidgetClass is invalid or not a UCommonActivatableWidget. WidgetTag=%s, RowName=%s"),
				*InWidgetTag.ToString(),
				*RowPair.Key.ToString());
			return nullptr;
		}

		DM_LOG(nullptr, LogTemp, Warning, TEXT("ResolveWidgetClassByTag failed: No row matched WidgetTag=%s"),
			*InWidgetTag.ToString());
		return nullptr;
	}

	static UCommonActivatableWidgetContainerBase* ResolveLayerForLocalPlayer(APlayerController* PlayerController, FGameplayTag InLayerTag)
	{
		if (!IsValid(PlayerController) || !PlayerController->GetLocalPlayer())
		{
			DM_LOG(PlayerController, LogTemp, Warning, TEXT("ResolveLayerForLocalPlayer failed: PlayerController or LocalPlayer is invalid. PC=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*InLayerTag.ToString());
			return nullptr;
		}

		UDMUIScreen* Screen = UDMUIScreen::GetUIScreen(PlayerController);
		if (!Screen)
		{
			DM_LOG(PlayerController, LogTemp, Warning, TEXT("ResolveLayerForLocalPlayer failed: Screen is invalid. PC=%s, LayerTag=%s"),
				*GetNameSafe(PlayerController),
				*InLayerTag.ToString());
			return nullptr;
		}

		UDMUILayout* Layout = Screen->GetLayoutFromLocalPlayer(PlayerController->GetLocalPlayer());
		if (!Layout)
		{
			DM_LOG(PlayerController, LogTemp, Warning, TEXT("ResolveLayerForLocalPlayer failed: Layout is invalid. PC=%s, LayerTag=%s"),
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
		DM_LOG(InPlayerController, LogTemp, Warning, TEXT("CreateWidgetToLayer skipped: Invalid PlayerController or WidgetClass. LayerTag=%s"),
			*InLayerTag.ToString());
		return;
	}

	UDMUIScreen* Screen = UDMUIScreen::GetUIScreen(InPlayerController);
	if (!Screen || !InPlayerController->GetLocalPlayer())
	{
		DM_LOG(InPlayerController, LogTemp, Warning, TEXT("CreateWidgetToLayer failed: Screen or LocalPlayer is invalid. PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(InPlayerController),
			*GetNameSafe(InWidgetClass.Get()),
			*InLayerTag.ToString());
		return;
	}

	UDMUILayout* Layout = Screen->GetLayoutFromLocalPlayer(InPlayerController->GetLocalPlayer());
	if (!Layout)
	{
		DM_LOG(InPlayerController, LogTemp, Warning, TEXT("CreateWidgetToLayer failed: Layout is invalid. PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(InPlayerController),
			*GetNameSafe(InWidgetClass.Get()),
			*InLayerTag.ToString());
		return;
	}

	UCommonActivatableWidgetContainerBase* Layer = Layout->GetLayerFromGameplayTag(InLayerTag);
	if (!Layer)
	{
		DM_LOG(InPlayerController, LogTemp, Warning, TEXT("CreateWidgetToLayer failed: Layer not found. PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(InPlayerController),
			*GetNameSafe(InWidgetClass.Get()),
			*InLayerTag.ToString());
		return;
	}

	DM_LOG(InPlayerController, LogTemp, Log, TEXT("CreateWidgetToLayer add widget: PC=%s, WidgetClass=%s, LayerTag=%s"),
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
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("CreateWidgetByTagToLayer failed: World is invalid. WidgetTag=%s, LayerTag=%s, Context=%s"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	DM_LOG(WorldContextObject, LogTemp, Log, TEXT("CreateWidgetByTagToLayer begin: WidgetTag=%s, LayerTag=%s, Context=%s"),
		*InWidgetTag.ToString(),
		*InLayerTag.ToString(),
		*GetNameSafe(WorldContextObject));

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = DMUILibraryPrivate::ResolveWidgetClassByTag(InWidgetTag);
	if (!WidgetClass)
	{
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("CreateWidgetByTagToLayer failed: ResolveWidgetClassByTag returned null. WidgetTag=%s, LayerTag=%s"),
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

		DM_LOG(PlayerController, LogTemp, Log, TEXT("CreateWidgetByTagToLayer process local controller: PC=%s, WidgetClass=%s, LayerTag=%s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(WidgetClass.Get()),
			*InLayerTag.ToString());
		CreateWidgetToLayer(PlayerController, WidgetClass, InLayerTag);
		bCreatedAnyWidget = true;
	}

	if (bCreatedAnyWidget)
	{
		DM_LOG(WorldContextObject, LogTemp, Log, TEXT("CreateWidgetByTagToLayer end: WidgetTag=%s, LayerTag=%s, CreatedAnyWidget=true"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	else
	{
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("CreateWidgetByTagToLayer end: WidgetTag=%s, LayerTag=%s, CreatedAnyWidget=false"),
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
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("RemoveWidgetFromLayer failed: World is invalid. Widget=%s, LayerTag=%s, Context=%s"),
			*GetNameSafe(InWidget),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	if (!IsValid(InWidget))
	{
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("RemoveWidgetFromLayer failed: Widget is invalid. LayerTag=%s, Context=%s"),
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
			DM_LOG(PlayerController, LogTemp, Warning, TEXT("RemoveWidgetFromLayer skipped: Layer not found. PC=%s, Widget=%s, LayerTag=%s"),
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
		DM_LOG(PlayerController, LogTemp, Log, TEXT("RemoveWidgetFromLayer removed widget: PC=%s, Widget=%s, LayerTag=%s"),
			*GetNameSafe(PlayerController),
			*GetNameSafe(InWidget),
			*InLayerTag.ToString());
		bRemovedAnyWidget = true;
	}

	if (!bRemovedAnyWidget)
	{
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("RemoveWidgetFromLayer end: Widget=%s, LayerTag=%s, RemovedAnyWidget=false"),
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
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("RemoveWidgetByTagFromLayer failed: World is invalid. WidgetTag=%s, LayerTag=%s, Context=%s"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString(),
			*GetNameSafe(WorldContextObject));
		return false;
	}

	DM_LOG(WorldContextObject, LogTemp, Log, TEXT("RemoveWidgetByTagFromLayer begin: WidgetTag=%s, LayerTag=%s, Context=%s"),
		*InWidgetTag.ToString(),
		*InLayerTag.ToString(),
		*GetNameSafe(WorldContextObject));

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = DMUILibraryPrivate::ResolveWidgetClassByTag(InWidgetTag);
	if (!WidgetClass)
	{
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("RemoveWidgetByTagFromLayer failed: ResolveWidgetClassByTag returned null. WidgetTag=%s, LayerTag=%s"),
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
			DM_LOG(PlayerController, LogTemp, Warning, TEXT("RemoveWidgetByTagFromLayer skipped: Layer not found. PC=%s, WidgetTag=%s, LayerTag=%s"),
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
		DM_LOG(WorldContextObject, LogTemp, Log, TEXT("RemoveWidgetByTagFromLayer end: WidgetTag=%s, LayerTag=%s, RemovedAnyWidget=true"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	else
	{
		DM_LOG(WorldContextObject, LogTemp, Warning, TEXT("RemoveWidgetByTagFromLayer end: WidgetTag=%s, LayerTag=%s, RemovedAnyWidget=false"),
			*InWidgetTag.ToString(),
			*InLayerTag.ToString());
	}
	return bRemovedAnyWidget;
}
