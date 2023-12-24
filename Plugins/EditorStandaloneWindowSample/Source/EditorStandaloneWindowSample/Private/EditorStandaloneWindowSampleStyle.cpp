// Copyright Epic Games, Inc. All Rights Reserved.

#include "EditorStandaloneWindowSampleStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FEditorStandaloneWindowSampleStyle::StyleInstance = nullptr;

void FEditorStandaloneWindowSampleStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FEditorStandaloneWindowSampleStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FEditorStandaloneWindowSampleStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("EditorStandaloneWindowSampleStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FEditorStandaloneWindowSampleStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("EditorStandaloneWindowSampleStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("EditorStandaloneWindowSample")->GetBaseDir() / TEXT("Resources"));

	Style->Set("EditorStandaloneWindowSample.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FEditorStandaloneWindowSampleStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FEditorStandaloneWindowSampleStyle::Get()
{
	return *StyleInstance;
}
