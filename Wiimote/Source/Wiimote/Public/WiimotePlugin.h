// Copyright 2015 Tencent Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "IInputDeviceModule.h"

/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */
class FWiimotePlugin : public IInputDeviceModule
{
public:
	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual void ShutdownModule();

	FString GetModulePriorityKeyName() const
	{
		return FString(TEXT("Wiimote"));
	}

	FORCEINLINE TSharedPtr<class FWiimoteInputDevice> GetWiimoteDevice() const
	{
		return WiimoteDevice;
	}

	static inline class FWiimoteInputDevice* GetWiimoteDeviceSafe()
	{
#if WITH_EDITOR
		FWiimoteInputDevice* Device = FWiimotePlugin::IsAvailable() ? FWiimotePlugin::Get().GetWiimoteDevice().Get() : nullptr;
#else
		FWiimoteInputDevice* Device = FWiimotePlugin::Get().GetWiimoteDevice().Get();
#endif
		return Device;
	}

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline FWiimotePlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<FWiimotePlugin>("Wiimote");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Wiimote");
	}

private:
	TSharedPtr<class FWiimoteInputDevice> WiimoteDevice;
};
