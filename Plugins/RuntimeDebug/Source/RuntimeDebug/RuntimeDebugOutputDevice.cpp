#include "RuntimeDebugOutputDevice.h"

#include "RuntimeDebugGameInstanceSubSystem.h"

FRuntimeDebugOutputDevice::FRuntimeDebugOutputDevice()
{
}

FRuntimeDebugOutputDevice::FRuntimeDebugOutputDevice(URuntimeDebugGameInstanceSubSystem* RuntimeDebugGameInstanceSubSystem)
{
	this->RuntimeDebugGameInstanceSubSystem = MakeWeakObjectPtr(RuntimeDebugGameInstanceSubSystem);
}

FRuntimeDebugOutputDevice::~FRuntimeDebugOutputDevice()
{
	if(RuntimeDebugGameInstanceSubSystem != nullptr)
	{
		RuntimeDebugGameInstanceSubSystem = nullptr;
	}
}

void FRuntimeDebugOutputDevice::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category)
{
	TSharedPtr<FRuntimeDebugConsoleEntryData> Data = MakeShareable(new FRuntimeDebugConsoleEntryData());
	Data->LogText = V;
	Data->LogType = Verbosity;

	if(RuntimeDebugGameInstanceSubSystem != nullptr)
	{
		RuntimeDebugGameInstanceSubSystem->AddLogEntryData(Data);
	}
}
