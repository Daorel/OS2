#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <mutex>

using namespace std;

#define BLOCKSIZE 9308200 //размер блока, номер студ билета 930820 
#define N 100000000 //количество знаков осле запятой

//тут прототипы функций
DWORD WINAPI NumberPi(LPVOID); //функция подсчета пи

mutex mtx; //мьютекс
double finalPi; //глобальная переменная чтобы работала и в мейне и в функции

//тут мейн
int main()
{
    for(int numOfThreads = 1, offset = 1; numOfThreads <= 32; numOfThreads = 1 << (offset++)) //смещение чтобы получать значения как степени двойки
    {
        double totalTime = 0.0;
        HANDLE* handleThread = new HANDLE[numOfThreads]; //создаем массив указателей на хендлы numOfThreads потоков
        if (handleThread != NULL)
        {
            int sumOfBlocks = 0;
            finalPi = 0;
            for(int i = 0; i < numOfThreads; ++i)
            {
                /*
                HANDLE CreateThread(
                LPSECURITY_ATTRIBUTES lpThreadAttributes, // дескриптор защиты
                SIZE_T dwStackSize,                       // начальный размер стека
                LPTHREAD_START_ROUTINE lpStartAddress,    // функция потока (указатель на функцию - имя функции без параметра)
                LPVOID lpParameter,                       // параметр потока (параметр который подаем в функцию выше)
                DWORD dwCreationFlags,                    // опции создания
                LPDWORD lpThreadId                        // идентификатор потока)
                */
                handleThread[i] = CreateThread(NULL, 0, NumberPi, &sumOfBlocks, CREATE_SUSPENDED, NULL);
            }

            DWORD startTime = GetTickCount(); //начинаем отсчитывать время тут, в миллисекундах

            for(int i = 0; i < numOfThreads; ++i)
            {
                ResumeThread(handleThread[i]);
            }        
            /*
            DWORD WaitForMultipleObjects(
            [in] DWORD        nCount,  //количество хендлов потоков (в нашем случае)
            [in] const HANDLE *lpHandles, (указател на начало массива хендлов потоков)
            [in] BOOL         bWaitAll, //true, чтобы дождаться всех потоков
            [in] DWORD        dwMilliseconds)  //INFINITE, так как не ждем определенного времени, а просто ждем сколько угодно, пока все потоки не завершатся
            */
            WaitForMultipleObjects(numOfThreads, handleThread, true, INFINITE);

            DWORD finalTime = GetTickCount();
            totalTime = ((double)finalTime - (double)startTime)/1000; //сразу перевод в секунды

            //это в самый КОНЕЦ
            for(int i = 0; i < numOfThreads; ++i)
            {
                CloseHandle(handleThread[i]);
            }
                    
            delete handleThread; //удалить указатель на первый эелемент массива закрытых хендлов
        }
        else
        {
            cout << "\nOoops... Something wrong with handles of threads..." << "\n";
        }

        printf("\nBy %i threads we get this value of Pi: %.10f\n", numOfThreads, finalPi);
        printf("It took %.3f sec\n", totalTime);
        //printf("\nPi is = %.10f\n", finalPi);
    }
    
    return 0;
}

DWORD WINAPI NumberPi(LPVOID lpParameter)
{
    int* sumOfBlocks = (int*)lpParameter; //размер блока const, но мы не считаем, сколько блоков мы сделали, так что сюда суммируем все наши блоки, чтобы не вылезти в итоге за N из-за количества потоков
    double pi = 0.0;
    int stopBlock = 0; //переменная для контроля того, что мы работаем по блокам
    while (stopBlock < N)
    {
        mtx.lock();             //данный блок должен выполнятся в каждом потоке по очереди так, чтобы было понятно, когда освободится последнему потоку, иначе у всех эти значения получатся одинаковыми
        *sumOfBlocks += BLOCKSIZE;
        stopBlock = *sumOfBlocks;
        mtx.unlock();
        for(int i = stopBlock-BLOCKSIZE; (i < N) && (i < stopBlock); ++i) //обеспеиваем конец подсчетов либо при конце очередного блока, либо при значении Н, без скобочек не работает
        {
            pi += 4.0 / (1.0 + ((i + 0.5) / N) * ((i + 0.5) / N)); // это у нас для х0, ((0 + 0.5) / N) на 1 итерации, н - константа, все приссумируем к пи
        }
    }
    finalPi += pi / N; //делим после цикла один раз на нашу большую Н
    return 0;
}

//https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread
//http://www.vsokovikov.narod.ru/New_MSDN_API/Process_thread/fn_createthread.htm

//https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-resumethread
//http://www.vsokovikov.narod.ru/New_MSDN_API/Process_thread/fn_resumethread.htm

//https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
