/********************************** (C) COPYRIGHT *******************************
 * File Name          : peripheral.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2018/12/10
 * Description        : ����ӻ�������Ӧ�ó��򣬳�ʼ���㲥���Ӳ�����Ȼ��㲥������������
 *                      ����������Ӳ�����ͨ���Զ������������
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "devinfoservice.h"
#include "gattprofile.h"
#include "motor_config.h"
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform read rssi event
#define SBP_READ_RSSI_EVT_PERIOD             3200

// Parameter update delay
#define SBP_PARAM_UPDATE_DELAY               6400

// What is the advertising interval when device is discoverable (units of 625us, 80=50ms)
#define DEFAULT_ADVERTISING_INTERVAL         80

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE            GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 6=7.5ms)
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL    6

// Maximum connection interval (units of 1.25ms, 100=125ms)
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL    100

// Slave latency to use parameter update
#define DEFAULT_DESIRED_SLAVE_LATENCY        0

// Supervision timeout value (units of 10ms, 100=1s)
#define DEFAULT_DESIRED_CONN_TIMEOUT         100

// Company Identifier: WCH
#define WCH_COMPANY_ID                       0x07D7

// Motor outputs and Hall inputs (CH571R)
#define MOTOR1_IN1_PIN                       GPIO_Pin_12  // PA12
#define MOTOR1_IN2_PIN                       GPIO_Pin_13  // PA13
#define MOTOR2_IN1_PIN                       GPIO_Pin_14  // PA14
#define MOTOR2_IN2_PIN                       GPIO_Pin_15  // PA15
#define MOTOR1_HALL_PIN                      GPIO_Pin_22  // PB22
#define MOTOR2_HALL_PIN                      GPIO_Pin_23  // PB23
#define LED1_PIN                             GPIO_Pin_15  // PB15, active low
#define LED2_PIN                             GPIO_Pin_10  // PB10, active high

// Software PWM: 16 * 625us = 10ms, about 100Hz.
#define SBP_MOTOR_EVT_PERIOD                 1
#define MOTOR_PWM_STEPS                      16

// Endpoint detection. Adjust these values for the actual motor/Hall speed.
#define MOTOR_MIN_RUN_TICKS                  240     // 150ms before endpoint detection is allowed
#define HALL_STABLE_TICKS                    1600    // 1s without a Hall edge means arrived
#define HALL_DEBUG_PRINT_TICKS               800     // Print raw Hall state every 500ms
#define MOTOR_MAX_RUN_TICKS                  48000   // 30s safety timeout

#define MOTOR_DIR_STOP                       0
#define MOTOR_DIR_FORWARD                    1
#define MOTOR_DIR_REVERSE                    2

#define CONTROL_MODE_MOTOR                   0
#define CONTROL_MODE_LIGHT_SWITCH            1
#define CONTROL_MODE_LIGHT_BREATH            2
#define CONTROL_MODE_LIGHT_BRIGHTNESS        3

#define MOTOR_ACTION_USER                     0
#define MOTOR_ACTION_HOME_MOTOR1              1
#define MOTOR_ACTION_HOME_MOTOR2              2
#define MOTOR_ACTION_CLEAR_FOR_PENDING        3
#define MOTOR_ACTION_GOTO_MOTOR1_WORK_MIN     4

#define MOTOR1_WORK_MIN_HALL_COUNT \
    ((MOTOR1_TOTAL_HALL_COUNT * MOTOR1_WORK_MIN_PERCENT) / 100UL)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t Peripheral_TaskID = INVALID_TASK_ID; // Task ID for internal task/event processing

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8_t scanRspData[] = {
    // complete name
    0x0A, // length of this data
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'B',
    'L',
    'E',
    ' ',
    'M',
    'o',
    't',
    'o',
    'r',
    // connection interval range
    0x05, // length of this data
    GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
    LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL), // 100ms
    HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
    LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL), // 1s
    HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

    // Tx power level
    0x02, // length of this data
    GAP_ADTYPE_POWER_LEVEL,
    0 // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertising)
static uint8_t advertData[] = {
    // Flags; this sets the device to use limited discoverable
    // mode (advertises for 30 seconds at a time) instead of general
    // discoverable mode (advertises indefinitely)
    0x02, // length of this data
    GAP_ADTYPE_FLAGS,
    DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

    // service UUID, to notify central devices what services are included
    // in this peripheral
    0x03,                  // length of this data
    GAP_ADTYPE_16BIT_MORE, // some of the UUID's, but not all
    LO_UINT16(SIMPLEPROFILE_SERV_UUID),
    HI_UINT16(SIMPLEPROFILE_SERV_UUID)
};

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "BLE Motor";

// Connection item list
static peripheralConnItem_t peripheralConnList;

static uint8_t peripheralMTU = ATT_MTU_SIZE;

static uint8_t motorPwmPhase = 0;
static uint8_t activeMotor = 0;
static uint8_t activeDirection = MOTOR_DIR_STOP;
static uint8_t activeDuty = 0;
static uint8_t lastHallLevel = 0;
static uint16_t hallStableCount = 0;
static uint16_t hallDebugPrintCount = 0;
static uint32_t motorRunCount = 0;
static uint32_t motorRotationCount[2] = {0, 0};
static uint32_t hallTriggerCount[2] = {0, 0};
static uint8_t motorReverseReady[2] = {0, 0};
static uint8_t pendingMotor = 0;
static uint8_t pendingDirection = MOTOR_DIR_STOP;
static uint8_t pendingSpeed = 0;
// Motor selected by the latest jog command. A stale release from another
// button must not stop the command that replaced it.
static uint8_t jogCommandMotor = 0;
static uint8_t motorAction = MOTOR_ACTION_USER;
static uint8_t homingComplete = 0;
static uint32_t motorRealtimePosition[2] = {0, 0};
static uint8_t controlMode = CONTROL_MODE_MOTOR;
static uint8_t lightValues[4] = {0, 0, 0, 0};
static uint8_t lightPwmPhase = 0;
static uint8_t lightBreathLevel = 0;
static uint8_t lightBreathRising = 1;
static uint8_t lightBreathHold = 0;

// Sine-based breathing lookup table (0-15 PWM duty, 100 steps)
// Rise: 0→15 over 50 steps, Fall: 15→0 over 50 steps
/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg);
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent);
static void motorTaskHandler(void);
static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len);
static void peripheralMotorGpioInit(void);
static void motorStop(void);
static void motorStart(uint8_t motor, uint8_t direction, uint8_t speed);
static void motorHandleCommand(uint8_t motor, uint8_t direction, uint8_t speed);
static uint32_t motorTotalHallCount(uint8_t motor);
static uint8_t motorPositionPercent(uint8_t motor);
static void motorClearPending(void);
static void motorStartHoming(void);
static void lightAllOff(void);
static void lightSetMode(uint8_t mode);
static void lightSetChannel(uint8_t channel, uint8_t value);
static void lightTaskHandler(void);
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                    uint16_t connSlaveLatency, uint16_t connTimeout);
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList);
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi);
/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t Peripheral_PeripheralCBs = {
    peripheralStateNotificationCB, // Profile State Change Callbacks
    peripheralRssiCB,              // When a valid RSSI is read from controller (not used by application)
    peripheralParamUpdateCB
};

// Broadcast Callbacks
static gapRolesBroadcasterCBs_t Broadcaster_BroadcasterCBs = {
    NULL, // Not used in peripheral role
    NULL  // Receive scan request callback
};

// GAP Bond Manager Callbacks
static gapBondCBs_t Peripheral_BondMgrCBs = {
    NULL, // Passcode callback (not used by application)
    NULL  // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t Peripheral_SimpleProfileCBs = {
    simpleProfileChangeCB // Characteristic value change callback
};
/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      Peripheral_Init
 *
 * @brief   Initialization function for the Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void Peripheral_Init()
{
    Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);

    peripheralMotorGpioInit();

    // Setup the GAP Peripheral Role Profile
    {
        uint8_t  initial_advertising_enable = TRUE;
        uint16_t desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
        uint16_t desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;

        // Set the GAP Role Parameters
        GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &initial_advertising_enable);
        GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData), scanRspData);
        GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);
        GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t), &desired_min_interval);
        GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t), &desired_max_interval);
    }

    // Set advertising interval
    {
        uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;

        GAP_SetParamValue(TGAP_DISC_ADV_INT_MIN, advInt);
        GAP_SetParamValue(TGAP_DISC_ADV_INT_MAX, advInt);
    }

    // Setup the GAP Bond Manager
    {
        uint32_t passkey = 0; // passkey "000000"
        uint8_t  pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
        uint8_t  mitm = TRUE;
        uint8_t  bonding = TRUE;
        uint8_t  ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
        GAPBondMgr_SetParameter(GAPBOND_PERI_DEFAULT_PASSCODE, sizeof(uint32_t), &passkey);
        GAPBondMgr_SetParameter(GAPBOND_PERI_PAIRING_MODE, sizeof(uint8_t), &pairMode);
        GAPBondMgr_SetParameter(GAPBOND_PERI_MITM_PROTECTION, sizeof(uint8_t), &mitm);
        GAPBondMgr_SetParameter(GAPBOND_PERI_IO_CAPABILITIES, sizeof(uint8_t), &ioCap);
        GAPBondMgr_SetParameter(GAPBOND_PERI_BONDING_ENABLED, sizeof(uint8_t), &bonding);
    }

    // Initialize GATT attributes
    GGS_AddService(GATT_ALL_SERVICES);           // GAP
    GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes
    DevInfo_AddService();                        // Device Information Service
    SimpleProfile_AddService(GATT_ALL_SERVICES); // Simple GATT Profile

    // Set the GAP Characteristics
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

    // Setup the SimpleProfile Characteristic Values
    {
        uint8_t charValue1[SIMPLEPROFILE_CHAR1_LEN] = {0, 0, 0};
        uint8_t charValue2[SIMPLEPROFILE_CHAR2_LEN] = {0};
        uint8_t charValue3[SIMPLEPROFILE_CHAR3_LEN] = {0};
        uint8_t charValue4[SIMPLEPROFILE_CHAR4_LEN] = {4};
        uint8_t charValue5[SIMPLEPROFILE_CHAR5_LEN] = {0};

        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR1, SIMPLEPROFILE_CHAR1_LEN, charValue1);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR2, SIMPLEPROFILE_CHAR2_LEN, charValue2);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, SIMPLEPROFILE_CHAR3_LEN, charValue3);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR4, SIMPLEPROFILE_CHAR4_LEN, charValue4);
        SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5);
    }

    // Init Connection Item
    peripheralInitConnItem(&peripheralConnList);

    // Register callback with SimpleGATTprofile
    SimpleProfile_RegisterAppCBs(&Peripheral_SimpleProfileCBs);

    // Register receive scan request callback
    GAPRole_BroadcasterSetCB(&Broadcaster_BroadcasterCBs);

    // Setup a delayed profile startup
    tmos_set_event(Peripheral_TaskID, SBP_START_DEVICE_EVT);

    // Establish a known realtime position after every power-up.
    motorStartHoming();
}

/*********************************************************************
 * @fn      peripheralInitConnItem
 *
 * @brief   Init Connection Item
 *
 * @param   peripheralConnList -
 *
 * @return  NULL
 */
