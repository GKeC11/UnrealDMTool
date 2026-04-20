#include "DMUILibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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
			return nullptr;
		}

		const UDMToolBoxDeveloperSetting* DMToolBoxDeveloperSettings = GetDefault<UDMToolBoxDeveloperSetting>();
		if (!DMToolBoxDeveloperSettings)
		{
			return nullptr;
		}

		UDataTable* WidgetDataTable = DMToolBoxDeveloperSettings->WidgetConfigDataTable.LoadSynchronous();
		if (!WidgetDataTable)
		{
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

			UClass* WidgetClass = Row->WidgetClass.Get();
			if (WidgetClass && WidgetClass->IsChildOf(UCommonActivatableWidget::StaticClass()))
			{
				return WidgetClass;
			}

			return nullptr;
		}

		return nullptr;
	}
}

void UDMUILibrary::CreateWidgetToLayer(APlayerController* InPlayerController, TSubclassOf<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerTag)
{
	if (!IsValid(InPlayerController) || !InWidgetClass)
	{
		return;
	}

	UDMUIScreen* Screen = UDMUIScreen::GetUIScreen(InPlayerController);
	if (!Screen || !InPlayerController->GetLocalPlayer())
	{
		return;
	}

	Screen->RegisterLayoutForLocalPlayer(InPlayerController->GetLocalPlayer());
	UDMUILayout* Layout = Screen->GetLayoutFromLocalPlayer(InPlayerController->GetLocalPlayer());
	if (!Layout)
	{
		return;
	}

	UCommonActivatableWidgetContainerBase* Layer = Layout->GetLayerFromGameplayTag(InLayerTag);
	if (!Layer)
	{
		return;
	}

	Layer->AddWidget(InWidgetClass);
}

bool UDMUILibrary::CreateWidgetByTagToLayer(UObject* WorldContextObject, FGameplayTag InWidgetTag, FGameplayTag InLayerTag)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = DMUILibraryPrivate::ResolveWidgetClassByTag(InWidgetTag);
	if (!WidgetClass)
	{
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

		CreateWidgetToLayer(PlayerController, WidgetClass, InLayerTag);
		bCreatedAnyWidget = true;
	}

	return bCreatedAnyWidget;
}

bool UDMUILibrary::RemoveWidgetByTag(UObject* WorldContextObject, FGameplayTag InWidgetTag)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = DMUILibraryPrivate::ResolveWidgetClassByTag(InWidgetTag);
	if (!WidgetClass)
	{
		return false;
	}

	TArray<UUserWidget*> FoundWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(World, FoundWidgets, WidgetClass, false);

	bool bRemovedAnyWidget = false;
	for (UUserWidget* Widget : FoundWidgets)
	{
		if (!IsValid(Widget))
		{
			continue;
		}

		if (UCommonActivatableWidget* ActivatableWidget = Cast<UCommonActivatableWidget>(Widget))
		{
			ActivatableWidget->DeactivateWidget();
		}

		Widget->RemoveFromParent();
		bRemovedAnyWidget = true;
	}

	return bRemovedAnyWidget;
}
