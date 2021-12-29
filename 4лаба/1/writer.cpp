#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <random>
#include <ctime>
#define LOG_FILE_NAME "writer.log"

using namespace std;

int main()
{
    const int numberOfPages = 13;
    const int pageSize = 4096;
    srand(GetCurrentProcessId()); //
    //открываем файлик
    HANDLE handleMappedFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "MappedFile");

    //http://blog.kislenko.net/show.php?id=1402

    fstream logger;

    logger.open(LOG_FILE_NAME, fstream::app | fstream::out);

    if (handleMappedFile != INVALID_HANDLE_VALUE)
    {

        //http://www.vsokovikov.narod.ru/New_MSDN_API/Menage_files/fn_mapviewoffile.htm
        //полуаем адрес
        void* address = MapViewOfFile(handleMappedFile, FILE_MAP_READ, 0, 0, 0);

        //открываем семафорчики и мьютексы    

        HANDLE writeSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "WriteSemaphore");
        HANDLE readSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "ReadSemaphore");
        HANDLE loggerMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "loggerMutex");
        HANDLE pageMutex[numberOfPages];

        stringstream name;

        for (int i = 0; i < numberOfPages; i++)
        {

            name << "pageMutex" << i;
            pageMutex[i] = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name.str().c_str());
        }


        //процесс какой у нас вообще
        int proccessId = GetCurrentProcessId();

        //начинаеца
        WaitForSingleObject(loggerMutex, INFINITE);

        //http://www.vsokovikov.narod.ru/New_MSDN_API/Time/fn_gettickcount.htm

        logger << GetTickCount() << " | " << proccessId << " writer: ready to write!" << endl;
        logger.flush();

        //https://docs.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtuallock


        ReleaseMutex(loggerMutex); //освободили ага да
        VirtualLock(address, pageSize * numberOfPages); //предотвращаем запись памяти на диск

        for (int i = 0; i < 3; i++)
        {

            WaitForSingleObject(loggerMutex, INFINITE);

            logger << GetTickCount() << " | " << proccessId << " writer: waiting for writer's semaphore" << endl;
            logger.flush();

            ReleaseMutex(loggerMutex);
            WaitForSingleObject(writeSemaphore, INFINITE);

            //https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
            /*
            DWORD WaitForMultipleObjects(
            DWORD nCount,               // число объектов в массиве lpHandles
            CONST HANDLE *lpHandles,    // указатель на массив описателей объектов ядра
            BOOL bWaitAll,              // флаг, означающей надо ли дожидаться всех объектов или достаточно одного
            DWORD dwMilliseconds        // таймаут
            );
            */

            int index = WaitForMultipleObjects(numberOfPages, pageMutex, FALSE, INFINITE);

            WaitForSingleObject(loggerMutex, INFINITE);

            logger << GetTickCount() << " | " << proccessId << " writer: writing page number to page #" << index << endl;
            logger.flush();

            ReleaseMutex(loggerMutex);

            unsigned int pause = (rand() % 1000) + 500; //симуляция записи информации

            Sleep((DWORD)pause); //точнее вот симуляция
            WaitForSingleObject(loggerMutex, INFINITE);

            logger << GetTickCount() << " | " << proccessId << " writer: release reader's semaphore #" << index << endl;
            logger.flush();

            //все освободили
            ReleaseMutex(loggerMutex);
            ReleaseMutex(pageMutex[index]);
            ReleaseSemaphore(readSemaphore, 1, NULL);

            Sleep(10); //еще поспи
        }

        //теперь можно и разблочить запись памяти на диск, хотя он ничего не записал
        VirtualUnlock(address, pageSize * numberOfPages);

        for (int i = 0; i < numberOfPages; i++)
        {
            CloseHandle(pageMutex[i]);
        }

        WaitForSingleObject(loggerMutex, INFINITE);

        logger <<  GetTickCount()
               << " | " << proccessId << " writer exiting" << endl;
        logger.flush();

        ReleaseMutex(loggerMutex);
        CloseHandle(loggerMutex);
        UnmapViewOfFile(address); //все проецируемый уйди

        CloseHandle(handleMappedFile);
        CloseHandle(writeSemaphore);
        CloseHandle(readSemaphore);
    }
    else
    {
        logger << "Cant't open mapped file\n";
    }
    logger.close();

    return 0;
}