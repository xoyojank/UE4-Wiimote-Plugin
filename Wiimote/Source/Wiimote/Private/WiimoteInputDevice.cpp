// Copyright 2015 Tencent Games, Inc. All Rights Reserved.

#include "WiimotePrivatePCH.h"
#include "WiimoteInputDevice.h"


FWiimoteInputDevice::FWiimoteInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
    : MessageHandler(InMessageHandler)
{
    InitialButtonRepeatDelay = 0.2f;
    ButtonRepeatDelay = 0.1f;

    Buttons[0] = FGamepadKeyNames::FaceButtonBottom;            //WIIMOTE_BUTTON_A
    Buttons[1] = FGamepadKeyNames::FaceButtonRight;         //WIIMOTE_BUTTON_B
    Buttons[2] = FGamepadKeyNames::FaceButtonLeft;          //WIIMOTE_BUTTON_ONE
    Buttons[3] = FGamepadKeyNames::FaceButtonTop;               //WIIMOTE_BUTTON_TWO
    Buttons[4] = FGamepadKeyNames::SpecialRight;                //WIIMOTE_BUTTON_PLUS
    Buttons[5] = FGamepadKeyNames::SpecialLeft;             //WIIMOTE_BUTTON_MINUS
    Buttons[6] = FGamepadKeyNames::DPadUp;                  //WIIMOTE_BUTTON_UP
    Buttons[7] = FGamepadKeyNames::DPadDown;                    //WIIMOTE_BUTTON_DOWN
    Buttons[8] = FGamepadKeyNames::DPadLeft;                    //WIIMOTE_BUTTON_LEFT
    Buttons[9] = FGamepadKeyNames::DPadRight;                   //WIIMOTE_BUTTON_RIGHT
    Buttons[10] = FGamepadKeyNames::LeftShoulder;               //NUNCHUK_BUTTON_C
    Buttons[11] = FGamepadKeyNames::LeftTriggerThreshold;       //NUNCHUK_BUTTON_Z
    Buttons[12] = FGamepadKeyNames::LeftStickUp;                //NUNCHUK_JOYSTICK
    Buttons[13] = FGamepadKeyNames::LeftStickDown;          //NUNCHUK_JOYSTICK
    Buttons[14] = FGamepadKeyNames::LeftStickLeft;          //NUNCHUK_JOYSTICK
    Buttons[15] = FGamepadKeyNames::LeftStickRight;         //NUNCHUK_JOYSTICK

    int found, connected;
    /*
    *   Initialize an array of wiimote objects.
    *
    *   The parameter is the number of wiimotes I want to create.
    */
    GWiimotes = wiiuse_init(MAX_WIIMOTES);

    /*
    *   Find wiimote devices
    *
    *   Now we need to find some wiimotes.
    *   Give the function the wiimote array we created, and tell it there
    *   are MAX_WIIMOTES wiimotes we are interested in.
    *
    *   Set the timeout to be 5 seconds.
    *
    *   This will return the number of actual wiimotes that are in discovery mode.
    */
    found = wiiuse_find(GWiimotes, MAX_WIIMOTES, 5);
    if (!found)
    {
        UE_LOG(LogWiimote, Log, TEXT("No wiimotes found."));
        return;
    }

    /*
    *   Connect to the wiimotes
    *
    *   Now that we found some wiimotes, connect to them.
    *   Give the function the wiimote array and the number
    *   of wiimote devices we found.
    *
    *   This will return the number of established connections to the found wiimotes.
    */
    connected = wiiuse_connect(GWiimotes, MAX_WIIMOTES);
    if (connected)
    {
        UE_LOG(LogWiimote, Log, TEXT("Connected to %i wiimotes (of %i found)."), connected, found);
    }
    else
    {
        UE_LOG(LogWiimote, Log, TEXT("Failed to connect to any wiimote."));
        return;
    }

    /*
    *   Now set the LEDs and rumble for a second so it's easy
    *   to tell which wiimotes are connected (just like the wii does).
    */
    wiiuse_set_leds(GWiimotes[0], WIIMOTE_LED_1);
    wiiuse_set_leds(GWiimotes[1], WIIMOTE_LED_2);
    wiiuse_set_leds(GWiimotes[2], WIIMOTE_LED_3);
    wiiuse_set_leds(GWiimotes[3], WIIMOTE_LED_4);

    UE_LOG(LogWiimote, Log, TEXT("\nControls:"));
    UE_LOG(LogWiimote, Log, TEXT("\tB toggles rumble."));
    UE_LOG(LogWiimote, Log, TEXT("\t+ to start Wiimote accelerometer reporting, - to stop"));
    UE_LOG(LogWiimote, Log, TEXT("\tUP to start IR camera (sensor bar mode), DOWN to stop."));
    UE_LOG(LogWiimote, Log, TEXT("\t1 to start Motion+ reporting, 2 to stop."));
    UE_LOG(LogWiimote, Log, TEXT("\n"));

    FMemory::Memzero(ControllerStates);
    for (int i = 0; i < MAX_WIIMOTES; ++i)
    {
        ControllerStates[i].ControllerId = i;
        ControllerStates[i].Wiimote = GWiimotes[i];
    }
}

