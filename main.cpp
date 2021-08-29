#include <chrono>
#include <time.h>
#include <thread>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <filesystem>

#include "reader.h"
#include "directory_watcher.h"
#include "parser.h"
#include "agregator.h"
#include "sender.h"
#include "settings.h"
#include "logger.h"
#include "program_options.h"

#include <fcntl.h>

using time_point = std::chrono::time_point<std::chrono::system_clock>;
using LoggerType = YellowWatcher::Logger::Type;

static auto LOGGER = YellowWatcher::Logger::getInstance();

SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);

static const std::string VERSION = "1.2";

static std::filesystem::path PROGRAM_PATH;
static std::filesystem::path FILE_PATH;
static std::filesystem::path SETTINGS_PATH;

wchar_t SERVICE_NAME[100] = L"Yellow Watcher Service"; 

void RunConsole();
int InstallService(LPCWSTR serviceName, LPCWSTR servicePath);
int RemoveService(LPCWSTR serviceName);

bool NeedSend(const time_point& send, const time_point& cur){
    std::int64_t agregate_period = std::chrono::duration_cast<std::chrono::milliseconds>(cur - send).count();
    time_t cur_time = std::chrono::system_clock::to_time_t(cur);
    struct tm * timeinfo;
    timeinfo = localtime (&cur_time);
    if((timeinfo->tm_sec > 50 || agregate_period > 60000) && agregate_period > 15000){
        return true;
    }
    else{
        return false;
    }
}

void SendCounter(
    const std::shared_ptr<YellowWatcher::Sender> sender,
    const std::shared_ptr<YellowWatcher::Agregator> agregator,
    const std::shared_ptr<YellowWatcher::Settings> settings
    )
{
    sender->Send(settings->HttpTarget(), agregator->ValueAsString(settings->Host()), settings->HttpLogin(), settings->HttpPassword());
}

void Init(
    std::shared_ptr<YellowWatcher::Settings>& settings,
    std::shared_ptr<YellowWatcher::DirectoryWatcher>& dw,
    std::shared_ptr<YellowWatcher::Agregator>& agregator,
    std::shared_ptr<YellowWatcher::Sender>& sender
    )
{
    settings = std::make_shared<YellowWatcher::Settings>();
    settings->Read(PROGRAM_PATH);
    LOGGER->Print(std::string("PathSettings=").append(settings->PathSettings()));
    LOGGER->Print(std::string("host=").append(settings->Host()));

    dw = std::make_shared<YellowWatcher::DirectoryWatcher>(settings->PathMonitoring());
    agregator = std::make_shared<YellowWatcher::Agregator>();
    sender = std::make_shared<YellowWatcher::Sender>(settings->HttpHost(), settings->HttpPort());
}

void RunStep(
    std::shared_ptr<YellowWatcher::DirectoryWatcher> dw,
    std::shared_ptr<YellowWatcher::Agregator> agregator,
    std::shared_ptr<YellowWatcher::Sender> sender,
    std::shared_ptr<YellowWatcher::Settings> settings,
    time_point& send
    )
{
    dw->ExecuteStep();
    const std::vector<YellowWatcher::EventData>& events = dw->GetEvents();
    agregator->Add(events);
    dw->ClearEvents();
       
    time_point cur = std::chrono::system_clock::now();
    if(NeedSend(send, cur)){
        if(LOGGER->LogType() == LoggerType::Trace){
            LOGGER->Print("SEND", LoggerType::Trace);
        }
        SendCounter(sender, agregator, settings);
        send = cur;
        agregator->Clear();
    }
}

void SetLoggerLevel(YellowWatcher::Logger* logger, const std::wstring& level){
    if(level == L"trace") logger->SetLogType(LoggerType::Trace);
    else if(level == L"info") logger->SetLogType(LoggerType::Info);
    else if(level == L"error") logger->SetLogType(LoggerType::Error);
}

