// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "RuntimeDebug/RuntimeDebugGameInstanceSubSystem.h"
#include "../../Source/Runtime/Engine/Classes/Engine/GameInstance.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeRuntimeDebugGameInstanceSubSystem() {}
// Cross Module References
	ENGINE_API UClass* Z_Construct_UClass_UGameInstanceSubsystem();
	RUNTIMEDEBUG_API UClass* Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem();
	RUNTIMEDEBUG_API UClass* Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_NoRegister();
	UPackage* Z_Construct_UPackage__Script_RuntimeDebug();
// End Cross Module References
	void URuntimeDebugGameInstanceSubSystem::StaticRegisterNativesURuntimeDebugGameInstanceSubSystem()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(URuntimeDebugGameInstanceSubSystem);
	UClass* Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_NoRegister()
	{
		return URuntimeDebugGameInstanceSubSystem::StaticClass();
	}
	struct Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_UGameInstanceSubsystem,
		(UObject* (*)())Z_Construct_UPackage__Script_RuntimeDebug,
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::DependentSingletons) < 16);
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n * \n */" },
#endif
		{ "IncludePath", "RuntimeDebugGameInstanceSubSystem.h" },
		{ "ModuleRelativePath", "RuntimeDebugGameInstanceSubSystem.h" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<URuntimeDebugGameInstanceSubSystem>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::ClassParams = {
		&URuntimeDebugGameInstanceSubSystem::StaticClass,
		nullptr,
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x001000A0u,
		METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::Class_MetaDataParams), Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::Class_MetaDataParams)
	};
	UClass* Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem()
	{
		if (!Z_Registration_Info_UClass_URuntimeDebugGameInstanceSubSystem.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_URuntimeDebugGameInstanceSubSystem.OuterSingleton, Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_URuntimeDebugGameInstanceSubSystem.OuterSingleton;
	}
	template<> RUNTIMEDEBUG_API UClass* StaticClass<URuntimeDebugGameInstanceSubSystem>()
	{
		return URuntimeDebugGameInstanceSubSystem::StaticClass();
	}
	URuntimeDebugGameInstanceSubSystem::URuntimeDebugGameInstanceSubSystem() {}
	DEFINE_VTABLE_PTR_HELPER_CTOR(URuntimeDebugGameInstanceSubSystem);
	URuntimeDebugGameInstanceSubSystem::~URuntimeDebugGameInstanceSubSystem() {}
	struct Z_CompiledInDeferFile_FID_UnrealProjects_DMTool_Plugins_RuntimeDebug_Source_RuntimeDebug_RuntimeDebugGameInstanceSubSystem_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UnrealProjects_DMTool_Plugins_RuntimeDebug_Source_RuntimeDebug_RuntimeDebugGameInstanceSubSystem_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_URuntimeDebugGameInstanceSubSystem, URuntimeDebugGameInstanceSubSystem::StaticClass, TEXT("URuntimeDebugGameInstanceSubSystem"), &Z_Registration_Info_UClass_URuntimeDebugGameInstanceSubSystem, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(URuntimeDebugGameInstanceSubSystem), 2920912969U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UnrealProjects_DMTool_Plugins_RuntimeDebug_Source_RuntimeDebug_RuntimeDebugGameInstanceSubSystem_h_2909728361(TEXT("/Script/RuntimeDebug"),
		Z_CompiledInDeferFile_FID_UnrealProjects_DMTool_Plugins_RuntimeDebug_Source_RuntimeDebug_RuntimeDebugGameInstanceSubSystem_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UnrealProjects_DMTool_Plugins_RuntimeDebug_Source_RuntimeDebug_RuntimeDebugGameInstanceSubSystem_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
