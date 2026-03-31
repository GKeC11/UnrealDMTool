#include "EnhancedInputHelperComponent.h"

#include "EnhancedInputComponent.h"
#include "DMToolBox/Framework/Library/DMGameplayLibrary.h"

void UEnhancedInputHelperComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEnhancedInputHelperComponent::RegisterEnhancedInput(APlayerController* InPlayerController)
{
	RegisterInputMappingContext(InPlayerController);
	RegisterInputActions();
}

void UEnhancedInputHelperComponent::UnregisterEnhancedInput(APlayerController* InPlayerController)
{
	UnregisterInputMappingContext(InPlayerController);
	UnregisterInputActions();
}

void UEnhancedInputHelperComponent::RegisterInputMappingContext(APlayerController* InPlayerController)
{
	if (InputMappingContextToRegister.LoadSynchronous())
	{
		if (InPlayerController)
		{
			UDMGameplayLibrary::RegisterInputMappingContext(InPlayerController, InputMappingContextToRegister.LoadSynchronous());
		}
	}
}

void UEnhancedInputHelperComponent::UnregisterInputMappingContext(APlayerController* InPlayerController)
{
	if (InputMappingContextToRegister.LoadSynchronous())
	{
		if (InPlayerController)
		{
			UDMGameplayLibrary::UnregisterInputMappingContext(InPlayerController, InputMappingContextToRegister.LoadSynchronous());
		}
	}
}

void UEnhancedInputHelperComponent::RegisterInputActions()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent))
	{
		for (FEnhancedInputHelperInputActionConfig& InputActionConfig : InputActionConfigs)
		{
			if (InputActionConfig.InputAction.LoadSynchronous())
			{
				EnhancedInputComponent->BindAction(InputActionConfig.InputAction.LoadSynchronous(),
				                                   ETriggerEvent::Triggered,
				                                   GetOwner(),
				                                   FName(InputActionConfig.FunctionToBind));
			}
		}
	}
}

void UEnhancedInputHelperComponent::UnregisterInputActions()
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(GetOwner()->InputComponent))
	{
		EnhancedInputComponent->ClearActionBindings();
	}
}

TArray<FString> UEnhancedInputHelperComponent::GetFunctionNamesForInputAction()
{
	TArray<FString> FunctionNames;
	UClass* OwnerClass = GetOwner()->GetClass();
	for (TFieldIterator<UFunction> It(OwnerClass, EFieldIterationFlags::IncludeSuper); It; ++It)
	{
		if (UFunction* Function = *It)
		{
			FunctionNames.Add(Function->GetName());
		}
	}

	return FunctionNames;
}
