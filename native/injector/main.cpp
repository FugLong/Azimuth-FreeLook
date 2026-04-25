#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cwchar>

namespace {

void print_usage() {
  fwprintf(stderr, L"Usage: azimuth_injector.exe <full_path_to_dll> <pid>\n");
}

bool parse_pid(const wchar_t* text, DWORD& out_pid) {
  wchar_t* end = nullptr;
  unsigned long value = wcstoul(text, &end, 10);
  if (end == text || *end != L'\0' || value == 0 || value > 0xFFFFFFFEUL) {
    return false;
  }
  out_pid = static_cast<DWORD>(value);
  return true;
}

bool file_exists(const wchar_t* path) {
  const DWORD attrs = GetFileAttributesW(path);
  return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool inject_load_library(HANDLE process, const wchar_t* dll_path) {
  const SIZE_T path_bytes = (wcslen(dll_path) + 1) * sizeof(wchar_t);

  void* remote_mem = VirtualAllocEx(process, nullptr, path_bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!remote_mem) {
    fwprintf(stderr, L"VirtualAllocEx failed: %lu\n", GetLastError());
    return false;
  }

  if (!WriteProcessMemory(process, remote_mem, dll_path, path_bytes, nullptr)) {
    fwprintf(stderr, L"WriteProcessMemory failed: %lu\n", GetLastError());
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    return false;
  }

  HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
  if (!kernel32) {
    fwprintf(stderr, L"GetModuleHandleW(kernel32) failed: %lu\n", GetLastError());
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    return false;
  }

  auto* load_library_w = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(kernel32, "LoadLibraryW"));
  if (!load_library_w) {
    fwprintf(stderr, L"GetProcAddress(LoadLibraryW) failed: %lu\n", GetLastError());
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    return false;
  }

  HANDLE thread = CreateRemoteThread(process, nullptr, 0, load_library_w, remote_mem, 0, nullptr);
  if (!thread) {
    fwprintf(stderr, L"CreateRemoteThread failed: %lu\n", GetLastError());
    VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);
    return false;
  }

  WaitForSingleObject(thread, INFINITE);
  DWORD exit_code = 0;
  GetExitCodeThread(thread, &exit_code);
  CloseHandle(thread);
  VirtualFreeEx(process, remote_mem, 0, MEM_RELEASE);

  if (exit_code == 0) {
    fwprintf(stderr, L"LoadLibraryW in remote process returned NULL (DLL failed to load).\n");
    return false;
  }

  return true;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
  if (argc != 3) {
    print_usage();
    return 1;
  }

  const wchar_t* dll_path = argv[1];
  DWORD pid = 0;
  if (!parse_pid(argv[2], pid)) {
    fwprintf(stderr, L"Invalid PID.\n");
    return 1;
  }

  if (!file_exists(dll_path)) {
    fwprintf(stderr, L"DLL not found: %ls\n", dll_path);
    return 1;
  }

  HANDLE token = nullptr;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid)) {
      AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr);
    }
    CloseHandle(token);
  }

  HANDLE process = OpenProcess(
      PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
      FALSE,
      pid);
  if (!process) {
    fwprintf(stderr, L"OpenProcess(%lu) failed: %lu\n", pid, GetLastError());
    return 1;
  }

  const bool ok = inject_load_library(process, dll_path);
  CloseHandle(process);
  return ok ? 0 : 1;
}
