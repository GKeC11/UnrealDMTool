#pragma once

class URuntimeDebugGameInstanceSubSystem;

class FRuntimeDebugOutputDevice : public FOutputDevice
{
public:
	FRuntimeDebugOutputDevice();
	FRuntimeDebugOutputDevice(URuntimeDebugGameInstanceSubSystem* RuntimeDebugGameInstanceSubSystem);
	~FRuntimeDebugOutputDevice();

private:
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
	
	TWeakObjectPtr<URuntimeDebugGameInstanceSubSystem> RuntimeDebugGameInstanceSubSystem;
};
