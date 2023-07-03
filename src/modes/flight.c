/**
 * Source file of pico-fbw: https://github.com/MylesAndMore/pico-fbw
 * Licensed under the GNU GPL-3.0
*/

#include <stdbool.h>

#include "../io/flash.h"
#include "../io/servo.h"
#include "../lib/pid.h"

#include "../config.h"

#include "modes.h"
#include "tune.h"

#include "flight.h"

bool flightInitialized = false;

PIDController rollController;
PIDController pitchController;
PIDController yawController;

float flight_yawOutput;
float flight_yawSetpoint;
bool flight_ydOn;

// TODO: centralize getting angles to here instead of in every file...
// and possibly checking flight envelope if I'm getting IMU data in this file anyways?

void flight_init() {
    if (!flightInitialized) {
        // Create PID controllers for the roll and pitch axes and initialize them
        #ifdef PID_AUTOTUNE
            rollController = (PIDController){flash_read(1, 1), flash_read(1, 2), flash_read(1, 3), roll_tau, -AIL_LIMIT, AIL_LIMIT, roll_integMin, roll_integMax, roll_kT};
            pitchController = (PIDController){flash_read(2, 1), flash_read(2, 2), flash_read(2, 3), pitch_tau, -ELEV_LIMIT, ELEV_LIMIT, pitch_integMin, pitch_integMax, pitch_kT};
        #else
            rollController = (PIDController){roll_kP, roll_kI, roll_kD, roll_tau, -AIL_LIMIT, AIL_LIMIT, roll_integMin, roll_integMax, roll_kT};
            pitchController = (PIDController){pitch_kP, pitch_kI, pitch_kD, pitch_tau, -ELEV_LIMIT, ELEV_LIMIT, pitch_integMin, pitch_integMax, pitch_kT};
        #endif
        yawController = (PIDController){yaw_kP, yaw_kI, yaw_kD, yaw_tau, -RUD_LIMIT, RUD_LIMIT, yaw_integMin, yaw_integMax, yaw_kT};
        pid_init(&rollController);
        pid_init(&pitchController);
        pid_init(&yawController);
    }
}

void flight_update(double rollSetpoint, double rollAngle, double pitchSetpoint, double pitchAngle, double yawSetpoint, double yawAngle, bool yawOverride) {
    // Update PIDs with new supplied data and output to servos
    pid_update(&rollController, rollSetpoint, rollAngle);
    pid_update(&pitchController, pitchSetpoint, pitchAngle);
    if (yawOverride) {
        // Yaw override (raw)
        flight_yawOutput = yawSetpoint;
        flight_ydOn = false;
    } else if (rollSetpoint > DEADBAND_VALUE || rollSetpoint < -DEADBAND_VALUE) {
        // Yaw damper disabled (passthrough)
        flight_yawOutput = rollController.out * RUDDER_TURNING_VALUE;
        flight_ydOn = false;
    } else {
        // Yaw damper enabled
        if (!flight_ydOn) {
            flight_yawSetpoint = yawAngle; // Yaw damper was just enabled, create our setpoint
        }
        pid_update(&yawController, flight_yawSetpoint, yawAngle);
        flight_yawOutput = yawController.out;
        flight_ydOn = true;
    }
    servo_set(SERVO_AIL_PIN, (uint16_t)(rollController.out + 90));
    servo_set(SERVO_ELEV_PIN, (uint16_t)(rollController.out + 90));
    servo_set(SERVO_RUD_PIN, (uint16_t)(flight_yawOutput + 90));
}

double flight_getRollOut() { return rollController.out; }

double flight_getPitchOut() { return pitchController.out; }

bool flight_checkEnvelope(float rollAngle, float pitchAngle) {
    // Failsafe to check if actual IMU values are too out of spec and revert to direct.
    // The PID loops should keep us from getting to this point, so we should disable if we ever get here--IMU data could be at fault
    if (rollAngle > 72 || rollAngle < -72 || pitchAngle > 35 || pitchAngle < -20) {
        setIMUSafe(false);
        return false;
    } else {
        return true;
    }
}
