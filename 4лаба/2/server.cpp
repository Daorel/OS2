#include <iostream>
#include <Windows.h>

using namespace std;

const char PIPE_NAME[] = "\\\\.\\pipe\\pipename"; //назвение канала
const int PIPE_SIZE = 2048; //рамер буфера

int main()
{
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

    size_t i;
    HANDLE handlePipe = CreateNamedPipeA(PIPE_NAME, PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED | WRITE_DAC, PIPE_TYPE_MESSAGE | PIPE_WAIT, 1, 0, 0, 0, NULL);

    if(handlePipe != INVALID_HANDLE_VALUE)
    {
        cout << "* Pipe created. " << endl;

        //https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-connectnamedpipe

        WINBOOL connection = ConnectNamedPipe(handlePipe, NULL);
        if(connection)
        {
            //https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createeventa
            /*
            HANDLE CreateEvent
            (
                LPSECURITY_ATTRIBUTES lpEventAttributes,	// атрибут защиты
                BOOL bManualReset,				// тип сброса TRUE - ручной
                BOOL bInitialState,			// начальное состояние TRUE - сигнальное
                LPCTSTR lpName				// имя обьекта
            );
            */

           /*
           http://www.vsokovikov.narod.ru/New_MSDN_API/Synchronization/str_overlapped.htm
           */

            cout << "* Connected to the pipe. " << endl;
            OVERLAPPED overlapped;
            overlapped.hEvent = CreateEvent(NULL, true, false, NULL);
            overlapped.Offset = 0;
            overlapped.OffsetHigh = 0;

            char buff[PIPE_SIZE];
            string strTemp;
            while(strcmp(buff, "STOP") != 0) // сравнение строк
            {
                //http://rusproject.narod.ru/winapi/z/zeromemory.html
                ZeroMemory(buff, 0);
                cout << "Enter message \"STOP\" to exit: \n> ";
                getline(cin, strTemp);
                if(strTemp.length()-1 > PIPE_SIZE)
                {
                    cout << "Message more than " << PIPE_SIZE << " bytes. Enter again, please... " << endl;
                    continue;
                }
                else
                {
                    for(i = 0; i < strTemp.length(); ++i)
                        buff[i] = strTemp[i];
                    buff[i] = '\0';
                }

                //http://www.vsokovikov.narod.ru/New_MSDN_API/Menage_files/fn_writefile.htm 
                /*
                BOOL WriteFile(
                HANDLE hFile,                    // дескриптор файла
                LPCVOID lpBuffer,                // буфер данных
                DWORD nNumberOfBytesToWrite,     // число байтов для записи
                LPDWORD lpNumberOfBytesWritten,  // число записанных байтов
                LPOVERLAPPED lpOverlapped        // асинхронный буфер
                );
                */

                WriteFile(handlePipe, buff, strlen(buff) + 1, NULL, &overlapped);
                WaitForSingleObject(overlapped.hEvent, INFINITE);
                cout << "Writted! " << endl;
            }

            //https://docs.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-disconnectnamedpipe
            DisconnectNamedPipe(handlePipe);
            CloseHandle(overlapped.hEvent);
        }
        else
        {
            cout << "Ooops... Something wrong with connect to pipe... Error code is " << GetLastError() << ". " << endl;
        }

        CloseHandle(handlePipe);
    }
    else
    {
        cout << "Ooops... Something wrong with creation to pipe... Error code is " << GetLastError() << ". " << endl;
    }

    return 0;
}