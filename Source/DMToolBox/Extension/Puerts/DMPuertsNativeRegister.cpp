#include "DMPuertsNativeRegister.h"

#include "UEDataBinding.hpp"
#include "GameFramework/GameplayMessageSubsystem.h"

UsingUClass(UGameplayMessageSubsystem)
UsingUStruct(FGameplayTag)
UsingUClass(UScriptStruct)

struct DMPuertsNativeRegister
{
	DMPuertsNativeRegister()
	{
		puerts::DefineClass<UGameplayMessageSubsystem>()
		.Function("Generic_BroadcastMessage", MakeFunction(&UGameplayMessageSubsystem::Generic_BroadcastMessage))
		.Register();
	}
};

DMPuertsNativeRegister _DMPuertsNativeRegister__;