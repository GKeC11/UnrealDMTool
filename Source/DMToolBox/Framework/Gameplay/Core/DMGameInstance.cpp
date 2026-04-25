#include "DMGameInstance.h"

#include "UObject/Class.h"

bool UDMGameInstance::AddPuertsLoadedClassReference(UClass* InClass)
{
	if (!IsValid(InClass))
	{
		return false;
	}

	PuertsLoadedClassReferences.AddUnique(InClass);

	return true;
}

void UDMGameInstance::OnStart()
{
	Super::OnStart();

	TS_Init();
}

void UDMGameInstance::NotifyScriptInitialized()
{
	if (bScriptInitialized)
	{
		return;
	}

	bScriptInitialized = true;
	OnScriptInitialized.Broadcast();
}