static void peripheralInitConnItem(peripheralConnItem_t *peripheralConnList)
{
    peripheralConnList->connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList->connInterval = 0;
    peripheralConnList->connSlaveLatency = 0;
    peripheralConnList->connTimeout = 0;
}

/*********************************************************************
 * @fn      peripheralMotorGpioInit
 *
 * @brief   Initialize four motor outputs and two Hall inputs.
 *
 * @return  none
 */
static void peripheralMotorGpioInit(void)
{
    GPIOA_ResetBits(MOTOR1_IN1_PIN | MOTOR1_IN2_PIN | MOTOR2_IN1_PIN | MOTOR2_IN2_PIN);
    GPIOA_ModeCfg(MOTOR1_IN1_PIN | MOTOR1_IN2_PIN | MOTOR2_IN1_PIN | MOTOR2_IN2_PIN,
                  GPIO_ModeOut_PP_5mA);
    GPIOB_ModeCfg(MOTOR1_HALL_PIN | MOTOR2_HALL_PIN, GPIO_ModeIN_PU);
    GPIOB_ModeCfg(LED1_PIN | LED2_PIN, GPIO_ModeOut_PP_5mA);
    lightAllOff();
}

/*********************************************************************
 * @fn      Peripheral_ProcessEvent
 *
 * @brief   Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16_t Peripheral_ProcessEvent(uint8_t task_id, uint16_t events)
{
    //  VOID task_id; // TMOS required parameter that isn't used in this function

    if(events & SYS_EVENT_MSG)
    {
        uint8_t *pMsg;

        if((pMsg = tmos_msg_receive(Peripheral_TaskID)) != NULL)
        {
            Peripheral_ProcessTMOSMsg((tmos_event_hdr_t *)pMsg);
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

    if(events & SBP_START_DEVICE_EVT)
    {
        // Start the Device
        GAPRole_PeripheralStartDevice(Peripheral_TaskID, &Peripheral_BondMgrCBs, &Peripheral_PeripheralCBs);
        return (events ^ SBP_START_DEVICE_EVT);
    }

    if(events & SBP_MOTOR_EVT)
    {
        tmos_start_task(Peripheral_TaskID, SBP_MOTOR_EVT, SBP_MOTOR_EVT_PERIOD);
        motorTaskHandler();
        return (events ^ SBP_MOTOR_EVT);
    }

    if(events & SBP_PARAM_UPDATE_EVT)
    {
        // Send connect param update request
        GAPRole_PeripheralConnParamUpdateReq(peripheralConnList.connHandle,
                                             DEFAULT_DESIRED_MIN_CONN_INTERVAL,
                                             DEFAULT_DESIRED_MAX_CONN_INTERVAL,
                                             DEFAULT_DESIRED_SLAVE_LATENCY,
                                             DEFAULT_DESIRED_CONN_TIMEOUT,
                                             Peripheral_TaskID);

        return (events ^ SBP_PARAM_UPDATE_EVT);
    }

    if(events & SBP_READ_RSSI_EVT)
    {
        GAPRole_ReadRssiCmd(peripheralConnList.connHandle);
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);
        return (events ^ SBP_READ_RSSI_EVT);
    }

    // Discard unknown events
    return 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void Peripheral_ProcessTMOSMsg(tmos_event_hdr_t *pMsg)
{
    switch(pMsg->event)
    {
        case GAP_MSG_EVENT:
        {
            break;
        }

        case GATT_MSG_EVENT:
        {
            gattMsgEvent_t *pMsgEvent;

            pMsgEvent = (gattMsgEvent_t *)pMsg;
            if(pMsgEvent->method == ATT_MTU_UPDATED_EVENT)
            {
                peripheralMTU = pMsgEvent->msg.exchangeMTUReq.clientRxMTU;
                PRINT("mtu exchange: %d\n", pMsgEvent->msg.exchangeMTUReq.clientRxMTU);
            }
            break;
        }

        default:
            break;
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkEstablished
 *
 * @brief   Process link established.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkEstablished(gapRoleEvent_t *pEvent)
{
    gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *)pEvent;

    // See if already connected
    if(peripheralConnList.connHandle != GAP_CONNHANDLE_INIT)
    {
        GAPRole_TerminateLink(pEvent->linkCmpl.connectionHandle);
        PRINT("Connection max...\n");
    }
    else
    {
        peripheralConnList.connHandle = event->connectionHandle;
        peripheralConnList.connInterval = event->connInterval;
        peripheralConnList.connSlaveLatency = event->connLatency;
        peripheralConnList.connTimeout = event->connTimeout;
        peripheralMTU = ATT_MTU_SIZE;
        // Set timer for param update event
        tmos_start_task(Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY);

        // Start read rssi
        tmos_start_task(Peripheral_TaskID, SBP_READ_RSSI_EVT, SBP_READ_RSSI_EVT_PERIOD);

        PRINT("Conn %x - Int %x \n", event->connectionHandle, event->connInterval);
    }
}

/*********************************************************************
 * @fn      Peripheral_LinkTerminated
 *
 * @brief   Process link terminated.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkTerminated(gapRoleEvent_t *pEvent)
{
    gapTerminateLinkEvent_t *event = (gapTerminateLinkEvent_t *)pEvent;

    if(event->connectionHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
        peripheralConnList.connInterval = 0;
        peripheralConnList.connSlaveLatency = 0;
        peripheralConnList.connTimeout = 0;
        tmos_stop_task(Peripheral_TaskID, SBP_READ_RSSI_EVT);

        // A BLE write starts an autonomous board-side movement. Do not stop it
        // when the one-shot Python client disconnects after write_gatt_char().

        // Restart advertising
        {
            uint8_t advertising_enable = TRUE;
            GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t), &advertising_enable);
        }
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralRssiCB
 *
 * @brief   RSSI callback.
 *
 * @param   connHandle - connection handle
 * @param   rssi - RSSI
 *
 * @return  none
 */
