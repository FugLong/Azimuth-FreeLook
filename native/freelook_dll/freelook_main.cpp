#include "freelook_internal.hpp"

#include "freelook/engine_module.hpp"
#include "freelook/log.hpp"
#include "freelook/ue_discovery.hpp"

#include <Windows.h>

namespace {

volatile LONG g_stop_worker = 0;

DWORD WINAPI worker_main(LPVOID) noexcept {
  freelook::logf("[azimuth_freelook] discovery thread started");

  void* last_world = nullptr;
  bool logged_engine = false;

  while (InterlockedCompareExchange(&g_stop_worker, 0, 0) == 0) {
    HMODULE engine = nullptr;
    wchar_t path[MAX_PATH]{};
    if (!freelook::engine::find_main_engine_module(engine, path, MAX_PATH)) {
      Sleep(250);
      continue;
    }

    if (!logged_engine) {
      char narrow[MAX_PATH * 4]{};
      (void)WideCharToMultiByte(CP_UTF8, 0, path, -1, narrow, static_cast<int>(sizeof(narrow)), nullptr, nullptr);
      freelook::logf("[azimuth_freelook] engine module: %s", narrow);

      char label[160]{};
      if (freelook::ue::sniff_embedded_build_label(engine, label, sizeof(label))) {
        freelook::logf("[azimuth_freelook] embedded build label: %s", label);
      } else {
        freelook::logf("[azimuth_freelook] embedded build label not found");
      }

      logged_engine = true;
    }

    void* world = nullptr;
    const bool ok = freelook::ue::try_resolve_gworld(engine, world);
    if (ok && world != last_world) {
      freelook::logf("[azimuth_freelook] GWorld candidate -> %p", world);
      last_world = world;
    }

    Sleep(500);
  }

  freelook::logf("[azimuth_freelook] discovery thread exiting");
  return 0;
}

}  // namespace

void freelook_request_shutdown() noexcept {
  InterlockedExchange(&g_stop_worker, 1);
}

bool freelook_start_worker_thread() noexcept {
  InterlockedExchange(&g_stop_worker, 0);
  const HANDLE t = CreateThread(nullptr, 0, worker_main, nullptr, 0, nullptr);
  if (!t) {
    return false;
  }
  CloseHandle(t);
  return true;
}