void SetPath(){
    WCHAR path[500];
    DWORD size = GetModuleFileNameW(NULL, path, 500);

    FILE_PATH = std::filesystem::path(path); 
    PROGRAM_PATH = FILE_PATH.parent_path();

    SETTINGS_PATH = PROGRAM_PATH;
    SETTINGS_PATH.append("settings.txt");
}

int wmain (int argc, wchar_t  ** argv)
{
    

    YellowWatcher::ProgrammOptions program_options(argc, argv);
    if(!program_options.Help().empty()){
        std::cout << program_options.Help();
        return 0;
    }

    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin),  _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);

    SetPath();

    LOGGER->Open(PROGRAM_PATH);
    
    SetLoggerLevel(LOGGER, program_options.LogLevel());
    if(program_options.Mode() == L"console"){
        LOGGER->SetOutConsole(true);
        LOGGER->Print("Yellow Watcher: run console mode", true);
        LOGGER->Print(std::string("Version: ").append(VERSION), true);
        RunConsole();
        return 0;
    }
    else if(program_options.Mode() == L"install"){
        LOGGER->SetOutConsole(true);
        int result = InstallService(SERVICE_NAME, std::wstring(L"\"").append(FILE_PATH.wstring()).append(L"\"").c_str());
        if(result == 0){
            LOGGER->Print(std::wstring(L"Binary path: \"").append(FILE_PATH.wstring().append(L"\"")), LoggerType::Info, true);
            YellowWatcher::Settings settings;
            settings.CreateSettings(SETTINGS_PATH);
        }
        return result;
    }
    else if(program_options.Mode() == L"uninstall"){
        LOGGER->SetOutConsole(true);
        return RemoveService(SERVICE_NAME);
    }
    else
    {
        LOGGER->Print("Yellow Watcher: run service mode", true);
        LOGGER->Print(std::string("Version: ").append(VERSION), true);
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       auto error = GetLastError();
       if(error == 1063){
           LOGGER->SetOutConsole(true);
           LOGGER->Print("Set startup mode = console. For more details, see the help.", LoggerType::Error);
           return 0;
       }
       else{
           LOGGER->Print(std::string("Yellow Watcher: Main: StartServiceCtrlDispatcher returned error ").append(std::to_string(error)), LoggerType::Error);
       }
       return error;
    }

    LOGGER->Print("Yellow Watcher: stop service", true);

    return 0;
}

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    LOGGER->Print("Yellow Watcher: ServiceMain: Entry");
    
    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) 
    {
        LOGGER->Print("Yellow Watcher: ServiceMain: RegisterServiceCtrlHandler returned error", LoggerType::Error);
        LOGGER->Print("Yellow Watcher: ServiceMain: Exit", LoggerType::Info, true);
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        //"My Sample Service: ServiceMain: SetServiceStatus returned error";
    }

    /* 
     * Perform tasks neccesary to start the service here
     */
    //"My Sample Service: ServiceMain: Performing Service Start Operations";

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) 
    {
        //"My Sample Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error";

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
	    {
		    //"My Sample Service: ServiceMain: SetServiceStatus returned error";
	    }
        //"My Sample Service: ServiceMain: Exit";
        return; 
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    //"My Sample Service: ServiceMain: SetServiceStatus returned error";
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    //"My Sample Service: ServiceMain: Waiting for Worker Thread to complete";

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);
    
    //"My Sample Service: ServiceMain: Worker Thread Stop Event signaled";
    
    
    /* 
     * Perform any cleanup tasks
     */
    //"My Sample Service: ServiceMain: Performing Cleanup Operations";

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
	    //"My Sample Service: ServiceMain: SetServiceStatus returned error";
    }

    //"My Sample Service: ServiceMain: Exit";

    return;
}

VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    //"My Sample Service: ServiceCtrlHandler: Entry";

    switch (CtrlCode) 
	{
     case SERVICE_CONTROL_STOP :

        //"My Sample Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request";

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        /* 
         * Perform tasks neccesary to stop the service here 
         */
        
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			//"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error";
		}

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }

    //"My Sample Service: ServiceCtrlHandler: Exit";
}

