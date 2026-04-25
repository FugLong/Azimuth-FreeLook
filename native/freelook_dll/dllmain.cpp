#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "freelook_internal.hpp"

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(module);
      if (!freelook_start_worker_thread()) {
        return FALSE;
      }
      break;
    case DLL_PROCESS_DETACH:
      freelook_request_shutdown();
      break;
    default:
      break;
  }
  return TRUE;
}
