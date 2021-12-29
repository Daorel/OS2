#include <iostream>
#include <Windows.h>

using namespace std;

void CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped);

const char PIPE_NAME[] = "\\\\.\\pipe\\pipename"; //назвение канала
const int PIPE_SIZE = 2048; //рамер буфера

int main()
{

    //https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-waitnamedpipea

    WaitNamedPipeA(PIPE_NAME, NMPWAIT_WAIT_FOREVER);

    HANDLE handlePipe = CreateFileA(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);

    if(handlePipe != INVALID_HANDLE_VALUE)
    {
        cout << "Connected to the pipe. " << endl;

        OVERLAPPED overlapped;
        overlapped.Offset = 0;
        overlapped.OffsetHigh = 0;

        char buff[PIPE_SIZE]; buff[0] = '\0';
        while(strcmp(buff, "STOP") != 0) //сравнение строк
        {
            //http://rusproject.narod.ru/winapi/z/zeromemory.html
            
            ZeroMemory(buff, PIPE_SIZE);

            //http://www.vsokovikov.narod.ru/New_MSDN_API/Menage_files/fn_readfileex.htm
            /*
            BOOL ReadFileEx(
            HANDLE hFile,                        // дескриптор файла
            LPVOID lpBuffer,                     // буфер данных
            DWORD nNumberOfBytesToRead,          // число читаемых байтов
            LPOVERLAPPED lpOverlapped,           // смещение
            LPOVERLAPPED_COMPLETION_ROUTINE 
                    lpCompletionRoutine            // процедура завершения
            );
            */

            ReadFileEx(handlePipe, buff, PIPE_SIZE, &overlapped, FileIOCompletionRoutine);
            SleepEx(-1, TRUE);

            cout << "Server says: \"" << buff << "\". " <<  endl;
        }
        CloseHandle(handlePipe);
    }
    else
    {
        cout << "Ooops... Something wrong with connect to pipe... Error code is " << GetLastError() << ". " << endl;
    }

    return 0;
}

void CALLBACK FileIOCompletionRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped)
{
    
}