static void peripheralRssiCB(uint16_t connHandle, int8_t rssi)
{
    PRINT("RSSI -%d dB Conn  %x \n", -rssi, connHandle);
}

/*********************************************************************
 * @fn      peripheralParamUpdateCB
 *
 * @brief   Parameter update complete callback
 *
 * @param   connHandle - connect handle
 *          connInterval - connect interval
 *          connSlaveLatency - connect slave latency
 *          connTimeout - connect timeout
 *
 * @return  none
 */
static void peripheralParamUpdateCB(uint16_t connHandle, uint16_t connInterval,
                                    uint16_t connSlaveLatency, uint16_t connTimeout)
{
    if(connHandle == peripheralConnList.connHandle)
    {
        peripheralConnList.connInterval = connInterval;
        peripheralConnList.connSlaveLatency = connSlaveLatency;
        peripheralConnList.connTimeout = connTimeout;

        PRINT("Update %x - Int %x \n", connHandle, connInterval);
    }
    else
    {
        PRINT("ERR..\n");
    }
}

/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB(gapRole_States_t newState, gapRoleEvent_t *pEvent)
{
    switch(newState)
    {
        case GAPROLE_STARTED:
            PRINT("Initialized..\n");
            break;

        case GAPROLE_ADVERTISING:
            if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
                PRINT("Advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Advertising..\n");
            }
            break;

        case GAPROLE_CONNECTED:
            if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                Peripheral_LinkEstablished(pEvent);
                PRINT("Connected..\n");
            }
            break;

        case GAPROLE_CONNECTED_ADV:
            if(pEvent->gap.opcode == GAP_MAKE_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Connected Advertising..\n");
            }
            break;

        case GAPROLE_WAITING:
            if(pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT)
            {
                PRINT("Waiting for advertising..\n");
            }
            else if(pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT)
            {
                Peripheral_LinkTerminated(pEvent);
                PRINT("Disconnected.. Reason:%x\n", pEvent->linkTerminate.reason);
            }
            else if(pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT)
            {
                if(pEvent->gap.hdr.status != SUCCESS)
                {
                    PRINT("Waiting for advertising..\n");
                }
                else
                {
                    PRINT("Error..\n");
                }
            }
            else
            {
                PRINT("Error..%x\n", pEvent->gap.opcode);
            }
            break;

        case GAPROLE_ERROR:
            PRINT("Error..\n");
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      breathPwmHandler
 *
 * @brief   Breathing LED PWM handler. Called by SBP_BREATH_EVT.
 *          Implements software PWM at ~100Hz (16 sub-cycles of 625us)
 *          and modulates duty cycle using a sine LUT for breathing effect.
 *
 * @param   none
 *
 * @return  none
 */
