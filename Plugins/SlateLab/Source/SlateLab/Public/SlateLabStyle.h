#pragma once

class FSlateLabStyle
{
public:
	
	static void Initialize();

	static void Shutdown();

	static FName GetStyleSetName();

	static ISlateStyle& Get();

	static void ReloadTextures();

private:

	static TSharedPtr<FSlateStyleSet> Create();
	

private:

	static TSharedPtr<FSlateStyleSet> Instance;
	
};
