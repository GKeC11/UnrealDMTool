#include "DMUISubsystem.h"

#include "DMToolBox/Framework/Config/DMToolBoxDeveloperSetting.h"

bool UDMUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UGameInstance* GameInstance = Cast<UGameInstance>(Outer);
	return Super::ShouldCreateSubsystem(Outer) && GameInstance && !GameInstance->IsDedicatedServerInstance();
}

void UDMUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	const UDMToolBoxDeveloperSetting* DMToolBoxDeveloperSettings = GetDefault<UDMToolBoxDeveloperSetting>();
	check(DMToolBoxDeveloperSettings);

	if (UClass* UIScreenClass = DMToolBoxDeveloperSettings->DefaultUIScreenClass.LoadSynchronous())
	{
		CurrentScreen = NewObject<UDMUIScreen>(this, UIScreenClass);
	}

	GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::OnLocalPlayerAddedEvent);
}

void UDMUISubsystem::Deinitialize()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		TArray<ULocalPlayer*> LocalPlayers = GameInstance->GetLocalPlayers();
		for (ULocalPlayer* LocalPlayer : LocalPlayers)
		{
			LocalPlayer->OnPlayerControllerChanged().RemoveAll(this);
		}
		GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
	}

	Super::Deinitialize();
}

void UDMUISubsystem::OnLocalPlayerAddedEvent(ULocalPlayer* InLocalPlayer)
{
	check(CurrentScreen)

	InLocalPlayer->OnPlayerControllerChanged().AddWeakLambda(this, [this](const APlayerController* InPlayerController)
	{
		CurrentScreen->RegisterLayoutForLocalPlayer(InPlayerController->GetLocalPlayer());
	});
}
