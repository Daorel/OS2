#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <omp.h>

using namespace std;

#define BLOCKSIZE 9308200 //размер блока, номер студ билета 930820 
#define N 100000000 //количество знаков осле запятой

//https://docs.microsoft.com/ru-ru/cpp/parallel/openmp/reference/openmp-directives?view=msvc-160

int main()
{
    double totalTime;
    for(int numOfThreads = 1, offset = 1; numOfThreads <= 16; numOfThreads = 1 << (offset++))
        {
            double Pi = 0.0;
            omp_set_num_threads(numOfThreads);
            DWORD startTime = GetTickCount();
            //https://docs.microsoft.com/ru-ru/cpp/parallel/openmp/reference/openmp-clauses?view=msvc-160#schedule
            //https://docs.microsoft.com/en-us/cpp/parallel/openmp/reference/openmp-clauses?view=msvc-160#reduction
            #pragma omp parallel for schedule(dynamic, BLOCKSIZE) reduction(+:Pi) // фор - делится между потоками, расписание динамическое, размер итераций - блоксайз,
            //reduction(+:Pi) после цикла результаты отдельных циклов сложатся в Пи
            for(int i = 0; i < N; ++i)
            {
                Pi += 4.0 / (1.0 + ((i + 0.5) / N) * ((i + 0.5) / N));
            }

            Pi = Pi / N; //не забыть поделить на Н

            DWORD finalTime = GetTickCount();
            totalTime = ((double)finalTime - (double)startTime)/1000;

            printf("\nBy %i threads we get this value of Pi: %.10f\n", numOfThreads, Pi);
            printf("It took %.3f sec\n", totalTime);

        }
    return 0;
}