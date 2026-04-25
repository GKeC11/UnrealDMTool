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

	PostWorldInitializationDelegateHandle = FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::HandlePostWorldInitialization);
	WorldCleanupDelegateHandle = FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::HandleWorldCleanup);
}

void UDMUISubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.Remove(PostWorldInitializationDelegateHandle);
	FWorldDelegates::OnWorldCleanup.Remove(WorldCleanupDelegateHandle);

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

void UDMUISubsystem::HandlePostWorldInitialization(UWorld* InitializedWorld, const UWorld::InitializationValues InitializationValues)
{
	(void)InitializationValues;

	MarkLevelTravelCompleted(InitializedWorld);
}

void UDMUISubsystem::HandleWorldCleanup(UWorld* CleanupWorld, bool bSessionEnded, bool bCleanupResources)
{
	(void)bSessionEnded;
	(void)bCleanupResources;

	if (CleanupWorld && CleanupWorld->GetGameInstance() == GetGameInstance())
	{
		bLevelTravelCompleted = false;
	}
}

void UDMUISubsystem::MarkLevelTravelCompleted(UWorld* LoadedWorld)
{
	if (!LoadedWorld || LoadedWorld->GetGameInstance() != GetGameInstance() || bLevelTravelCompleted)
	{
		return;
	}

	bLevelTravelCompleted = true;
	OnLevelTravelCompleted.Broadcast();
}
