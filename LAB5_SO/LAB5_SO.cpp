#include <windows.h>
#include <iostream>
#include <thread>
#include <random>
const int MEMORY_SIZE = sizeof(int);
const char* MEMORY_NAME = "Local\\SharedMemoryExample";
const char* SEMAPHORE_NAME = "Local\\SemaphoreExample";
using namespace std;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> coin(1, 2);
void simulateProcess(int processId) {
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, MEMORY_NAME);
    if (hMapFile == NULL) {
        cerr << "Nu s-a putut deschide fișierul mapat: " << GetLastError() << "\n";
        return;
    }
    int* sharedMemory = (int*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, MEMORY_SIZE);
    if (sharedMemory == NULL) {
        cerr << "Nu s-a putut mapa vizualizarea: " << GetLastError() << "\n";
        CloseHandle(hMapFile);
        return;
    }
    HANDLE hSemaphore = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_NAME);
    if (hSemaphore == NULL) {
        cerr << "Nu s-a putut deschide semaforul: " << GetLastError() << "\n";
        UnmapViewOfFile(sharedMemory);
        CloseHandle(hMapFile);
        return;
    }
    int counter = 1;
    while (counter <= 1000) {
        WaitForSingleObject(hSemaphore, INFINITE);
        cout << "Proces " << processId << " a citit: " << *sharedMemory << "\n";
        if (coin(gen) == 2) {
            *sharedMemory = counter++;
            cout << "Proces " << processId << " a scris: " << *sharedMemory << "\n";
            //this_thread::sleep_for(chrono::milliseconds(10));
        }
        ReleaseSemaphore(hSemaphore, 1, NULL);
    }
    UnmapViewOfFile(sharedMemory);
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);
}
int main() {
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MEMORY_SIZE, MEMORY_NAME);
    if (hMapFile == NULL) {
        cerr << "Nu s-a putut crea fișierul mapat: " << GetLastError() << "\n";
        return 1;
    }
    HANDLE hSemaphore = CreateSemaphore(NULL, 1, 1, SEMAPHORE_NAME);
    if (hSemaphore == NULL) {
        cerr << "Nu s-a putut crea semaforul: " << GetLastError() << "\n";
        CloseHandle(hMapFile);
        return 1;
    }
    thread process1(simulateProcess, 1);
    thread process2(simulateProcess, 2);
    process1.join();
    process2.join();
    CloseHandle(hMapFile);
    CloseHandle(hSemaphore);
    return 0;
}
