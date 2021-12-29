#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

using namespace std;

int main()
{
    const int numberOfPages = 13;
    const int numberOfProcess = 14;
    const int pageSize = 4096;

    //https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createnamedpipea
    /**
     * HANDLE CreateNamedPipeA(
    [in]           LPCSTR                lpName,
    [in]           DWORD                 dwOpenMode,
    [in]           DWORD                 dwPipeMode,
    [in]           DWORD                 nMaxInstances,
    [in]           DWORD                 nOutBufferSize,
    [in]           DWORD                 nInBufferSize,
    [in]           DWORD                 nDefaultTimeOut,
    [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
     */

    HANDLE handleFile = CreateFileA("fileMapping", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    //https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-setfilepointer
    /*
    DWORD SetFilePointer(
    HANDLE hFile,                // дескриптор файла
    LONG lDistanceToMove,        // байты перемещения указателя
    PLONG lpDistanceToMoveHigh,  // байты перемещения указателя
    DWORD dwMoveMethod           // точка отсчета
    );
    */

    SetFilePointer(handleFile, pageSize * numberOfPages, 0, FILE_BEGIN); //в начало файла
    SetEndOfFile(handleFile);  //установка конца файла

    if (handleFile != INVALID_HANDLE_VALUE)
    {

        //создаем проецируемый файлик
        HANDLE handleFileMapping = CreateFileMappingA(handleFile, NULL, PAGE_READWRITE, 0, 0, "MappedFile");

        if (handleFileMapping != INVALID_HANDLE_VALUE)
        {
            //https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createsemaphorea
            /*
            HANDLE CreateSemaphore(
            LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, //Атрибуты защиты
            LONG lInitialCount, //Количество свободных при инициализации ресурсов
            LONG lMaximumCount, //Всего ресурсов
            LPCTSTR lpName //Имя
            ); 
            */

            //https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createmutexa

            //HANDLE writeSemaphore = CreateSemaphoreA(NULL, numberOfPages, numberOfPages, "WriteSemaphore");
            //HANDLE readSemaphore = CreateSemaphoreA(NULL, 0, numberOfPages, "ReadSemaphore");
            HANDLE loggerMutex = CreateMutexA(NULL, FALSE, "loggerMutex");
            HANDLE pageSemW[numberOfPages];
            HANDLE pageSemR[numberOfPages];
            stringstream name; //ввод-вывод с "консоли", файла и т.д.

            for (int i = 0; i < numberOfPages; i++)
            {
                name << "pageSemW" << i;
                //pageSemW[i] = CreateMutexA(NULL, FALSE, name.str().c_str());
                pageSemW[i] = CreateSemaphoreA(NULL, 1, 1, name.str().c_str());
            }

            for (int i = 0; i < numberOfPages; i++)
            {
                name << "pageSemR" << i;
                //pageSemR[i] = CreateMutexA(NULL, TRUE, name.str().c_str());
                pageSemR[i] = CreateSemaphoreA(NULL, 0, 1, name.str().c_str());
            }

            HANDLE handleChildProcess[2 * numberOfProcess];
            HANDLE handleThreadChild[2 * numberOfProcess];

            for (int i = 0; i < numberOfProcess; i++)
            {
                //http://demiurgjr.narod.ru/Documents/WinAPI/STARTUPINFO.htm
                //http://www.vsokovikov.narod.ru/New_MSDN_API/Process_thread/str_process_information.htm

                STARTUPINFOA startupInfoA;
                ZeroMemory(&startupInfoA, sizeof(STARTUPINFOA));
                startupInfoA.cb = sizeof(STARTUPINFOA); //размер структуры
                PROCESS_INFORMATION processInf;

                //http://www.vsokovikov.narod.ru/New_MSDN_API/Process_thread/fn_createprocess.htm

                /*
                BOOL CreateProcess(
                LPCTSTR lpApplicationName ,                 // имя исполняемого модуля
                LPTSTR lpCommandLine,                       // командная строка
                LPSECURITY_ATTRIBUTES lpProcessAttributes , // SD (дескриптор безопасности)
                LPSECURITY_ATTRIBUTES lpThreadAttributes,   // SD
                BOOL bInheritHandles,                       // дескриптор параметра наследования
                DWORD dwCreationFlags,                      // флажки создания
                LPVOID lpEnvironment,                       // новый блок конфигурации
                LPCTSTR lpCurrentDirectory,                 // имя текущего каталога
                LPSTARTUPINFO lpStartupInfo,                // информация предустановки
                LPPROCESS_INFORMATION lpProcessInformation  // информация о процессе
                ); 
                */
    
                if (CreateProcessA("writer.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfoA, &processInf))

                    cout << "Writer" << i << " started\n";

                handleChildProcess[i] = processInf.hProcess;
                handleThreadChild[i] = processInf.hThread;
            }

            for (int i = 0; i < numberOfProcess; i++)
            {
                STARTUPINFOA startupInfoA;
                ZeroMemory(&startupInfoA, sizeof(STARTUPINFOA));
                startupInfoA.cb = sizeof(STARTUPINFOA); //размер структуры
                PROCESS_INFORMATION processInf;

                if (CreateProcessA("reader.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfoA, &processInf))

                    cout << "Reader" << i << " started\n";

                handleChildProcess[i + numberOfProcess] = processInf.hProcess;
                handleThreadChild[i + numberOfProcess] = processInf.hThread;
            }

            //https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
            /*
            DWORD WaitForMultipleObjects(
            DWORD nCount,               // число объектов в массиве lpHandles
            CONST HANDLE *lpHandles,    // указатель на массив описателей объектов ядра
            BOOL bWaitAll,              // флаг, означающей надо ли дожидаться всех объектов или достаточно одного
            DWORD dwMilliseconds        // таймаут
            );
            */

            WaitForMultipleObjects(2 * numberOfProcess, handleChildProcess, TRUE, INFINITE); //все объекты
            cout << "All process are done\n";
            cin.get();

            for (int i = numberOfPages; i < 2 * numberOfPages; i++)
            {
                CloseHandle(handleThreadChild[i]);
                CloseHandle(handleChildProcess[i]);
            }

            for (int i = 0; i < numberOfPages; i++)
            {
                CloseHandle(pageSemW[i]);
                CloseHandle(pageSemR[i]);
            }

            //CloseHandle(writeSemaphore);
            //CloseHandle(readSemaphore);
            CloseHandle(handleFileMapping);
        }

        else
        {
            cout << "Can't create file mapping object\n";
            cin.get();
        }
        CloseHandle(handleFile);
    }
    else
    {
        cout << "Can't create file for mapping\n";
        cin.get();
    }

    return 0;
}