#include "DMUIScreen.h"
#include "DMUILayout.h"
#include "DMUISubsystem.h"

UDMUIScreen* UDMUIScreen::GetUIScreen(UObject* InContext)
{
	UDMUISubsystem* UIManager = InContext->GetWorld()->GetGameInstance()->GetSubsystem<UDMUISubsystem>();
	return UIManager->GetCurrentScreen();
}

void UDMUIScreen::AddLayoutToViewport(ULocalPlayer* InLocalPlayer, UDMUILayout* Layout)
{
	if (IsValid(Layout))
	{
		Layout->SetPlayerContext(InLocalPlayer);
		Layout->AddToPlayerScreen(1000);
	}
}

void UDMUIScreen::RegisterLayoutForLocalPlayer(ULocalPlayer* InLocalPlayer)
{
	if (!IsValid(InLocalPlayer))
	{
		return;
	}

	UDMUILayout** LayoutPtr = LayoutEntryMap.Find(InLocalPlayer);
	UDMUILayout* Layout = LayoutPtr ? *LayoutPtr : nullptr;
	APlayerController* PlayerController = InLocalPlayer->GetPlayerController(GetWorld());
	
	// If the recorded layout is invalid or belongs to a different PlayerController, we should recreate it to prevent memory leaks/dangling outer worlds.
	if (LayoutPtr && (!IsValid(Layout) || (PlayerController && Layout->GetOwningPlayer() != PlayerController)))
	{
		if (IsValid(Layout))
		{
			Layout->RemoveFromParent();
		}
		LayoutEntryMap.Remove(InLocalPlayer);
		Layout = nullptr;
		LayoutPtr = nullptr;
	}

	if (!Layout)
	{
		if (PlayerController)
		{
			if (UClass* LayoutClass = DefaultUILayoutClass.LoadSynchronous())
			{
				Layout = CreateWidget<UDMUILayout>(PlayerController, LayoutClass);
				LayoutEntryMap.Add(InLocalPlayer, Layout);
			}
		}
	}

	if (IsValid(Layout))
	{
		AddLayoutToViewport(InLocalPlayer, Layout);
	}
}

UDMUILayout* UDMUIScreen::GetLayoutFromLocalPlayer(ULocalPlayer* InLocalPlayer)
{
	if (UDMUILayout** LayoutPtr = LayoutEntryMap.Find(InLocalPlayer))
	{
		return *LayoutPtr;
	}

	return nullptr;
}
