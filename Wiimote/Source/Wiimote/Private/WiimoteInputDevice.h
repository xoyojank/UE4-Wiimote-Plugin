// Copyright 2015 Tencent Games, Inc. All Rights Reserved.

#pragma once
#include "WiimotePlugin.h"
#include "InputDevice.h"


#define MAX_WIIMOTES            4
#define MAX_NUM_WIIMOTE_BUTTONS 16

/**
 * SteamVR Head Mounted Display
 */
class FWiimoteInputDevice : public IInputDevice, public TSharedFromThis<FWiimoteInputDevice>
{
    friend class FWiimotePlugin;
    FWiimoteInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler);
public:
    ~FWiimoteInputDevice();

    /** Tick the interface (e.g. check for new controllers) */
    virtual void Tick(float DeltaTime) override;

    /** Poll for controller state and send events if needed */
    virtual void SendControllerEvents() override;

    /** Set which MessageHandler will get the events from SendControllerEvents. */
    virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;

    /** Exec handler to allow console commands to be passed through for debugging */
    virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;

    /**
     * IForceFeedbackSystem pass through functions
     */
    virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
    virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override;


    bool AnyWiimoteConnected() const;

    void SetIREnabled(int32 ControllerId, bool IsEnabled);
    void SetMotionPlusEnabled(int32 ControllerId, bool IsEnabled);
    void SetMotionSensingEnabled(int32 ControllerId, bool IsEnabled);
    void SetRumbleEnabled(int32 ControllerId, bool IsEnabled);

private:


    /**
    *   @brief Callback that handles an event.
    *
    *   @param wm       Pointer to a wiimote_t structure.
    *
    *   This function is called automatically by the wiiuse library when an
    *   event occurs on the specified wiimote.
    */
    void handle_event(struct wiimote_t* wm, int id);

    /**
    *   @brief Callback that handles a controller status event.
    *
    *   @param wm               Pointer to a wiimote_t structure.
    *   @param attachment       Is there an attachment? (1 for yes, 0 for no)
    *   @param speaker          Is the speaker enabled? (1 for yes, 0 for no)
    *   @param ir               Is the IR support enabled? (1 for yes, 0 for no)
    *   @param led              What LEDs are lit.
    *   @param battery_level    Battery level, between 0.0 (0%) and 1.0 (100%).
    *
    *   This occurs when either the controller status changed
    *   or the controller status was requested explicitly by
    *   wiiuse_status().
    *
    *   One reason the status can change is if the nunchuk was
    *   inserted or removed from the expansion port.
    */
    void handle_ctrl_status(struct wiimote_t* wm, int id);


    /**
    *   @brief Callback that handles a disconnection event.
    *
    *   @param wm               Pointer to a wiimote_t structure.
    *
    *   This can happen if the POWER button is pressed, or
    *   if the connection is interrupted.
    */
    void handle_disconnect(wiimote* wm, int id);

private:

    struct FControllerState
    {
        /** Last frame's button states, so we only send events on edges */
        bool ButtonStates[MAX_NUM_WIIMOTE_BUTTONS];

        /** Next time a repeat event should be generated for each button */
        double NextRepeatTime[MAX_NUM_WIIMOTE_BUTTONS];
        /** Raw Left thumb x analog value */
        short LeftXAnalog;

        /** Raw left thumb y analog value */
        short LeftYAnalog;

        /** Id of the controller */
        int32 ControllerId;

        struct wiimote_t* Wiimote;

        /* Accelerometer */
        /** IR */
        /** MotionPlus */
        /** NunChuk */
    };

    /** Names of all the buttons */
    FGamepadKeyNames::Type Buttons[MAX_NUM_WIIMOTE_BUTTONS];
    bool CurrentStates[MAX_NUM_WIIMOTE_BUTTONS];


    /** Controller states */
    FControllerState ControllerStates[MAX_WIIMOTES];
    struct wiimote_t** GWiimotes;

    /** Delay before sending a repeat message after a button was first pressed */
    float InitialButtonRepeatDelay;

    /** Delay before sendign a repeat message after a button has been pressed for a while */
    float ButtonRepeatDelay;

    TSharedRef<FGenericApplicationMessageHandler> MessageHandler; ///< The message handler
};


DEFINE_LOG_CATEGORY_STATIC(LogWiimote, Log, All);
