#pragma once

#include "DMToolBox/Framework/UI/DMUIScreen.h"
#include "Engine/DataTable.h"

#include "DMToolBoxDeveloperSetting.generated.h"

UCLASS(Config = Game, DefaultConfig, DisplayName = "DMToolBox")
class UDMToolBoxDeveloperSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere)
	TSoftClassPtr<UDMUIScreen> DefaultUIScreenClass;

	UPROPERTY(Config, EditAnywhere)
	TSoftObjectPtr<UDataTable> WidgetConfigDataTable;
};