static void motorStop(void)
{
    GPIOA_ResetBits(MOTOR1_IN1_PIN | MOTOR1_IN2_PIN | MOTOR2_IN1_PIN | MOTOR2_IN2_PIN);
    activeMotor = 0;
    activeDirection = MOTOR_DIR_STOP;
    activeDuty = 0;
    motorPwmPhase = 0;
    hallStableCount = 0;
    hallDebugPrintCount = 0;
    motorRunCount = 0;
    tmos_stop_task(Peripheral_TaskID, SBP_MOTOR_EVT);
}

static void motorStart(uint8_t motor, uint8_t direction, uint8_t speed)
{
    motorStop();

    activeMotor = motor;
    activeDirection = direction;
    activeDuty = (uint8_t)(((uint16_t)speed * MOTOR_PWM_STEPS + 254) / 255);
    if(activeDuty == 0)
    {
        activeDuty = 1;
    }

    lastHallLevel = (motor == 1)
                        ? (GPIOB_ReadPortPin(MOTOR1_HALL_PIN) ? 1 : 0)
                        : (GPIOB_ReadPortPin(MOTOR2_HALL_PIN) ? 1 : 0);
    PRINT("H%d=%d\n", motor, lastHallLevel);

    // It is trusted as reverse-ready again only after a completed reverse run.
    motorReverseReady[motor - 1] = 0;
    motorRotationCount[motor - 1] = 0;

    tmos_start_task(Peripheral_TaskID, SBP_MOTOR_EVT, SBP_MOTOR_EVT_PERIOD);
    PRINT("Motor %d start, dir=%d, speed=%d, rotations=0\n", motor, direction, speed);
}

