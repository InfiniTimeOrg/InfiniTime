# Motion Service

## Introduction

The motion service exposes step count and raw X/Y/Z motion value as READ and NOTIFY characteristics.

## Service

The service UUID is **00030000-78fc-48fe-8e23-433b3a1942d0**

## Characteristics

### Step count (UUID 00030001-78fc-48fe-8e23-433b3a1942d0)

The current number of steps represented as a single `uint32_t` (4 bytes) value.

### Raw motion values (UUID 00030002-78fc-48fe-8e23-433b3a1942d0)

The current raw motion values. This is a 3 `int16_t` array:

- [0] : X
- [1] : Y
- [2] : Z

The three motion values are in units of "binary milli-g", where 1g is represented by a value of 1024.

**Timing:** InfiniTime sends a Bluetooth motion notification whenever the X, Y, or Z reading changes. Because these packets do not include timestamps, you must log the arrival time of each packet to use as your horizontal axis. Spacing the data points evenly instead of using arrival times will distort the apparent speed of the wrist movement.
