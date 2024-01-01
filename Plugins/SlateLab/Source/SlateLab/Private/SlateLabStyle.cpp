#include "SlateLabStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateStyleRegistry.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSlateLabStyle::Instance = nullptr;

void FSlateLabStyle::Initialize()
{
	if(!Instance.IsValid())
	{
		Instance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*Instance);
	}
}

void FSlateLabStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*Instance);
	ensure(Instance.IsUnique());
	Instance.Reset();
}

FName FSlateLabStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SlateLabStyle"));
	return StyleSetName;
}

ISlateStyle& FSlateLabStyle::Get()
{
	return *Instance;
}

void FSlateLabStyle::ReloadTextures()
{
	if(FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const FVector2D Icon20x20(20.0f, 20.0f);

TSharedPtr<FSlateStyleSet> FSlateLabStyle::Create()
{
	TSharedPtr<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("SlateLabStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SlateLab")->GetBaseDir() / TEXT("Resources"));
	Style->Set("SlateLab.OpenSlateLab",
		new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}
