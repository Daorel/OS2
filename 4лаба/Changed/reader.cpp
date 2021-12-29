#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <ctime>
#include <random>

using namespace std;

#define LOG_FILE_NAME "reader.log"

int main()
{

    const int numberOfPages = 13;
    const int pageSize = 4096;

    srand(GetCurrentProcessId());
    HANDLE handleMappedFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "MappedFile");
    fstream logger;

    logger.open(LOG_FILE_NAME, fstream::app | fstream::out);

    if (handleMappedFile != INVALID_HANDLE_VALUE)
    {

        void* address = MapViewOfFile(handleMappedFile, FILE_MAP_READ, 0, 0, 0);
        //char* addressToCheck = (char*)MapViewOfFile(handleMappedFile, FILE_MAP_READ, 0, 0, 0);
        //HANDLE writeSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "WriteSemaphore");
        //HANDLE readSemaphore = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, "ReadSemaphore");
        HANDLE loggerMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "loggerMutex");
        HANDLE pageSemW[numberOfPages];
        HANDLE pageSemR[numberOfPages];

        stringstream name;

        for (int i = 0; i < numberOfPages; i++)
        {

            name << "pageSemW" << i;

            //pageMutexW[i] = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name.str().c_str());
            pageSemW[i] = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, name.str().c_str());
        }

        for (int i = 0; i < numberOfPages; i++)
        {

            name << "pageSemR" << i;

            //pageMutexR[i] = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, name.str().c_str());
            pageSemR[i] = OpenSemaphoreA(SYNCHRONIZE | SEMAPHORE_MODIFY_STATE, false, name.str().c_str());
        }

        int proccessId = GetCurrentProcessId();

        //начинаеца
        WaitForSingleObject(loggerMutex, INFINITE);

        //если ты опять все забыла открой райтера там все есть
        logger << GetTickCount() << " | " << proccessId << " reader: ready to read!" << endl;
        logger.flush();

        ReleaseMutex(loggerMutex);
        VirtualLock(address, pageSize * numberOfPages);

        //char data[pageSize];

        for (int i = 0; i < 3; i++)
        {

            WaitForSingleObject(loggerMutex, INFINITE);

            logger<< GetTickCount() << " | " << proccessId << " reader: waiting for read semaphore..." << endl;

            ReleaseMutex(loggerMutex);
            //WaitForSingleObject(readSemaphore, INFINITE);
		//}

            int index = WaitForMultipleObjects(numberOfPages, pageSemR, FALSE, INFINITE);
            int offset = index * pageSize + numberOfPages * sizeof(char);
            //WaitForSingleObject(pageMutex, INFINITE);

            /*if(addressToCheck[index] == 0)
            {
                logger << GetTickCount() << " reader: page collision! page = " << index << "\n";
                logger.flush();
                --i;

                ReleaseMutex(loggerMutex);
                ReleaseMutex(pageMutex[index]);
                ReleaseSemaphore(writeSemaphore, 1, NULL);

                Sleep(10);
            }
            else
            {*/
                //addressToCheck[index] = 0;
                //memcpy(data, addressToCheck[index] + offset, pageSize);
                /*logger << GetTickCount() << " reader: reading completed. page " << index << " : " << data << ". waiting\n";
                RealeaseMutex(pageMutex[index]);
                ReleaseSemaphore(readSemaphore, 1, NULL);*/
                WaitForSingleObject(loggerMutex, INFINITE);
                logger << GetTickCount() << " | " << proccessId << " reader: reading page #" << index << endl;
                logger.flush();
                ReleaseMutex(loggerMutex);
                //ага хитрый пес опять симуляция 
                unsigned int pause = (rand() % 1000) + 500;

                Sleep((DWORD)pause);
                

                
                
                //ReleaseSemaphore(writeSemaphore, 1, NULL);

                WaitForSingleObject(loggerMutex, INFINITE);
                logger << GetTickCount() << " | " << proccessId << " reader: read the page #" << index << ". Release writer's semaphore" << endl;
                logger.flush();
                ReleaseMutex(loggerMutex);

                //ReleaseMutex(pageSemW[index]);
                ReleaseSemaphore(pageSemW[index], 1, NULL);

                Sleep(10);

                /*ReleaseMutex(loggerMutex);
                ReleaseMutex(pageMutex[index]);
                ReleaseSemaphore(writeSemaphore, 1, NULL);*/

            //}


            //logger << GetTickCount() << " | " << proccessId << " reader: reading page #" << index << endl;
            //logger.flush();

            //ReleaseMutex(loggerMutex);

            //ага хитрый пес опять симуляция 
            /*unsigned int pause = (rand() % 1000) + 500;

            Sleep((DWORD)pause);
            WaitForSingleObject(loggerMutex, INFINITE);

            //logger << GetTickCount() << " | " << proccessId << " reader: read the page #" << index << ". Release writer's semaphore" << endl;

            logger.flush();

            ReleaseMutex(loggerMutex);
            ReleaseMutex(pageMutex[index]);
            ReleaseSemaphore(writeSemaphore, 1, NULL);
            Sleep(10);
            */
    
        }

        VirtualUnlock(address, pageSize * numberOfPages);

        for (int i = 0; i < numberOfPages; i++)
        {
            CloseHandle(pageSemR[i]);
            CloseHandle(pageSemW[i]);
        }

        WaitForSingleObject(loggerMutex, INFINITE);

        logger << GetTickCount()
               << " | " << proccessId << " reader exiting" << endl;
        logger.flush();

        ReleaseMutex(loggerMutex);
        UnmapViewOfFile(address);

        CloseHandle(handleMappedFile);
        //CloseHandle(writeSemaphore);
        //CloseHandle(readSemaphore);
    }

    else
    {

        logger << "Cant't open mapped file\n";
    }

    logger.close();

    return 0;
}