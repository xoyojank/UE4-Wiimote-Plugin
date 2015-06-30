#include "WiimotePrivatePCH.h"
#include "WiimoteFunctionLibrary.h"
#include "WiimoteInputDevice.h"

UWiimoteFunctionLibrary::UWiimoteFunctionLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UWiimoteFunctionLibrary::SetIREnabled(int32 ControllerId, bool IsEnabled)
{
	FWiimoteInputDevice* WiimoteDevice = FWiimotePlugin::GetWiimoteDeviceSafe();
	if (WiimoteDevice)
	{
		WiimoteDevice->SetIREnabled(ControllerId, IsEnabled);
	}
}

void UWiimoteFunctionLibrary::SetMotionPlusEnabled(int32 ControllerId, bool IsEnabled)
{
	FWiimoteInputDevice* WiimoteDevice = FWiimotePlugin::GetWiimoteDeviceSafe();
	if (WiimoteDevice)
	{
		WiimoteDevice->SetMotionPlusEnabled(ControllerId, IsEnabled);
	}
}

void UWiimoteFunctionLibrary::SetMotionSensingEnabled(int32 ControllerId, bool IsEnabled)
{
	FWiimoteInputDevice* WiimoteDevice = FWiimotePlugin::GetWiimoteDeviceSafe();
	if (WiimoteDevice)
	{
		WiimoteDevice->SetMotionSensingEnabled(ControllerId, IsEnabled);
	}
}

void UWiimoteFunctionLibrary::SetRumbleEnabled(int32 ControllerId, bool IsEnabled)
{
	FWiimoteInputDevice* WiimoteDevice = FWiimotePlugin::GetWiimoteDeviceSafe();
	if (WiimoteDevice)
	{
		WiimoteDevice->SetRumbleEnabled(ControllerId, IsEnabled);
	}
}