FWiimoteInputDevice::~FWiimoteInputDevice()
{
    /*
    *   Disconnect the wiimotes
    */
    wiiuse_cleanup(GWiimotes, MAX_WIIMOTES);
}

void FWiimoteInputDevice::Tick(float DeltaTime)
{

}

void FWiimoteInputDevice::SendControllerEvents()
{
    /*
    *   Maybe I'm interested in the battery power of the 0th
    *   wiimote.  This should be WIIMOTE_ID_1 but to be sure
    *   you can get the wiimote associated with WIIMOTE_ID_1
    *   using the wiiuse_get_by_id() function.
    *
    *   A status request will return other things too, like
    *   if any expansions are plugged into the wiimote or
    *   what LEDs are lit.
    */
    /* wiiuse_status(wiimotes[0]); */

    /*
    *   This is the main loop
    *
    *   wiiuse_poll() needs to be called with the wiimote array
    *   and the number of wiimote structures in that array
    *   (it doesn't matter if some of those wiimotes are not used
    *   or are not connected).
    *
    *   This function will set the event flag for each wiimote
    *   when the wiimote has things to report.
    */
    if (AnyWiimoteConnected())
    {
        if (wiiuse_poll(GWiimotes, MAX_WIIMOTES))
        {
            /*
            *   This happens if something happened on any wiimote.
            *   So go through each one and check if anything happened.
            */
            const double CurrentTime = FPlatformTime::Seconds();
            for (int i = 0; i < MAX_WIIMOTES; ++i)
            {
                FControllerState& ControllerState = ControllerStates[i];
                ControllerState.Wiimote = GWiimotes[i];
                if (ControllerState.Wiimote && WIIMOTE_IS_CONNECTED(ControllerState.Wiimote))
                {
                    // Check Analog state
                    if (ControllerState.Wiimote->exp.type == EXP_NUNCHUK ||
                        ControllerState.Wiimote->exp.type == EXP_MOTION_PLUS_NUNCHUK)
                    {
                        /* nunchuk */
                        struct nunchuk_t* nc = (nunchuk_t*)&ControllerState.Wiimote->exp.nunchuk;

                        if (ControllerState.LeftXAnalog != nc->js.x)
                        {
                            MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftAnalogX, ControllerState.ControllerId, nc->js.x);
                            ControllerState.LeftXAnalog = nc->js.x;
                        }

                        if (ControllerState.LeftYAnalog != nc->js.y)
                        {
                            MessageHandler->OnControllerAnalog(FGamepadKeyNames::LeftAnalogY, ControllerState.ControllerId, nc->js.y);
                            ControllerState.LeftYAnalog = nc->js.y;
                        }
                    }
                }

                FMemory::Memzero(CurrentStates);

                switch (ControllerState.Wiimote->event)
                {
                case WIIUSE_EVENT:
                    /* a generic event occurred */
                    handle_event(ControllerState.Wiimote, i);
                    break;

                case WIIUSE_STATUS:
                    /* a status event occurred */
                    handle_ctrl_status(ControllerState.Wiimote, i);
                    break;

                case WIIUSE_DISCONNECT:
                case WIIUSE_UNEXPECTED_DISCONNECT:
                    /* the wiimote disconnected */
                    handle_disconnect(ControllerState.Wiimote, i);
                    break;

                case WIIUSE_READ_DATA:
                    /*
                    *   Data we requested to read was returned.
                    *   Take a look at ControllerState.Wiimote>read_req
                    *   for the data.
                    */
                    break;

                case WIIUSE_NUNCHUK_INSERTED:
                    /*
                    *   a nunchuk was inserted
                    *   This is a good place to set any nunchuk specific
                    *   threshold values.  By default they are the same
                    *   as the wiimote.
                    */
                    /* wiiuse_set_nunchuk_orient_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 90.0f); */
                    /* wiiuse_set_nunchuk_accel_threshold((struct nunchuk_t*)&wiimotes[i]->exp.nunchuk, 100); */
                    UE_LOG(LogWiimote, Log, TEXT("Nunchuk inserted."));
                    break;

                case WIIUSE_MOTION_PLUS_ACTIVATED:
                    UE_LOG(LogWiimote, Log, TEXT("Motion+ was activated"));
                    break;

                case WIIUSE_NUNCHUK_REMOVED:
                case WIIUSE_MOTION_PLUS_REMOVED:
                    /* some expansion was removed */
                    handle_ctrl_status(ControllerState.Wiimote, i);
                    UE_LOG(LogWiimote, Log, TEXT("An expansion was removed."));
                    break;

                default:
                    break;
                }

                // For each button check against the previous state and send the correct message if any
                for (int32 ButtonIndex = 0; ButtonIndex < MAX_NUM_WIIMOTE_BUTTONS; ++ButtonIndex)
                {
                    if (CurrentStates[ButtonIndex] != ControllerState.ButtonStates[ButtonIndex])
                    {
                        if (CurrentStates[ButtonIndex])
                        {
                            MessageHandler->OnControllerButtonPressed(Buttons[ButtonIndex], ControllerState.ControllerId, false);
                        }
                        else
                        {
                            MessageHandler->OnControllerButtonReleased(Buttons[ButtonIndex], ControllerState.ControllerId, false);
                        }

                        if (CurrentStates[ButtonIndex] != 0)
                        {
                            // this button was pressed - set the button's NextRepeatTime to the InitialButtonRepeatDelay
                            ControllerState.NextRepeatTime[ButtonIndex] = CurrentTime + InitialButtonRepeatDelay;
                        }
                    }
                    else if (CurrentStates[ButtonIndex] != 0 && ControllerState.NextRepeatTime[ButtonIndex] <= CurrentTime)
                    {
                        MessageHandler->OnControllerButtonPressed(Buttons[ButtonIndex], ControllerState.ControllerId, true);

                        // set the button's NextRepeatTime to the ButtonRepeatDelay
                        ControllerState.NextRepeatTime[ButtonIndex] = CurrentTime + ButtonRepeatDelay;
                    }

                    // Update the state for next time
                    ControllerState.ButtonStates[ButtonIndex] = CurrentStates[ButtonIndex];
                }

            }
        }
    }
}