static uint32_t motorTotalHallCount(uint8_t motor)
{
    return (motor == 1) ? MOTOR1_TOTAL_HALL_COUNT : MOTOR2_TOTAL_HALL_COUNT;
}

static uint8_t motorPositionPercent(uint8_t motor)
{
    uint32_t total = motorTotalHallCount(motor);
    uint32_t position = motorRealtimePosition[motor - 1];

    if((total == 0) || (position >= total))
    {
        return (position == 0) ? 0 : 100;
    }
    return (uint8_t)((position * 100UL) / total);
}

static void motorClearPending(void)
{
    pendingMotor = 0;
    pendingDirection = MOTOR_DIR_STOP;
    pendingSpeed = 0;
}

static void motorStartHoming(void)
{
    homingComplete = 0;
    jogCommandMotor = 0;
    motorClearPending();
    controlMode = CONTROL_MODE_MOTOR;
    motorAction = MOTOR_ACTION_HOME_MOTOR1;
    PRINT("Homing motor 1\n");
    motorStart(1, MOTOR_DIR_REVERSE, MOTOR_HOME_SPEED);
}

static void motorHandleCommand(uint8_t motor, uint8_t direction, uint8_t speed)
{
    if(direction == MOTOR_DIR_STOP)
    {
        if((motor < 1) || (motor > 2) || (motor != jogCommandMotor))
        {
            return;
        }

        jogCommandMotor = 0;
        motorClearPending();
        motorStop();
        return;
    }

    if((motor < 1) || (motor > 2) ||
       ((direction != MOTOR_DIR_FORWARD) && (direction != MOTOR_DIR_REVERSE)) ||
       (speed == 0))
    {
        return;
    }

    // Commands are ignored until both motors have completed boot homing.
    if(!homingComplete)
    {
        return;
    }

    if(controlMode != CONTROL_MODE_MOTOR)
    {
        lightAllOff();
        controlMode = CONTROL_MODE_MOTOR;
    }

    // Every new command replaces the current/pending command. Only one motor can run.
    jogCommandMotor = motor;
    motorClearPending();
    motorAction = MOTOR_ACTION_USER;

    // Check motor 2 first when reversing motor 2: if motor 2 is already at 0
    // and reverse-ready, bring motor 1 up to MOTOR1_WORK_MIN_HALL_COUNT only
    // when motor 1 is below it; if motor 1 is already above it, keep motor 1 still.
    if((motor == 2) && (direction == MOTOR_DIR_REVERSE) &&
       (motorRealtimePosition[1] == 0) && motorReverseReady[1])
    {
        if(motorRealtimePosition[0] < MOTOR1_WORK_MIN_HALL_COUNT)
        {
            motorAction = MOTOR_ACTION_GOTO_MOTOR1_WORK_MIN;
            motorStart(1, MOTOR_DIR_FORWARD, speed);
        }
        else
        {
            jogCommandMotor = 0;
        }
        return;
    }

    // Motor 2 is never allowed to move until motor 1 has crossed the 30%
    // boundary and reached its physical zero endpoint.
    if((motor == 2) &&
       ((motorRealtimePosition[0] > 0) || !motorReverseReady[0]))
    {
        pendingMotor = motor;
        pendingDirection = direction;
        pendingSpeed = speed;
        motorAction = MOTOR_ACTION_CLEAR_FOR_PENDING;
        motorStart(1, MOTOR_DIR_REVERSE, speed);
        return;
    }

    // Motor 1 may move only while motor 2 is at zero. Retract motor 2 first
    // if a new motor-1 command arrives while motor 2 is extended.
    if((motor == 1) &&
       ((motorRealtimePosition[1] > 0) || !motorReverseReady[1]))
    {
        pendingMotor = motor;
        pendingDirection = direction;
        pendingSpeed = speed;
        motorAction = MOTOR_ACTION_CLEAR_FOR_PENDING;
        motorStart(2, MOTOR_DIR_REVERSE, speed);
        return;
    }

    if(direction == MOTOR_DIR_REVERSE)
    {
        if((motor == 1) && (motorRealtimePosition[0] < MOTOR1_WORK_MIN_HALL_COUNT))
        {
            motorAction = MOTOR_ACTION_GOTO_MOTOR1_WORK_MIN;
            motorStart(1, MOTOR_DIR_FORWARD, speed);
            return;
        }

        if(((motorRealtimePosition[motor - 1] == 0) &&
            motorReverseReady[motor - 1]) ||
           ((motor == 1) &&
            (motorRealtimePosition[0] == MOTOR1_WORK_MIN_HALL_COUNT)))
        {
            jogCommandMotor = 0;
            return;
        }
    }
    else if(motorRealtimePosition[motor - 1] >= motorTotalHallCount(motor))
    {
        jogCommandMotor = 0;
        return;
    }

    motorStart(motor, direction, speed);
}

