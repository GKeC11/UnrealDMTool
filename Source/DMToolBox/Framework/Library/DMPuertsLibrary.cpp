#include "DMPuertsLibrary.h"

void UDMPuertsLibrary::RegisterConsoleCommand(const FString& CommandName, const FString& CommandDesc, const FConsoleCommandDynamicDelegate& CommandDelegate)
{
	IConsoleManager::Get().RegisterConsoleCommand(
		*CommandName,
		*CommandDesc,
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda(
			[CommandDelegate](const TArray<FString>& Args, TWeakObjectPtr<UWorld> InWorld)
			{
				if (InWorld.IsValid()) CommandDelegate.ExecuteIfBound(InWorld.Get(), Args);
			}));
}
