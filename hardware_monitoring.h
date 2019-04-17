#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem> //For filesystem
#include <unistd.h>
#include <sys/reboot.h>
#include <stdio.h>
#include <cstdlib>
#include <sstream>
#include <cuda_runtime.h>
//Library for monitoring gpu.Must install CUDA Toolkit for using this header file.
#include <nvidia/gdk/nvml.h>
using namespace std;

//CPU temperature file direction
const string CPU_INFO_DIR = "/sys/class/thermal/thermal_zone0/temp";
//Fan information file direction
const experimental::filesystem::path FAN_INFO_DIR = "/sys/class/hwmon";
//Max temperature of CPU and GPU
#define CPU_MAX_TEMP 100
#define GPU_MAX_TEMP 105
//Max speed of fan
#define FAN_MAX_SPEED 6500
#define SHUTDOWN_SECONDS 5000000

//Gets the cpu temperature and if it is critical restart the machine.
//CPU temperature file path is standart in most linux distros
void cpu_monitor();

//Gets the fan speed and if it is critical restart the machine.
//Fan information file path can differ from device to device
void fan_monitor();

//Find fans information files in hwmon directions
void find_fan_info(const experimental::filesystem::path& fun_info_path);

//Gets the gpu information(temperature,memory size,fan speed,etc.) and if it is critical restart the machine.
void gpu_monitor();

#endif /* TEMPERATURE_H */