static void motorTaskHandler(void)
{
    uint8_t hallLevel;
    uint8_t finishedMotor;
    uint8_t finishedDirection;
    uint8_t nextMotor;
    uint8_t nextDirection;
    uint8_t nextSpeed;

    if(controlMode != CONTROL_MODE_MOTOR)
    {
        lightTaskHandler();
        return;
    }

    if(activeMotor == 0)
    {
        motorStop();
        return;
    }

    motorPwmPhase = (uint8_t)((motorPwmPhase + 1) % MOTOR_PWM_STEPS);

    // Set only one input at a time; the opposite input always remains low.
    GPIOA_ResetBits(MOTOR1_IN1_PIN | MOTOR1_IN2_PIN | MOTOR2_IN1_PIN | MOTOR2_IN2_PIN);
    if(motorPwmPhase < activeDuty)
    {
        if(activeMotor == 1)
        {
            GPIOA_SetBits((activeDirection == MOTOR_DIR_FORWARD) ? MOTOR1_IN1_PIN : MOTOR1_IN2_PIN);
        }
        else
        {
            GPIOA_SetBits((activeDirection == MOTOR_DIR_FORWARD) ? MOTOR2_IN1_PIN : MOTOR2_IN2_PIN);
        }
    }

    hallLevel = (activeMotor == 1)
                    ? (GPIOB_ReadPortPin(MOTOR1_HALL_PIN) ? 1 : 0)
                    : (GPIOB_ReadPortPin(MOTOR2_HALL_PIN) ? 1 : 0);
    if(hallLevel != lastHallLevel)
    {
        lastHallLevel = hallLevel;
        hallStableCount = 0;
        hallDebugPrintCount = 0;
        motorRotationCount[activeMotor - 1]++;
        hallTriggerCount[activeMotor - 1]++;

        // Homing establishes zero. After that, every Hall edge updates the
        // signed travel direction and therefore the realtime position.
        if(homingComplete)
        {
            if(activeDirection == MOTOR_DIR_FORWARD)
            {
                if(motorRealtimePosition[activeMotor - 1] < motorTotalHallCount(activeMotor))
                {
                    motorRealtimePosition[activeMotor - 1]++;
                }
            }
            else if(motorRealtimePosition[activeMotor - 1] > 0)
            {
                motorRealtimePosition[activeMotor - 1]--;
            }
        }

        PRINT("H%d:%d C%d T%d P%d%%\n",
              activeMotor,
              hallLevel,
              motorRotationCount[activeMotor - 1],
              hallTriggerCount[activeMotor - 1],
              motorPositionPercent(activeMotor));

        // After motor 2 reverses to 0, move motor 1 to MOTOR1_WORK_MIN_HALL_COUNT
        // and stop once reached.
        if((activeMotor == 1) && (motorAction == MOTOR_ACTION_GOTO_MOTOR1_WORK_MIN))
        {
            if(((activeDirection == MOTOR_DIR_FORWARD) &&
                (motorRealtimePosition[0] >= MOTOR1_WORK_MIN_HALL_COUNT)) ||
               ((activeDirection == MOTOR_DIR_REVERSE) &&
                (motorRealtimePosition[0] <= MOTOR1_WORK_MIN_HALL_COUNT)))
            {
                motorRealtimePosition[0] = MOTOR1_WORK_MIN_HALL_COUNT;
                motorAction = MOTOR_ACTION_USER;
                jogCommandMotor = 0;
                motorStop();
                PRINT("Motor 1 arrived at WORK_MIN, P=%d\n",
                      motorRealtimePosition[0]);
                return;
            }
        }

        // In normal operation motor 1 may retract only to 30% while motor 2
        // remains at zero. The interlock action deliberately bypasses this
        // boundary and continues to the physical zero endpoint.
        if((activeMotor == 1) && (activeDirection == MOTOR_DIR_REVERSE) &&
           (motorAction == MOTOR_ACTION_USER) &&
           (motorRealtimePosition[0] <= MOTOR1_WORK_MIN_HALL_COUNT))
        {
            motorRealtimePosition[0] = MOTOR1_WORK_MIN_HALL_COUNT;
            jogCommandMotor = 0;
            motorStop();
            PRINT("Motor 1 stopped at 30%% boundary, P=%d\n",
                  motorRealtimePosition[0]);
            return;
        }

        if((activeDirection == MOTOR_DIR_FORWARD) &&
           (motorRealtimePosition[activeMotor - 1] >= motorTotalHallCount(activeMotor)))
        {
            motorRealtimePosition[activeMotor - 1] = motorTotalHallCount(activeMotor);
            PRINT("Motor %d reached configured 100%% position\n", activeMotor);
            jogCommandMotor = 0;
            motorStop();
            return;
        }
    }
    else if(hallStableCount < HALL_STABLE_TICKS)
    {
        hallStableCount++;
    }

    hallDebugPrintCount++;
    if(hallDebugPrintCount >= HALL_DEBUG_PRINT_TICKS)
    {
        hallDebugPrintCount = 0;
        PRINT("H%d=%d S%d\n",
              activeMotor, hallLevel, hallStableCount);
    }

    motorRunCount++;
    if(motorRunCount >= MOTOR_MAX_RUN_TICKS)
    {
        PRINT("Motor timeout\n");
        jogCommandMotor = 0;
        motorClearPending();
        motorStop();
        return;
    }

    if((motorRunCount < MOTOR_MIN_RUN_TICKS) || (hallStableCount < HALL_STABLE_TICKS))
    {
        return;
    }

    finishedMotor = activeMotor;
    finishedDirection = activeDirection;
    if(finishedDirection == MOTOR_DIR_REVERSE)
    {
        motorReverseReady[finishedMotor - 1] = 1;
        motorRealtimePosition[finishedMotor - 1] = 0;
    }
    else
    {
        motorRealtimePosition[finishedMotor - 1] = motorTotalHallCount(finishedMotor);
    }

    nextMotor = pendingMotor;
    nextDirection = pendingDirection;
    nextSpeed = pendingSpeed;
    motorClearPending();
    motorStop();
    PRINT("Motor %d arrived, rotations=%d, hall triggers=%d\n",
          finishedMotor,
          motorRotationCount[finishedMotor - 1],
          hallTriggerCount[finishedMotor - 1]);

    if(motorAction == MOTOR_ACTION_HOME_MOTOR1)
    {
        motorAction = MOTOR_ACTION_HOME_MOTOR2;
        PRINT("Homing motor 2\n");
        motorStart(2, MOTOR_DIR_REVERSE, MOTOR_HOME_SPEED);
    }
    else if(motorAction == MOTOR_ACTION_HOME_MOTOR2)
    {
        motorAction = MOTOR_ACTION_USER;
        homingComplete = 1;
        motorRealtimePosition[0] = 0;
        motorRealtimePosition[1] = 0;
        PRINT("Homing complete, P1=0 P2=0\n");
    }
    // Continue the deferred command only after the blocking motor has
    // physically reached zero.
    else if((nextMotor != 0) && (finishedDirection == MOTOR_DIR_REVERSE) &&
            (finishedMotor != nextMotor))
    {
        motorAction = MOTOR_ACTION_USER;
        motorStart(nextMotor, nextDirection, nextSpeed);
    }
    // When motor 2 finishes reversing to 0 during normal user control,
    // immediately move motor 1 to MOTOR1_WORK_MIN_HALL_COUNT and stop.
    else if((finishedMotor == 2) && (finishedDirection == MOTOR_DIR_REVERSE) &&
            (motorAction == MOTOR_ACTION_USER))
    {
        if(motorRealtimePosition[0] < MOTOR1_WORK_MIN_HALL_COUNT)
        {
            motorAction = MOTOR_ACTION_GOTO_MOTOR1_WORK_MIN;
            motorStart(1, MOTOR_DIR_FORWARD, nextSpeed ? nextSpeed : MOTOR_HOME_SPEED);
        }
        else if(motorRealtimePosition[0] > MOTOR1_WORK_MIN_HALL_COUNT)
        {
            motorAction = MOTOR_ACTION_GOTO_MOTOR1_WORK_MIN;
            motorStart(1, MOTOR_DIR_REVERSE, nextSpeed ? nextSpeed : MOTOR_HOME_SPEED);
        }
        else
        {
            jogCommandMotor = 0;
        }
    }
    else
    {
        jogCommandMotor = 0;
    }
}

