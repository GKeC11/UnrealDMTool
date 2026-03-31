#pragma once

#include "DMPlayerController.generated.h"

UCLASS()
class ADMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Player Controller
	void RegisterCurrentPawnInput();
	
	void UnregisterCurrentPawnInput();
};
