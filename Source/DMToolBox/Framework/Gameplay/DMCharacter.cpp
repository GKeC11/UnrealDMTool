
#include "DMCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

ADMCharacter::ADMCharacter(const FObjectInitializer& ObjectInitializer)
{
	EnhancedInputHelper = CreateDefaultSubobject<UEnhancedInputHelperComponent>(TEXT("EnhancedInputHelperComponent"));

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ADMCharacter::OnMoveTriggered(FInputActionValue ActionValue, float ElapsedTime, float TriggeredTime, const UInputAction* SourceAction)
{
	FVector2D PlayerInputValue = ActionValue.Get<FVector2D>();
	FVector MoveVelocity = FVector(-PlayerInputValue.X, PlayerInputValue.Y, 0);
	AddMovementInput(MoveVelocity);
}