void FWiimoteInputDevice::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
    MessageHandler = InMessageHandler;
}

bool FWiimoteInputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
    return true;
}

void FWiimoteInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{

}

void FWiimoteInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{

}

void FWiimoteInputDevice::handle_event(struct wiimote_t* wm, int id)
{
    UE_LOG(LogWiimote, Log, TEXT("\n\n--- EVENT [id %i] ---"), wm->unid);

    CurrentStates[0] = IS_PRESSED(wm, WIIMOTE_BUTTON_A) || IS_HELD(wm, WIIMOTE_BUTTON_A);
    CurrentStates[1] = IS_PRESSED(wm, WIIMOTE_BUTTON_B) || IS_HELD(wm, WIIMOTE_BUTTON_B);
    CurrentStates[2] = IS_PRESSED(wm, WIIMOTE_BUTTON_ONE) || IS_HELD(wm, WIIMOTE_BUTTON_ONE);
    CurrentStates[3] = IS_PRESSED(wm, WIIMOTE_BUTTON_TWO) || IS_HELD(wm, WIIMOTE_BUTTON_TWO);
    CurrentStates[4] = IS_PRESSED(wm, WIIMOTE_BUTTON_PLUS) || IS_HELD(wm, WIIMOTE_BUTTON_PLUS);
    CurrentStates[5] = IS_PRESSED(wm, WIIMOTE_BUTTON_MINUS) || IS_HELD(wm, WIIMOTE_BUTTON_MINUS);
    CurrentStates[6] = IS_PRESSED(wm, WIIMOTE_BUTTON_UP) || IS_HELD(wm, WIIMOTE_BUTTON_UP);
    CurrentStates[7] = IS_PRESSED(wm, WIIMOTE_BUTTON_DOWN) || IS_HELD(wm, WIIMOTE_BUTTON_DOWN);
    CurrentStates[8] = IS_PRESSED(wm, WIIMOTE_BUTTON_LEFT) || IS_HELD(wm, WIIMOTE_BUTTON_LEFT);
    CurrentStates[9] = IS_PRESSED(wm, WIIMOTE_BUTTON_RIGHT) || IS_HELD(wm, WIIMOTE_BUTTON_RIGHT);

    if (IS_PRESSED(wm, WIIMOTE_BUTTON_HOME))
    {
        UE_LOG(LogWiimote, Log, TEXT("HOME pressed"));
    }

    /* show events specific to supported expansions */
    if (wm->exp.type == EXP_NUNCHUK || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK)
    {
        /* nunchuk */
        struct nunchuk_t* nc = (nunchuk_t*)&wm->exp.nunchuk;

        const float DEADZONE = 0.6f;

        CurrentStates[10] = IS_PRESSED(nc, NUNCHUK_BUTTON_C) || IS_HELD(wm, NUNCHUK_BUTTON_C);
        CurrentStates[11] = IS_PRESSED(nc, NUNCHUK_BUTTON_Z) || IS_HELD(wm, NUNCHUK_BUTTON_Z);
        CurrentStates[12] = nc->js.y > DEADZONE;
        CurrentStates[13] = nc->js.y < -DEADZONE;
        CurrentStates[14] = nc->js.x < -DEADZONE;
        CurrentStates[15] = nc->js.x > DEADZONE;

        UE_LOG(LogWiimote, Log, TEXT("nunchuk roll  = %f"), nc->orient.roll);
        UE_LOG(LogWiimote, Log, TEXT("nunchuk pitch = %f"), nc->orient.pitch);
        UE_LOG(LogWiimote, Log, TEXT("nunchuk yaw   = %f"), nc->orient.yaw);

        UE_LOG(LogWiimote, Log, TEXT("nunchuk joystick angle:     %f"), nc->js.ang);
        UE_LOG(LogWiimote, Log, TEXT("nunchuk joystick magnitude: %f"), nc->js.mag);

        UE_LOG(LogWiimote, Log, TEXT("nunchuk joystick vals:      %f, %f"), nc->js.x, nc->js.y);
        UE_LOG(LogWiimote, Log, TEXT("nunchuk joystick calibration (min, center, max): x: %i, %i, %i  y: %i, %i, %i"),
               nc->js.min.x,
               nc->js.center.x,
               nc->js.max.x,
               nc->js.min.y,
               nc->js.center.y,
               nc->js.max.y);
    }

    /*
    *   If IR tracking is enabled then print the coordinates
    *   on the virtual screen that the wiimote is pointing to.
    *
    *   Also make sure that we see at least 1 dot.
    */
    if (WIIUSE_USING_IR(wm))
    {
        /* go through each of the 4 possible IR sources */
        for (int i = 0; i < 4; ++i)
        {
            /* check if the source is visible */
            if (wm->ir.dot[i].visible)
            {
                UE_LOG(LogWiimote, Log, TEXT("IR source %i: (%u, %u)"), i, wm->ir.dot[i].x, wm->ir.dot[i].y);
            }
        }

        UE_LOG(LogWiimote, Log, TEXT("IR cursor: (%u, %u)"), wm->ir.x, wm->ir.y);
        UE_LOG(LogWiimote, Log, TEXT("IR z distance: %f"), wm->ir.z);
    }
    // Update motion controls.
    FVector Tilt(0, 0, 0);
    FVector RotationRate(0, 0, 0);
    FVector Gravity(0, 0, 0);
    FVector Acceleration(0, 0, 0);

    /* if the accelerometer is turned on then print angles */
    if (WIIUSE_USING_ACC(wm))
    {
        Tilt.X = -wm->orient.pitch;
        Tilt.Y = wm->orient.yaw;
        Tilt.Z = wm->orient.roll;

        Acceleration.X = wm->accel.x;
        Acceleration.Y = wm->accel.y;
        Acceleration.Z = wm->accel.z;
    }

    if (wm->exp.type == EXP_MOTION_PLUS || wm->exp.type == EXP_MOTION_PLUS_NUNCHUK)
    {
        RotationRate.X = -wm->exp.mp.angle_rate_gyro.pitch;
        RotationRate.Y = wm->exp.mp.angle_rate_gyro.yaw;
        RotationRate.Z = wm->exp.mp.angle_rate_gyro.roll;
    }

    Gravity.X = wm->gforce.x;
    Gravity.Y = wm->gforce.y;
    Gravity.Z = wm->gforce.z;

    MessageHandler->OnMotionDetected(Tilt, RotationRate, Gravity, Acceleration, id);
}

