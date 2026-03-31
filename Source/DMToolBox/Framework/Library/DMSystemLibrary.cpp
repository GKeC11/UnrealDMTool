#include "DMSystemLibrary.h"

#include "DMToolBox/Framework/Gameplay/Core/DMWorldSetting.h"

UDMLevelInitializationSetting* UDMSystemLibrary::GetLevelInitializationSetting(UWorld* InWorld)
{
	AWorldSettings* WorldSetting = InWorld->GetWorldSettings();
	if (ADMWorldSetting* DMWorldSetting = Cast<ADMWorldSetting>(WorldSetting))
	{
		if (UDMLevelInitializationSetting* LevelInitializationSetting = DMWorldSetting->LevelInitializationSetting)
		{
			return LevelInitializationSetting;
		}
	}

	return nullptr;
}
