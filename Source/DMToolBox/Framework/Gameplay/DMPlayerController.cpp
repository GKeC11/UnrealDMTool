
#include "DMPlayerController.h"

#include "DMCharacter.h"

void ADMPlayerController::RegisterCurrentPawnInput()
{
	if (ADMCharacter* NewCharacter = Cast<ADMCharacter>(GetPawn()))
	{
		NewCharacter->GetEnhancedInputHelper()->RegisterEnhancedInput(this);
	}
}

void ADMPlayerController::UnregisterCurrentPawnInput()
{
	if (GetPawn())
	{
		if (ADMCharacter* ControlledCharacter = Cast<ADMCharacter>(GetPawn()))
		{
			ControlledCharacter->GetEnhancedInputHelper()->UnregisterEnhancedInput(this);
		}
	}
}
