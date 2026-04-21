#include "DMPuertsLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "DMToolBox/Framework/Gameplay/Core/DMGameInstance.h"
#include "Engine/Engine.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectIterator.h"

namespace DMPuertsLibraryPrivate
{
	static const FString MixinMethodSuffix = TEXT("__puerts_mixin__");

	static UDMGameInstance* ResolveDMGameInstance()
	{
		if (!GEngine)
		{
			return nullptr;
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			UWorld* World = WorldContext.World();
			if (!IsValid(World) || !World->IsGameWorld())
			{
				continue;
			}

			if (UDMGameInstance* GameInstance = Cast<UDMGameInstance>(World->GetGameInstance()))
			{
				return GameInstance;
			}
		}

		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			UWorld* World = WorldContext.World();
			if (!IsValid(World))
			{
				continue;
			}

			if (UDMGameInstance* GameInstance = Cast<UDMGameInstance>(World->GetGameInstance()))
			{
				return GameInstance;
			}
		}

		return nullptr;
	}
}

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

bool UDMPuertsLibrary::KeepLoadedClassReferenced(UClass* InClass)
{
	if (!IsValid(InClass))
	{
		return false;
	}

	if (UDMGameInstance* GameInstance = DMPuertsLibraryPrivate::ResolveDMGameInstance())
	{
		return GameInstance->AddPuertsLoadedClassReference(InClass);
	}

	UE_LOG(LogTemp, Warning, TEXT("[DMPuertsLibrary] Failed to keep loaded class referenced because no UDMGameInstance was available. Class=%s"),
		*InClass->GetPathName());
	return false;
}

FDMBlueprintClassLoadDiagnostic UDMPuertsLibrary::DiagnoseBlueprintClassLoad(const FString& ClassPath)
{
	FDMBlueprintClassLoadDiagnostic Diagnostic;
	Diagnostic.ClassPath = ClassPath;

	if (ClassPath.IsEmpty())
	{
		Diagnostic.Message = TEXT("Class path is empty.");
		return Diagnostic;
	}

	int32 DotIndex = INDEX_NONE;
	if (!ClassPath.FindLastChar(TEXT('.'), DotIndex))
	{
		Diagnostic.Message = TEXT("Class path is invalid because it does not contain a package/object separator.");
		return Diagnostic;
	}

	Diagnostic.PackagePath = ClassPath.Left(DotIndex);
	const FString AssetName = FPackageName::GetLongPackageAssetName(Diagnostic.PackagePath);
	Diagnostic.AssetObjectPath = FString::Printf(TEXT("%s.%s"), *Diagnostic.PackagePath, *AssetName);
	Diagnostic.bClassAlreadyLoaded = FindObject<UClass>(nullptr, *ClassPath) != nullptr;
	Diagnostic.bPackageExists = FPackageName::DoesPackageExist(Diagnostic.PackagePath);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssetsByPackageName(*Diagnostic.PackagePath, AssetDataList, true);
	Diagnostic.bAssetRegistered = AssetDataList.Num() > 0;

	if (UClass* LoadedClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassPath))
	{
		Diagnostic.bClassLoadSucceeded = true;
		Diagnostic.LoadedClassName = LoadedClass->GetPathName();
		Diagnostic.Message = Diagnostic.bClassAlreadyLoaded
			? TEXT("Blueprint class was already loaded before the diagnostic check.")
			: TEXT("Blueprint class load succeeded during the diagnostic check.");
		return Diagnostic;
	}

	if (!Diagnostic.bPackageExists && !Diagnostic.bAssetRegistered)
	{
		Diagnostic.Message = TEXT("Blueprint package was not found by package lookup and has no cooked asset registry entry. The path may be wrong or the asset was not cooked into the build.");
		return Diagnostic;
	}

	Diagnostic.Message = TEXT("Blueprint package metadata exists, but the generated class still failed to load. This usually points to a broken generated class, a missing dependency, or an invalid class object path.");
	return Diagnostic;
}

