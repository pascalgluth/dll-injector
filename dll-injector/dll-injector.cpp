#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <libloaderapi.h>

using namespace std;

// Get the target process ID
void get_t_proc_id(const char* window_title, DWORD& process_id) {
    GetWindowThreadProcessId(FindWindow(NULL, window_title), &process_id);
}

// Display error messages
void error(const char* title, const char* message) {
    MessageBox(0, message, title, 0);
    exit(-1);
}

// Check if given dll to inject exists
bool dll_exists(string filename) {
    struct stat buffer;
    // stat() gets information about file and saves it into buffer reference
    return (stat(filename.c_str(), &buffer) == 0);
}


int main()
{
    // EXAMPLE WITH MS PAINT

    DWORD proc_id = NULL;
    char dll_path[MAX_PATH];
    // Name of the dll to be injected
    const char* dll_name;
    // Name of the window title
    const char* window_title;

    // Get from user

    std::string tmp;
    cout << "Enter the name of the dll: " << endl;
    getline(cin, tmp);
    dll_name = tmp.c_str();

    
    string tmp2;
    cout << "Enter the title of the target process window: " << endl;
    getline(cin, tmp2);
    window_title = tmp2.c_str();

    if (dll_name == "" || dll_name == NULL) error("Error", "Please enter a valid dll name (e.g. mydll.dll)");
    if (window_title == "" || window_title == NULL) error("Error", "Please enter a valid window title");

    // Check if the file exists
    if (!dll_exists(dll_name)) {
        error("Error", "File does not exist");
    }
    if (!GetFullPathName(dll_name, MAX_PATH, dll_path, nullptr)) {
        error("Error", "Could not get full path");
    }

    // Get the process if of the target app
    get_t_proc_id(window_title, proc_id);
    if (proc_id == NULL) {
        error("Error", "Could not find process id");
    }

    // Open a handle to the process target
    HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, NULL, proc_id);
    if (!h_process) {
        error("Error", "Could not open a handle to the process");
    }

    // Allocate memory at target process
    void* allocated_mem = VirtualAllocEx(h_process, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!allocated_mem) {
        error("Error", "Could not allocate memory");
    }

    // Write dll into memory at target process
    if (!WriteProcessMemory(h_process, allocated_mem, dll_path, MAX_PATH, nullptr)) {
        error("Error", "Could not write process memory");
    }

    // Create the thread
    HANDLE h_thread = CreateRemoteThread(h_process, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated_mem, NULL, nullptr);
    if (!h_thread) {
        error("Error", "Could not create remote thread");
    }

    // Close the handle and free the memory
    CloseHandle(h_process);
    VirtualFreeEx(h_process, allocated_mem, NULL, MEM_RELEASE);

    MessageBox(0, "Sucessfully injected dll", "Success", 0);


}
