#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace {

DWORD WINAPI init_thread(LPVOID) {
  OutputDebugStringA("[azimuth_freelook] DLL loaded (scaffold; no UE hooks yet).\n");
  return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(module);
      if (HANDLE t = CreateThread(nullptr, 0, init_thread, nullptr, 0, nullptr)) {
        CloseHandle(t);
      }
      break;
    case DLL_PROCESS_DETACH:
      break;
    default:
      break;
  }
  return TRUE;
}