void RunConsole(){
    std::shared_ptr<YellowWatcher::Settings> settings = nullptr;
    std::shared_ptr<YellowWatcher::DirectoryWatcher> dw = nullptr;
    std::shared_ptr<YellowWatcher::Agregator> agregator = nullptr; 
    std::shared_ptr<YellowWatcher::Sender> sender = nullptr;
    Init(settings, dw, agregator, sender);
    time_point send = std::chrono::system_clock::now();

    for(;;)
    {
        RunStep(dw, agregator, sender, settings, send);        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int InstallService(LPCWSTR serviceName, LPCWSTR servicePath){
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if(!hSCManager) {
        LOGGER->Print("Can't open Service Control Manager.", LoggerType::Error);
        return -1;
    }

    SC_HANDLE hService = CreateService(
        hSCManager,
        serviceName,
        serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        servicePath,
        NULL, NULL, NULL, NULL, NULL
    );

    if(!hService) {
        int err = GetLastError();
        switch(err) {
        case ERROR_ACCESS_DENIED:
            LOGGER->Print("ERROR_ACCESS_DENIED", LoggerType::Error);
            break;
        case ERROR_CIRCULAR_DEPENDENCY:
            LOGGER->Print("ERROR_CIRCULAR_DEPENDENCY", LoggerType::Error);
            break;
        case ERROR_DUPLICATE_SERVICE_NAME:
            LOGGER->Print("ERROR_DUPLICATE_SERVICE_NAME", LoggerType::Error);
            break;
        case ERROR_INVALID_HANDLE:
            LOGGER->Print("ERROR_INVALID_HANDLE", LoggerType::Error);
            break;
        case ERROR_INVALID_NAME:
            LOGGER->Print("ERROR_INVALID_NAME", LoggerType::Error);
            break;
        case ERROR_INVALID_PARAMETER:
            LOGGER->Print("ERROR_INVALID_PARAMETER", LoggerType::Error);
            break;
        case ERROR_INVALID_SERVICE_ACCOUNT:
            LOGGER->Print("ERROR_INVALID_SERVICE_ACCOUNT", LoggerType::Error);
            break;
        case ERROR_SERVICE_EXISTS:
            LOGGER->Print("ERROR_SERVICE_EXISTS", LoggerType::Error);
            break;
        default:
            LOGGER->Print("Undefined", LoggerType::Error);
        }
        CloseServiceHandle(hSCManager);
        return -1;
    }

    SERVICE_DESCRIPTION info;
    info.lpDescription = LPWSTR(L"Yellow Watcher Service: analysis of technological journals 1C");
    ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &info);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    LOGGER->Print("Success install service!", true);

    return 0;
}

int RemoveService(LPCWSTR serviceName){
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(!hSCManager) {
        LOGGER->Print("Can't open Service Control Manager", LoggerType::Error);
        return -1;
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_STOP | DELETE);
    if(!hService) {
        LOGGER->Print("Can't remove service", LoggerType::Error);
        CloseServiceHandle(hSCManager);
        return -1;
    }
    
    DeleteService(hService);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    LOGGER->Print("Success remove service!", true);

    return 0;
}

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    LOGGER->Print("Yellow Watcher: ServiceWorkerThread: Entry", LoggerType::Trace);

    std::shared_ptr<YellowWatcher::Settings> settings = nullptr;
    std::shared_ptr<YellowWatcher::DirectoryWatcher> dw = nullptr;
    std::shared_ptr<YellowWatcher::Agregator> agregator = nullptr; 
    std::shared_ptr<YellowWatcher::Sender> sender = nullptr;
    Init(settings, dw, agregator, sender);
    time_point send = std::chrono::system_clock::now();

    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        LOGGER->Print("Yellow Watcher: ServiceWorkerThread: While", LoggerType::Trace);
        RunStep(dw, agregator, sender, settings, send);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    LOGGER->Print("Yellow Watcher: ServiceWorkerThread: Exit", LoggerType::Trace);
    
    return ERROR_SUCCESS;
}
