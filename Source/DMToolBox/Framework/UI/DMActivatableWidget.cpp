#include "DMActivatableWidget.h"

TOptional<FUIInputConfig> UDMActivatableWidget::GetDesiredInputConfig() const
{
	if (bOverrideDesiredInputConfig)
	{
		return DesiredInputConfig;
	}

	return Super::GetDesiredInputConfig();
}
