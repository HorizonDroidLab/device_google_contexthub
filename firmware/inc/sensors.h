#ifndef _SENSORS_H_
#define _SENSORS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <seos.h>
#include <eventnums.h>
#include <sensType.h>

#define MAX_REGISTERED_SENSORS  16 /* this may need to be revisted later */

//data for accel/gyro/mag takes this form
struct TrippleAxisDataPoint {
    uint32_t deltaTime; //delta since last sample, for 0th sample this is "num samples"
    int16_t x, y, z;
} __attribute__((packed));

struct TrippleAxisDataEvent {
    uint64_t referenceTime;
    struct TrippleAxisDataPoint samples[];
};





struct SensorOps {
    bool (*sensorPower)(bool on);          /* -> SENSOR_INTERNAL_EVT_POWER_STATE_CHG (success)         */
    bool (*sensorFirmwareUpload)(void);    /* -> SENSOR_INTERNAL_EVT_FW_STATE_CHG (rate or 0 if fail)  */
    bool (*sensorSetRate)(uint32_t rate, uint64_t latency);
                                           /* -> SENSOR_INTERNAL_EVT_RATE_CHG (rate)                   */
    bool (*sensorFlush)(void);
    bool (*sensorTriggerOndemand)(void);
};

struct SensorInfo {
    const char *sensorName; /* sensors.c code doe snot use this */
    const uint32_t *supportedRates;
    struct SensorOps ops;
    uint8_t sensorType;
    /* for some sensors more data may be relevant: */
    union {
        uint8_t numAxes;
    };
};


/*
 * Sensor rate is encoded as a 32-bit integer as number of samples it can
 * provide per 1024 seconds, allowing representations of all useful values
 * well. This define is to be used for static values only please, as GCC
 * will convert it into a const int at compile time. Do not use this at
 * runtime please. A few Magic values exist at both ends of the range
 * 0 is used as a list sentinel and high numbers for special abilities.
 */
#define SENSOR_RATE_ONDEMAND    0xFFFFFF00UL
#define SENSOR_RATE_ONCHANGE    0xFFFFFF01UL
#define SENSOR_HZ(_hz)          ((uint32_t)((_hz) * 1024.0f))


/*
 * sensors module api
 */
bool sensorsInit(void);

/*
 * Api for sensor drivers
 */
#define SENSOR_INTERNAL_EVT_POWER_STATE_CHG  0
#define SENSOR_INTERNAL_EVT_FW_STATE_CHG     1
#define SENSOR_INTERNAL_EVT_RATE_CHG         2

uint32_t sensorRegister(const struct SensorInfo *si); /* returns handle, copy is not made */
bool sensorUnregister(uint32_t handle); /* your job to be sure it is off already */
bool sensorSignalInternalEvt(uint32_t handle, uint32_t intEvtNum, uint32_t value1, uint64_t value2);

#define sensorGetMyEventType(_sensorType) (EVT_NO_FIRST_SENSOR_EVENT + (_sensorType))


/*
 * api for using sensors (enum is not synced with sensor sub/unusb, this is ok since we do not expect a lot of dynamic sub/unsub)
 * client ID should almost always be your TID (as we have no other way to disambiguate them)
 */
const struct SensorInfo* sensorFind(uint32_t sensorType, uint32_t idx, uint32_t *handleP); //enumerate all sensors of a type
bool sensorRequest(uint32_t clientId, uint32_t sensorHandle, uint32_t rate, uint64_t latency);
bool sensorRequestRateChange(uint32_t clientId, uint32_t sensorHandle, uint32_t newRate, uint64_t newLatency);
bool sensorRelease(uint32_t clientId, uint32_t sensorHandle);
bool sensorTriggerOndemand(uint32_t clientId, uint32_t sensorHandle);
bool sensorFlush(uint32_t sensorHandle);
uint32_t sensorGetCurRate(uint32_t sensorHandle);
uint64_t sensorGetCurLatency(uint32_t sensorHandle);


#ifdef __cplusplus
}
#endif

#endif

