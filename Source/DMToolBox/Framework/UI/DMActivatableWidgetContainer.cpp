#include "DMActivatableWidgetContainer.h"

#include "CommonActivatableWidget.h"
#include "DMToolBox/Framework/Common/DMMacros.h"

void UDMActivatableWidgetStack::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	Super::OnWidgetAddedToList(AddedWidget);

	DM_LOG(&AddedWidget, LogTemp, Log, TEXT("Added Widget: Name=%s, Class=%s, Path=%s, Layer=%s"),
		*AddedWidget.GetName(),
		*GetNameSafe(AddedWidget.GetClass()),
		*AddedWidget.GetPathName(),
		*LayerTag.ToString());
}

void UDMActivatableWidgetQueue::OnWidgetAddedToList(UCommonActivatableWidget& AddedWidget)
{
	Super::OnWidgetAddedToList(AddedWidget);

	DM_LOG(&AddedWidget, LogTemp, Log, TEXT("Added Widget: Name=%s, Class=%s, Path=%s, Layer=%s"),
		*AddedWidget.GetName(),
		*GetNameSafe(AddedWidget.GetClass()),
		*AddedWidget.GetPathName(),
		*LayerTag.ToString());
}
