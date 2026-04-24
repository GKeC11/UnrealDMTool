#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "DMPuertsLibrary.generated.h"

class UClass;

DECLARE_DYNAMIC_DELEGATE_TwoParams(FConsoleCommandDynamicDelegate, const UWorld*, InWorld, const TArray<FString>&, Args);

USTRUCT(BlueprintType)
struct FDMBlueprintClassLoadDiagnostic
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ClassPath;

	UPROPERTY(BlueprintReadOnly)
	FString PackagePath;

	UPROPERTY(BlueprintReadOnly)
	FString AssetObjectPath;

	UPROPERTY(BlueprintReadOnly)
	bool bClassAlreadyLoaded = false;

	UPROPERTY(BlueprintReadOnly)
	bool bPackageExists = false;

	UPROPERTY(BlueprintReadOnly)
	bool bAssetRegistered = false;

	UPROPERTY(BlueprintReadOnly)
	bool bClassLoadSucceeded = false;

	UPROPERTY(BlueprintReadOnly)
	FString LoadedClassName;

	UPROPERTY(BlueprintReadOnly)
	FString Message;
};

USTRUCT(BlueprintType)
struct FDMBlueprintFunctionDiagnostic
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ClassPath;

	UPROPERTY(BlueprintReadOnly)
	FString FunctionName;

	UPROPERTY(BlueprintReadOnly)
	bool bClassLoaded = false;

	UPROPERTY(BlueprintReadOnly)
	bool bFunctionFound = false;

	UPROPERTY(BlueprintReadOnly)
	FString OwnerClassName;

	UPROPERTY(BlueprintReadOnly)
	FString FunctionPathName;

	UPROPERTY(BlueprintReadOnly)
	FString Message;
};

UCLASS()
class DMTOOLBOX_API UDMPuertsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void RegisterConsoleCommand(const FString& CommandName, const FString& CommandDesc, const FConsoleCommandDynamicDelegate& CommandDelegate);

	UFUNCTION(BlueprintCallable)
	static bool KeepLoadedClassReferenced(UClass* InClass);

	UFUNCTION(BlueprintCallable)
	static FDMBlueprintClassLoadDiagnostic DiagnoseBlueprintClassLoad(const FString& ClassPath);

	UFUNCTION(BlueprintCallable)
	static FDMBlueprintFunctionDiagnostic DiagnoseBlueprintFunction(const FString& ClassPath, const FString& FunctionName);

	UFUNCTION(BlueprintCallable)
	static FDMBlueprintFunctionDiagnostic DiagnoseBlueprintFunctionByClass(UClass* InClass, const FString& FunctionName);

	UFUNCTION(BlueprintCallable)
	static void LogAllMixinClassesAndMethods();

	UFUNCTION(BlueprintPure, Category = "DMToolBox|Puerts", meta = (WorldContext = "WorldContextObject"))
	static FString GetExecutionContextLabel(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "DMToolBox|Puerts")
	static bool IsPackagedBuild();
};
