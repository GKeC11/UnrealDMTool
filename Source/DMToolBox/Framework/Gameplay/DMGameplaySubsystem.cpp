#include "DMGameplaySubsystem.h"

#include "DMToolBox/Framework/Gameplay/DMCameraActor.h"
#include "DMToolBox/Framework/Library/DMSystemLibrary.h"

void UDMGameplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &ThisClass::OnLocalPlayerAddedEvent);
	}
}

void UDMGameplaySubsystem::Deinitialize()
{
	Super::Deinitialize();

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
	}
}

void UDMGameplaySubsystem::OnLocalPlayerAddedEvent(ULocalPlayer* InLocalPlayer)
{
	InLocalPlayer->OnPlayerControllerChanged().AddWeakLambda(this, [this](const APlayerController* InPlayerController)
	{
		if (UDMLevelInitializationSetting* LevelInitializationSetting = UDMSystemLibrary::GetLevelInitializationSetting(GetWorld()))
		{
			if (TSubclassOf<ADMCameraActor> CameraActorClass = LevelInitializationSetting->CameraActorClass)
			{
				ADMCameraActor* CameraActor = GetWorld()->SpawnActor<ADMCameraActor>(CameraActorClass);
				CameraActor->InitializeFromPlayerController(InPlayerController);
			}
		}
	});
}
