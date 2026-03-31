#include "DMUISubsystem.h"

#include "DMToolBox/Framework/Config/DMToolBoxDeveloperSetting.h"

void UDMUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UDMToolBoxDeveloperSetting* DMToolBoxDeveloperSettings = GetDefault<UDMToolBoxDeveloperSetting>();
	check(DMToolBoxDeveloperSettings);
	
	if (UClass* UIScreenClass = DMToolBoxDeveloperSettings->DefaultUIScreenClass.LoadSynchronous())
	{
		CurrentScreen = NewObject<UDMUIScreen>(this, UIScreenClass);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::OnLocalPlayerAddedEvent);
	}
}

void UDMUISubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		TArray<ULocalPlayer*> LocalPlayers = GameInstance->GetLocalPlayers();
		for (ULocalPlayer* LocalPlayer : LocalPlayers)
		{
			LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);
		}
		GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
	}
}

void UDMUISubsystem::OnLocalPlayerAddedEvent(ULocalPlayer* InLocalPlayer)
{
	check(CurrentScreen)

	InLocalPlayer->OnPlayerControllerChanged().AddWeakLambda(this, [this](const APlayerController* InPlayerController)
	{
		CurrentScreen->RegisterLayoutForLocalPlayer(InPlayerController->GetLocalPlayer());
	});
}
