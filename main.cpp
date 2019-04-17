#include "hardware_monitoring.h"

int main() {
    cpu_monitor();
    fan_monitor();
    gpu_monitor();
    return 0;
}