static void lightWriteChannel(uint8_t channel, uint8_t on)
{
    if(channel == 0)
    {
        if(on)
        {
            GPIOB_ResetBits(LED1_PIN);
        }
        else
        {
            GPIOB_SetBits(LED1_PIN);
        }
    }
    else if(channel == 1)
    {
        if(on) GPIOB_SetBits(LED2_PIN);
        else GPIOB_ResetBits(LED2_PIN);
    }
    else if(channel == 2)
    {
        if(on) GPIOA_SetBits(MOTOR1_IN1_PIN);
        else GPIOA_ResetBits(MOTOR1_IN1_PIN);
    }
    else if(channel == 3)
    {
        if(on) GPIOA_SetBits(MOTOR1_IN2_PIN);
        else GPIOA_ResetBits(MOTOR1_IN2_PIN);
    }
}

static void lightAllOff(void)
{
    uint8_t channel;

    for(channel = 0; channel < 4; channel++)
    {
        lightWriteChannel(channel, 0);
    }
}

static void lightSetMode(uint8_t mode)
{
    uint8_t channel;

    if((mode < CONTROL_MODE_LIGHT_SWITCH) ||
       (mode > CONTROL_MODE_LIGHT_BRIGHTNESS))
    {
        return;
    }

    jogCommandMotor = 0;
    motorClearPending();
    motorStop();
    controlMode = mode;
    lightPwmPhase = 0;
    lightBreathLevel = 0;
    lightBreathRising = 1;
    lightBreathHold = 0;
    for(channel = 0; channel < 4; channel++)
    {
        lightValues[channel] = 0;
    }
    lightAllOff();

    if(mode != CONTROL_MODE_LIGHT_SWITCH)
    {
        tmos_start_task(Peripheral_TaskID, SBP_MOTOR_EVT, SBP_MOTOR_EVT_PERIOD);
    }
}

