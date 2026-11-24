#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

/*
 * Hall calibration values for one complete stroke.
 * Replace these two values with the measured Hall edge counts.
 */
#define MOTOR1_TOTAL_HALL_COUNT            1772UL
#define MOTOR2_TOTAL_HALL_COUNT            750UL

/* Motor 1 may normally retract only to this percentage while motor 2 is at 0. */
#define MOTOR1_WORK_MIN_PERCENT            30UL

/* Homing always runs in reverse until the Hall signal becomes stable. */
#define MOTOR_HOME_SPEED                   255

#endif
