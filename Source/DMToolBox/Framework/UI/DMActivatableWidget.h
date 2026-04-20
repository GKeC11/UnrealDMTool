#pragma once

#include "CommonActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"

#include "DMActivatableWidget.generated.h"

UCLASS()
class UDMActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	bool bOverrideDesiredInputConfig = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (EditCondition = "bOverrideDesiredInputConfig", EditConditionHides))
	FUIInputConfig DesiredInputConfig;
};