void FWiimoteInputDevice::handle_ctrl_status(struct wiimote_t* wm, int id)
{
    UE_LOG(LogWiimote, Log, TEXT("\n\n--- CONTROLLER STATUS [wiimote id %i] ---"), wm->unid);

    UE_LOG(LogWiimote, Log, TEXT("attachment:      %i"), wm->exp.type);
    UE_LOG(LogWiimote, Log, TEXT("speaker:         %i"), WIIUSE_USING_SPEAKER(wm));
    UE_LOG(LogWiimote, Log, TEXT("ir:              %i"), WIIUSE_USING_IR(wm));
    UE_LOG(LogWiimote, Log, TEXT("leds:            %i %i %i %i"), WIIUSE_IS_LED_SET(wm, 1), WIIUSE_IS_LED_SET(wm, 2), WIIUSE_IS_LED_SET(wm, 3), WIIUSE_IS_LED_SET(wm, 4));
    UE_LOG(LogWiimote, Log, TEXT("battery:         %f %%"), wm->battery_level);
}

void FWiimoteInputDevice::handle_disconnect((struct wiimote_t* wm, int id)
{
    UE_LOG(LogWiimote, Log, TEXT("\n\n--- DISCONNECTED [wiimote id %i] ---"), wm->unid);
}

bool FWiimoteInputDevice::AnyWiimoteConnected() const
{
    if (!GWiimotes)
    {
        return false;
    }

    for (int i = 0; i < MAX_WIIMOTES; i++)
    {
        if (GWiimotes[i] && WIIMOTE_IS_CONNECTED(GWiimotes[i]))
        {
            return true;
        }
    }

    return false;
}

void FWiimoteInputDevice::SetIREnabled(int32 ControllerId, bool IsEnabled)
{
    if (GWiimotes[ControllerId] == nullptr)
        return;
    if (IsEnabled)
    {
        wiiuse_set_ir(GWiimotes[ControllerId], 1);
    }
    else
    {
        wiiuse_set_ir(GWiimotes[ControllerId], 0);
    }
}

void FWiimoteInputDevice::SetMotionPlusEnabled(int32 ControllerId, bool IsEnabled)
{
    if (GWiimotes[ControllerId] == nullptr)
        return;
    if (IsEnabled)
    {
        if (WIIUSE_USING_EXP(GWiimotes[ControllerId]))
        {
            wiiuse_set_motion_plus(GWiimotes[ControllerId], 2);    // nunchuck pass-through
        }
        else
        {
            wiiuse_set_motion_plus(GWiimotes[ControllerId], 1);    // standalone
        }
    }
    else
    {
        wiiuse_set_motion_plus(GWiimotes[ControllerId], 0); // off
    }
}

void FWiimoteInputDevice::SetMotionSensingEnabled(int32 ControllerId, bool IsEnabled)
{
    if (GWiimotes[ControllerId] == nullptr)
        return;
    if (IsEnabled)
    {
        wiiuse_motion_sensing(GWiimotes[ControllerId], 1);
    }
    else
    {
        wiiuse_motion_sensing(GWiimotes[ControllerId], 0);
    }
}

void FWiimoteInputDevice::SetRumbleEnabled(int32 ControllerId, bool IsEnabled)
{
    if (GWiimotes[ControllerId] == nullptr)
        return;
    if (IsEnabled)
    {
        wiiuse_rumble(GWiimotes[ControllerId], 1);
    }
    else
    {
        wiiuse_rumble(GWiimotes[ControllerId], 0);
    }
}