static void lightSetChannel(uint8_t channel, uint8_t value)
{
    if((channel >= 4) || (controlMode == CONTROL_MODE_MOTOR))
    {
        return;
    }

    if(controlMode == CONTROL_MODE_LIGHT_SWITCH)
    {
        lightValues[channel] = value ? 1 : 0;
        lightWriteChannel(channel, lightValues[channel]);
    }
    else if(controlMode == CONTROL_MODE_LIGHT_BREATH)
    {
        lightValues[channel] = value ? 1 : 0;
    }
    else
    {
        lightValues[channel] = (value > 100) ? 100 : value;
    }
}

static void lightTaskHandler(void)
{
    uint8_t channel;
    uint8_t duty;

    lightPwmPhase = (uint8_t)((lightPwmPhase + 1) % MOTOR_PWM_STEPS);
    if((controlMode == CONTROL_MODE_LIGHT_BREATH) && (lightPwmPhase == 0))
    {
        lightBreathHold++;
        if(lightBreathHold >= 3)
        {
            lightBreathHold = 0;
            if(lightBreathRising)
            {
                if(++lightBreathLevel >= MOTOR_PWM_STEPS)
                {
                    lightBreathLevel = MOTOR_PWM_STEPS;
                    lightBreathRising = 0;
                }
            }
            else if(lightBreathLevel > 0)
            {
                lightBreathLevel--;
            }
            else
            {
                lightBreathRising = 1;
            }
        }
    }

    for(channel = 0; channel < 4; channel++)
    {
        if(controlMode == CONTROL_MODE_LIGHT_BREATH)
        {
            duty = lightValues[channel] ? lightBreathLevel : 0;
        }
        else
        {
            duty = (uint8_t)(((uint16_t)lightValues[channel] * MOTOR_PWM_STEPS + 99) / 100);
        }
        lightWriteChannel(channel, lightPwmPhase < duty);
    }
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *          pValue - pointer to data that was changed
 *          len - length of data
 *
 * @return  none
 */
static void simpleProfileChangeCB(uint8_t paramID, uint8_t *pValue, uint16_t len)
{
    if(!homingComplete)
    {
        return;
    }

    if((paramID == SIMPLEPROFILE_CHAR1) && (len == SIMPLEPROFILE_CHAR1_LEN))
    {
        // FFE1: [motor:1/2, direction, speed] or [0, light mode:1/2/3, 0].
        if(pValue[0] == 0)
        {
            if(pValue[1] == CONTROL_MODE_MOTOR)
            {
                lightAllOff();
                motorStop();
                controlMode = CONTROL_MODE_MOTOR;
            }
            else
            {
                lightSetMode(pValue[1]);
            }
        }
        else
        {
            motorHandleCommand(pValue[0], pValue[1], pValue[2]);
        }
    }
    else if((paramID >= SIMPLEPROFILE_CHAR2) &&
            (paramID <= SIMPLEPROFILE_CHAR5) && (len == 1))
    {
        lightSetChannel((uint8_t)(paramID - SIMPLEPROFILE_CHAR2), pValue[0]);
    }
}

/*********************************************************************
*********************************************************************/
