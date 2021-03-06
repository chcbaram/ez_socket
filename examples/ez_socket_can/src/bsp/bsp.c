#ifdef _WIN32
# include <windows.h>
#else
#endif

#include "bsp.h"





void delay(uint32_t time_ms)
{
#ifdef _WIN32
  Sleep((DWORD)time_ms);
#else
  if (time_ms < 1000000)
  {
    ::usleep(time_ms*1000);
  }
  else
  {
    uint32_t dwSec = time_ms / 1000;
    uint32_t dwMicro = (time_ms - dwSec*1000) * 1000;
    ::sleep(dwSec);
    ::usleep(dwMicro);
  }
#endif
}

uint32_t millis(void)
{
  double ret;

  LARGE_INTEGER freq, counter;

  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&freq);

  ret = (double)counter.QuadPart / (double)freq.QuadPart * 1000.0;

  return (uint32_t)ret;
}

uint32_t micros(void)
{
  LARGE_INTEGER freq, counter;
  double ret;

  QueryPerformanceCounter(&counter);
  QueryPerformanceFrequency(&freq);

  ret = (double)counter.QuadPart / (double)freq.QuadPart * 1000000.0;

  return (uint32_t)ret;
}
