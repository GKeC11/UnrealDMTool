#include "DMUILibrary.h"
#include "DMToolBox/Framework/UI/DMUIScreen.h"

void UDMUILibrary::CreateWidgetToLayer(APlayerController* InPlayerController, TSubclassOf<UCommonActivatableWidget> InWidgetClass, FGameplayTag InLayerTag)
{
	UDMUIScreen* Screen = UDMUIScreen::GetUIScreen(InPlayerController);
	UDMUILayout* Layout = Screen->GetLayoutFromLocalPlayer(InPlayerController->GetLocalPlayer());
	UCommonActivatableWidgetContainerBase* Layer = Layout->GetLayerFromGameplayTag(InLayerTag);
	Layer->AddWidget(InWidgetClass);
}
