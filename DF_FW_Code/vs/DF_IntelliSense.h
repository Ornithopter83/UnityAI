#pragma once

// Editor parsing only: ESP32 GCC retains all attributes in firmware builds.
#if defined(__INTELLISENSE__) && defined(_MSC_VER)
#ifndef __attribute__
#define __attribute__(DF_Attributes)
#endif
#endif
