#include "DMGameplayLibrary.h"

#include "EnhancedInputSubsystems.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

void UDMGameplayLibrary::RegisterInputMappingContext(APlayerController* InPlayerController, UInputMappingContext* InputMappingContextToRegister)
{
	if (ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			EnhancedInputSubsystem->AddMappingContext(InputMappingContextToRegister, 0);
		}
	}
}

void UDMGameplayLibrary::UnregisterInputMappingContext(APlayerController* InPlayerController, UInputMappingContext* InputMappingContextToRegister)
{
	if (ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer())
	{
		ULocalPlayerSubsystem* LocalPlayerSubsystem = USubsystemBlueprintLibrary::GetLocalPlayerSubsystem(LocalPlayer, UEnhancedInputLocalPlayerSubsystem::StaticClass());
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			EnhancedInputSubsystem->RemoveMappingContext(InputMappingContextToRegister);
		}
	}
}
