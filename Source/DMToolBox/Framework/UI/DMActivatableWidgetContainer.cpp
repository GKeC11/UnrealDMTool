#include "DMActivatableWidgetContainer.h"

#include "CommonActivatableWidget.h"

void UDMActivatableWidgetStack::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	Super::OnWidgetAddedToList(AddedWidget);

	UE_LOG(LogTemp, Log, TEXT("[DMActivatableWidgetStack] Added Widget: Name=%s, Class=%s, Path=%s, Layer=%s"),
		*AddedWidget.GetName(),
		*GetNameSafe(AddedWidget.GetClass()),
		*AddedWidget.GetPathName(),
		*LayerTag.ToString());
}

void UDMActivatableWidgetQueue::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	Super::OnWidgetAddedToList(AddedWidget);

	UE_LOG(LogTemp, Log, TEXT("[DMActivatableWidgetQueue] Added Widget: Name=%s, Class=%s, Path=%s, Layer=%s"),
		*AddedWidget.GetName(),
		*GetNameSafe(AddedWidget.GetClass()),
		*AddedWidget.GetPathName(),
		*LayerTag.ToString());
}
