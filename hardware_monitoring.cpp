#include "hardware_monitoring.h"

vector <string> fans_string;

//Gets the cpu temperature and if it is critical restart the machine.
//CPU temperature file path is standart in most linux distros
void cpu_monitor()
{
  double temp;
  FILE *info_file;
  const char* cpu_full_dir = CPU_INFO_DIR.c_str();

  if (!(info_file = fopen(cpu_full_dir, "r"))) {
      cout << "Failed to open CPU temperature file!\n";
  }
  else {
    // The temperature is stored in 5 digits.  The first two are degrees in °C.
    fscanf( info_file, "%lf", &temp);
    temp /= 1000;
    fclose (info_file);
    if(temp>CPU_MAX_TEMP){
      cout << "CPU Temperature is critical: " << temp << " °C." << "The system will restart after " << SHUTDOWN_SECONDS/1000000<< " seconds." << '\n';
      usleep(SHUTDOWN_SECONDS);
      system("shutdown -P now");
    }
    else
      cout << "CPU Temperature: " << temp << " °C" << '\n';
  }
}

//Find fans information files in hwmon directions
void find_fan_info(const experimental::filesystem::path& fun_info_path)
{
  for (const auto & entry : experimental::filesystem::directory_iterator(fun_info_path))
  {
    string path_string = entry.path().u8string();
    string string_to_find0 = path_string.substr (path_string.size()-6,6);
    if((string_to_find0 == "device") && (experimental::filesystem::is_directory(entry.path())))
    {
      find_fan_info(entry.path());
    }

    string_to_find0 = path_string.substr (path_string.size()-10,3);
    string string_to_find1 = path_string.substr (path_string.size()-6,6);
    if(((string_to_find0 == "fan") && (string_to_find1 == "_input")) && (experimental::filesystem::is_regular_file(entry.path())))
    {
      cout << "Founded fan info file in direction: " <<  fun_info_path << endl;
      string final_path_string = entry.path().u8string();
      fans_string.push_back(final_path_string);
    }
  }
}

//Gets the fan speed and if it is critical restart the machine.
//Fan information file path can differ from device to device
void fan_monitor()
{
  double fan_speed;
  FILE *info_file;

  //Find fan files in direction
  for (const auto & entry : experimental::filesystem::directory_iterator(FAN_INFO_DIR))
  {
    if(experimental::filesystem::is_directory(entry.path()))
      find_fan_info(entry.path());
  }

  //Loop for finding all fans speed
  for(int i = 0; i < fans_string.size(); i++)
  {
    const char* fan_full_dir = fans_string[i].c_str();
    if (!(info_file = fopen(fan_full_dir, "r"))) {
      cout << "Failed to open fan speed file or there is no any fan!\n";
    }
    else {
      fscanf( info_file, "%lf", &fan_speed);
      fclose (info_file);
      if(fan_speed>FAN_MAX_SPEED){
        cout << "Fan" << i << " speed is critical: " << fan_speed << " RPM." << "The system will restart after " << SHUTDOWN_SECONDS/1000000<< " seconds." << '\n';
        usleep(SHUTDOWN_SECONDS);
        system("shutdown -P now");
      }
      else
        cout << "Fan " << i+1 << " speed: " << fan_speed << " RPM" << '\n';
    }
  }
}

//Gets the gpu information(temperature,memory size,fan speed,etc.) and if it is critical restart the machine.
void gpu_monitor()
{
  int device_count;
  nvmlReturn_t result;
  unsigned int temp;
  unsigned int fan_speed;
  nvmlDevice_t device;
  const int kb = 1024;
  const int mb = kb * kb;

  cudaGetDeviceCount(&device_count);
  for (int i = 0; i < device_count; i++) {
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, i);
    cout << "Device Number: " << i << endl;
    cout << "Device name: " << prop.name << endl;
    cout << "Memory Clock Rate (KHz): " << prop.memoryClockRate << endl;
    cout << "Memory Bus Width (bits): " << prop.memoryBusWidth << endl;
    cout << "Peak Memory Bandwidth (GB/s): " << 2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6 << endl;
    cout << "Device: " << i << ": " << prop.name << ": " << prop.major << "." << prop.minor << endl;
    cout << "Global memory:   " << prop.totalGlobalMem / mb << "mb" << endl;
    cout << "Shared memory:   " << prop.sharedMemPerBlock / kb << "kb" << endl;
    cout << "Constant memory: " << prop.totalConstMem / kb << "kb" << endl;
    cout << "Block registers: " << prop.regsPerBlock << endl;
    cout << "Warp size:         " << prop.warpSize << endl;
    cout << "Threads per block: " << prop.maxThreadsPerBlock << endl;
    cout << "Max block dimensions: [ " << prop.maxThreadsDim[0] << ", " << prop.maxThreadsDim[1]  << ", " << prop.maxThreadsDim[2] << " ]" << endl;
    cout << "Max grid dimensions:  [ " << prop.maxGridSize[0] << ", " << prop.maxGridSize[1]  << ", " << prop.maxGridSize[2] << " ]" << endl;

    // Initialize NVML library
    result = nvmlInit();
    if (NVML_SUCCESS != result)
    {
      cout << "Failed to initialize NVML: " << nvmlErrorString(result) << endl;
      return;
    }
    result = nvmlDeviceGetHandleByIndex(i, &device);
    if (NVML_SUCCESS != result)
    {
      cout << "Failed to get handle for device: " << i << " " << nvmlErrorString(result) << endl;
      goto Error;
    }
    //Get GPU temperature
    result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
    if (NVML_SUCCESS != result) {
      cout << "Failed to get temperature of device: " << i << " " << nvmlErrorString(result) << endl;
    }
    if(temp>GPU_MAX_TEMP){
      cout << "Device Temperature is critical: " << temp << " °C." << "The system will restart after " << SHUTDOWN_SECONDS/1000000<< " seconds." << '\n';
      usleep(SHUTDOWN_SECONDS);
      system("shutdown -P now");
    }
    else
      cout << "Temperature of device: " << i << " " << temp << " °C." << endl;

    //Get GPU fan speed
    result = nvmlDeviceGetFanSpeed(device, &fan_speed);
    if (NVML_SUCCESS != result) {
      cout << "Failed to get fan speed of device: " << i << " " << nvmlErrorString(result) << endl;
    }
    if(fan_speed>FAN_MAX_SPEED){
      cout << "Fan speed of device is critical: " << fan_speed << " RPM." << "The system will restart after " << SHUTDOWN_SECONDS/1000000<< " seconds." << '\n';
      usleep(SHUTDOWN_SECONDS);
      system("shutdown -P now");
    }
    else
      cout << "Fan speed of device: " << i << " " << fan_speed << " RPM." << endl;

    result = nvmlShutdown();
    if (NVML_SUCCESS != result)
      cout << "Failed to shutdown NVML: " << nvmlErrorString(result) << endl;

    Error:
        result = nvmlShutdown();
        if (NVML_SUCCESS != result)
          cout << "Failed to shutdown NVML: " << nvmlErrorString(result) << endl;
  }
}

