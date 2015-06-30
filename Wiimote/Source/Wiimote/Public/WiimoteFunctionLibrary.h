// Copyright 2015 Tencent Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h" // needed for access to UBlueprintFunctionLibrary
#include "WiimoteFunctionLibrary.generated.h"

UCLASS()
class WIIMOTE_API UWiimoteFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category=Wiimote)
	static void SetIREnabled(int32 ControllerId, bool IsEnabled);
	UFUNCTION(BlueprintCallable, Category=Wiimote)
	static void SetMotionPlusEnabled(int32 ControllerId, bool IsEnabled);
	UFUNCTION(BlueprintCallable, Category=Wiimote)
	static void SetMotionSensingEnabled(int32 ControllerId, bool IsEnabled);
	UFUNCTION(BlueprintCallable, Category=Wiimote)
	static void SetRumbleEnabled(int32 ControllerId, bool IsEnabled);
};