FDMBlueprintFunctionDiagnostic UDMPuertsLibrary::DiagnoseBlueprintFunction(const FString& ClassPath, const FString& FunctionName)
{
	FDMBlueprintFunctionDiagnostic Diagnostic;
	Diagnostic.ClassPath = ClassPath;
	Diagnostic.FunctionName = FunctionName;

	if (ClassPath.IsEmpty())
	{
		Diagnostic.Message = TEXT("Class path is empty.");
		return Diagnostic;
	}

	if (FunctionName.IsEmpty())
	{
		Diagnostic.Message = TEXT("Function name is empty.");
		return Diagnostic;
	}

	UClass* LoadedClass = StaticLoadClass(UObject::StaticClass(), nullptr, *ClassPath);
	if (!LoadedClass)
	{
		Diagnostic.Message = TEXT("Blueprint class failed to load before function lookup.");
		return Diagnostic;
	}

	Diagnostic.bClassLoaded = true;

	if (UFunction* Function = LoadedClass->FindFunctionByName(*FunctionName))
	{
		Diagnostic.bFunctionFound = true;
		Diagnostic.FunctionPathName = Function->GetPathName();
		if (const UObject* Owner = Function->GetOuter())
		{
			Diagnostic.OwnerClassName = Owner->GetName();
		}
		Diagnostic.Message = TEXT("Function lookup succeeded.");
		return Diagnostic;
	}

	Diagnostic.Message = TEXT("Function lookup failed on the loaded class.");
	return Diagnostic;
}

FDMBlueprintFunctionDiagnostic UDMPuertsLibrary::DiagnoseBlueprintFunctionByClass(UClass* InClass, const FString& FunctionName)
{
	FDMBlueprintFunctionDiagnostic Diagnostic;
	Diagnostic.FunctionName = FunctionName;

	if (!InClass)
	{
		Diagnostic.Message = TEXT("Input class is null.");
		return Diagnostic;
	}

	Diagnostic.ClassPath = InClass->GetPathName();
	Diagnostic.bClassLoaded = true;

	if (FunctionName.IsEmpty())
	{
		Diagnostic.Message = TEXT("Function name is empty.");
		return Diagnostic;
	}

	if (UFunction* Function = InClass->FindFunctionByName(*FunctionName))
	{
		Diagnostic.bFunctionFound = true;
		Diagnostic.FunctionPathName = Function->GetPathName();
		if (const UObject* Owner = Function->GetOuter())
		{
			Diagnostic.OwnerClassName = Owner->GetName();
		}
		Diagnostic.Message = TEXT("Function lookup succeeded.");
		return Diagnostic;
	}

	Diagnostic.Message = TEXT("Function lookup failed on the input class.");
	return Diagnostic;
}

void UDMPuertsLibrary::LogAllMixinClassesAndMethods()
{
	int32 MixinClassCount = 0;
	int32 MixinMethodCount = 0;

	UE_LOG(LogTemp, Log, TEXT("[DMPuertsLibrary] Begin LogAllMixinClassesAndMethods"));

	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		UClass* Class = *ClassIt;
		if (!IsValid(Class))
		{
			continue;
		}

		TArray<FString> MixinMethods;
		for (TFieldIterator<UFunction> FuncIt(Class, EFieldIteratorFlags::ExcludeSuper, EFieldIteratorFlags::ExcludeDeprecated,
				 EFieldIteratorFlags::IncludeInterfaces);
			 FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			if (!IsValid(Function))
			{
				continue;
			}

			const FString FunctionName = Function->GetName();
			if (!FunctionName.EndsWith(DMPuertsLibraryPrivate::MixinMethodSuffix))
			{
				continue;
			}

			const FString OriginalName = FunctionName.LeftChop(DMPuertsLibraryPrivate::MixinMethodSuffix.Len());
			MixinMethods.Add(FString::Printf(TEXT("%s (Mixin=%s, Path=%s)"),
				*OriginalName,
				*FunctionName,
				*Function->GetPathName()));
		}

		if (MixinMethods.Num() == 0)
		{
			continue;
		}

		++MixinClassCount;
		MixinMethodCount += MixinMethods.Num();

		UE_LOG(LogTemp, Log, TEXT("[DMPuertsLibrary] MixinClass=%s, Path=%s, MethodCount=%d"),
			*Class->GetName(),
			*Class->GetPathName(),
			MixinMethods.Num());

		for (const FString& MixinMethod : MixinMethods)
		{
			UE_LOG(LogTemp, Log, TEXT("[DMPuertsLibrary]   MixinMethod=%s"), *MixinMethod);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[DMPuertsLibrary] End LogAllMixinClassesAndMethods ClassCount=%d MethodCount=%d"),
		MixinClassCount,
		MixinMethodCount);
}
