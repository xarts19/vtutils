#include "VTNtService.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Assert.h>
#include <stdio.h>

#define MAX_STOPPING_TIME (1000 * 33)

#define NTSERVICE_PARAMETERS            L"SYSTEM\\CurrentControlSet\\Services\\%s\\Parameters"
#define NTSERVICE_PARAMETERS_CMDLINE    L"CommandLine"

//
// Logging support (see: Log.h)
//

#define LOG_SERVICE(level, params)

#define LOG_DEBUG_SERVICE(params)   LOG_SERVICE(DEBUG, params)
#define LOG_VERBOSE_SERVICE(params) LOG_SERVICE(VERBOSE, params)
#define LOG_TRACE_SERVICE(params)   LOG_SERVICE(TRACE, params)
#define LOG_ERROR_SERVICE(params)   LOG_SERVICE(ERROR, params)

//
// Command line arguments
//

#define NTSERVICE_LONG_CONSOLE      L"console"
#define NTSERVICE_LONG_INSTALL      L"install"
#define NTSERVICE_LONG_UNINSTALL    L"uninstall"
#define NTSERVICE_LONG_START        L"start"
#define NTSERVICE_LONG_STOP         L"stop"
#define NTSERVICE_LONG_HELP         L"help"
#define NTSERVICE_LONG_LOGIN        L"login"
#define NTSERVICE_LONG_PASSWORD     L"password"
#define NTSERVICE_LONG_DELAYED      L"delayed"
#define NTSERVICE_LONG_NAME         L"name"
#define NTSERVICE_LONG_DISPLAY      L"display"

#define NTSERVICE_HELP              L"h"
#define NTSERVICE_HELP2             L"?"

NtService* NtService::ms_instance = 0;
Event NtService::m_stoppedEvent = 0;

NtService::NtService(): 
    m_argc(0), 
    m_argv(0),
    m_console(false),
    m_name(0),
    m_display(0),
    m_delayedAutoStart(0),
    m_login(0),
    m_password(0),
    m_statusHandle(0),
	m_controlsAccepted( SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN )
{
    assert(!ms_instance);

    ms_instance = this;

    //
    // SERVICE_STATUS members that rarely change
    //

    memset(&m_status, 0, sizeof(m_status));
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwServiceSpecificExitCode = 0;
    
    //
    // Check Windows version and set SERVICE_ACCEPT_PRESHUTDOWN
    //
    OSVERSIONINFO verInfo;
    
    memset(&verInfo, 0, sizeof(verInfo));
    
    verInfo.dwOSVersionInfoSize = sizeof(verInfo);
    
    if ( GetVersionEx(&verInfo) )
    {
        if ( verInfo.dwMajorVersion>=6 )
        {
            m_controlsAccepted = m_controlsAccepted | SERVICE_ACCEPT_PRESHUTDOWN;
        }
    }
    else
    {
        LOG_ERROR_SERVICE(("*ERROR* GetVersionEx failed: %s.",0));    
    }
}

NtService::~NtService()
{
    delete m_argv;
    m_argv = 0;

    ms_instance = 0;
}

//
// Function:  main
//
// Purpose:   The entry point of the application. Perform running as NT service activities.
//            Processes the following command line parameters:
//
//            --console                 - run as usual concole application;
//            --install                 - installs the service as service "name";
//                --login <name>        - the service account name;
//                --password <password> - the service account password;
//                --delayed             - turn on DelayedAutoStart option, service group is not registered;          
//            --uninstall               - uninstalls the service "name";
//            --start                   - starts the service "name";
//            --stop                    - stops the service "name";
//            --name <name>             - changes service name to "name"
//            --display <display name>  - changes service display name to "display name"
//            -h, --help                - show help message.
//

int
NtService::main(
    IN int argc, 
    IN wchar_t* argv[]
    )
{
    m_params = parameters();

    int (NtService::* action)() = &NtService::startDispatcher;
    bool showUsage = false;

    //
    // Copy parameters
    //

    m_argv = new const wchar_t* [argc];
    memset(m_argv, 0, sizeof(const wchar_t*) * argc);

    m_argc = 1;
    m_argv[0] = argv[0];

	//if(argc<2)
		//showUsage = true;

    for (int i = 1; i < argc; ++i)
    {
        if ((argv[i][0] == '-') && (argv[i][1] == '-'))
        {
            //
            // Long options
            //

            if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_CONSOLE))
            {
                action = &NtService::startConsole;
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_INSTALL))
            {
                action = &NtService::installService;
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_UNINSTALL))
            {
                action = &NtService::uninstallService;
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_START))
            {
                action = &NtService::startService;
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_STOP))
            {
                action = &NtService::stopService;
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_HELP))
            {
                showUsage = true;
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_LOGIN))
            {
                ++i;
                if (i < argc)
                {
                    m_login = argv[i];
                }
                else
                {
                    showUsage = true;
                }
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_PASSWORD))
            {
                ++i;
                if (i < argc)
                {
                    m_password = argv[i];
                }
                else
                {
                    showUsage = true;
                }
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_NAME))
            {
                ++i;
                if (i < argc)
                {
                    m_name = argv[i];
                }
                else
                {
                    showUsage = true;
                }
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_DISPLAY))
            {
                ++i;
                if (i < argc)
                {
                    m_display = argv[i];
                }
                else
                {
                    showUsage = true;
                }
            }
            else if (!wcscmp(argv[i] + 2, NTSERVICE_LONG_DELAYED))
            {
                m_delayedAutoStart = TRUE;
            }
            else
            {
                //
                // Unknown
                //

                m_argv[m_argc++] = argv[i];
            }
        }
        else if ((argv[i][0] == '-') || (argv[i][0] == '/'))
        {
            //
            // Short options
            //

            if (!wcscmp(argv[i] + 1, NTSERVICE_HELP) || !wcscmp(argv[i] + 1, NTSERVICE_HELP2))
            {
                showUsage = true;
            }
            else
            {
                //
                // Unknown
                //

                m_argv[m_argc++] = argv[i];
            }
        }
        else
        {
            //
            // Unknown
            //
			showUsage=true;
            m_argv[m_argc++] = argv[i];
        }
    }

    if (showUsage)
    {
        printUsage();
        return 0;
    }
    else
    {
        return (this->*action)();
    }
}

DWORD 
WINAPI 
NtService::watchdogProc(
    IN LPVOID /*arg*/
    )
{
    assert(ms_instance);

    //
    // Give a chance to stop
    //
    while (WAIT_OBJECT_0 != WaitForSingleObject(m_stoppedEvent.handle(), MAX_STOPPING_TIME / 2))
    {
        //
        // Need to wait some more
        //

        LOG_ERROR_SERVICE(("watchdog: service is still stopping... Updating service status."));

        ms_instance->reportStatus(SERVICE_STOP_PENDING, MAX_STOPPING_TIME);
    }

    return 0;
}

void
NtService::stop()
{
    if (!isConsole())
    {
        reportStatus(SERVICE_STOP_PENDING, MAX_STOPPING_TIME);
    }

    Thread watchdogThread(NULL, 4096, watchdogProc, 0, 0);

    if (!watchdogThread.isOk())
    {
        //
        // Stopping failed
        //

        LOG_ERROR_SERVICE(("watchdog: Cannot start watchdog thread: %s.", GetLastErrorText() ));

        ExitProcess(0);
    }
}

DWORD
WINAPI
NtService::ServiceCtrlHandlerEx(
	DWORD	dwCtrlCode,
	DWORD	dwEventType,
	LPVOID	lpEventData,
	LPVOID	lpContext
	)
{
	DWORD retVal = NO_ERROR;

    assert(ms_instance);

    //
    // Handle the requested control code.
    //
    switch (dwCtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        LOG_TRACE_SERVICE(("Got SERVICE_CONTROL_STOP signal."));

        ms_instance->m_status.dwCurrentState = SERVICE_STOP_PENDING;
        ms_instance->stop();
        break;

    case SERVICE_CONTROL_PAUSE:
        LOG_TRACE_SERVICE(("Got SERVICE_CONTROL_PAUSE signal."));

        //ms_instance->m_status.dwCurrentState = SERVICE_PAUSE_PENDING;
        //ms_instance->pause();
		retVal = ERROR_CALL_NOT_IMPLEMENTED;
        break;

    case SERVICE_CONTROL_CONTINUE:
        LOG_TRACE_SERVICE(("Got SERVICE_CONTROL_CONTINUE signal."));

        //ms_instance->m_status.dwCurrentState = SERVICE_CONTINUE_PENDING;
        //ms_instance->cont();
		retVal = ERROR_CALL_NOT_IMPLEMENTED;
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        LOG_TRACE_SERVICE(("*** SERVICE_CONTROL_SHUTDOWN signal. ***"));

        ms_instance->shutdown();
        break;

    case SERVICE_CONTROL_PRESHUTDOWN:
        LOG_TRACE_SERVICE(("*** SERVICE_CONTROL_PRESHUTDOWN signal. ***"));

        ms_instance->shutdown();
        break;

    case SERVICE_CONTROL_INTERROGATE:
        LOG_VERBOSE_SERVICE(("Got SERVICE_CONTROL_INTERROGATE signal."));

        ms_instance->reportStatus(ms_instance->m_status.dwCurrentState);
        break;

	case SERVICE_CONTROL_DEVICEEVENT:
		{
			//
			// return NO_ERROR to grant the request and an error code to deny the request. 
			//
			//LOG_VERBOSE_SERVICE(("SERVICE_CONTROL_DEVICEEVENT signal (0x%x).", dwEventType));

			NtService *srv = (NtService*)lpContext;

			retVal = 
				srv->OnDeviceEvent(
					dwEventType,
					(PDEV_BROADCAST_HDR)lpEventData
					);
		}
		break;

	default:
		//
        // unsupported control codes
		//
        LOG_TRACE_SERVICE("Unsupported control code\n");
		retVal = ERROR_CALL_NOT_IMPLEMENTED;
        break;
    }

	return retVal;
}

VOID 
WINAPI
NtService::ServiceMain(
    IN DWORD argc,
    IN LPTSTR* argv
    )
{
    assert(ms_instance);

    //
    // Get service name
    //

    if (argc > 0)
    {
        ms_instance->m_name = argv[0];
    }

    //
    // Register our service control handler.
    //

    ms_instance->m_statusHandle =
        RegisterServiceCtrlHandlerEx(
            ms_instance->serviceName(),
            NtService::ServiceCtrlHandlerEx,
			ms_instance // context
			);

    if (!ms_instance->m_statusHandle)
    {
        LOG_ERROR_SERVICE(("*ERROR* RegisterServiceCtrlHandler failed."));
    }
    else
    {
        //
        // Report the status to the service control manager.
        //

        if (ms_instance->reportStatus(SERVICE_START_PENDING))
        {
            //
            // Build command line
            //

            wchar_t* buf = 0;
            wchar_t** argv = 0;
            int argc = 0;

            //
            // Construct service registry path
            //

            DWORD bufSize = 0x1000;
            buf = new wchar_t[bufSize];

            swprintf(buf, bufSize, NTSERVICE_PARAMETERS, ms_instance->serviceName());

            HKEY paramsKey = 0;
            bool status = false;

            if (ERROR_SUCCESS == 
                RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE, 
                    buf,
                    0,
                    STANDARD_RIGHTS_READ | KEY_QUERY_VALUE,
                    &paramsKey))
            {
                DWORD type = 0;
                DWORD bytes = 0;

                if (
                    (ERROR_SUCCESS == RegQueryValueEx(paramsKey, NTSERVICE_PARAMETERS_CMDLINE, 0, &type, NULL, &bytes)) &&
                    (type == REG_MULTI_SZ) &&
                    (bytes > 0))
                {
                    if (bytes > bufSize)
                    {
                        delete[] buf;
                        buf = new wchar_t [bytes];
                    }

                    wchar_t* ptr = buf;
                    wchar_t* ptr_end = buf + bytes - 1;

                    if (ERROR_SUCCESS == RegQueryValueEx(paramsKey, NTSERVICE_PARAMETERS_CMDLINE, 0, &type, (LPBYTE)buf, &bytes))
                    {
                        //
                        // Find number of strings returned
                        //

                        for (argc = 0; ptr < ptr_end; ++ptr)
                        {
                            if (*ptr == '\0')
                            {
                                ++argc;
                            }
                        }

                        //
                        // Fill string pointers
                        //

                        if (argc > 0)
                        {
                            argv = new wchar_t* [argc];

                            argv[0] = buf;
                            int i = 1;

                            for (ptr = buf, --ptr_end; ptr < ptr_end; ++ptr)
                            {
                                if (*ptr == '\0')
                                {
                                    argv[i++] = ptr + 1;
                                }
                            }
                        }

                        //
                        // Command line arguments were extracted successfully.
                        //

                        status = true;
                    }
                }

                RegCloseKey(paramsKey);
            }

            ms_instance->run(argc, (const wchar_t**)argv);

            delete[] argv; 
            argv = 0;

            delete[] buf; 
            buf = 0;
        }

        //
        // Report the stopped status to the service control manager.
        //

        ms_instance->reportStatus(SERVICE_STOPPED);
    }
}


int
NtService::startDispatcher()
{
    LOG_TRACE_SERVICE(("Starting in service mode..."));


    int retval = 1;

    //
    // Note: For SERVICE_WIN32_SHARE_PROCESS services, each entry must contain 
    //       the name of a service. This name is the service name that was 
    //       specified by the CreateService function when the service was 
    //       installed. For SERVICE_WIN32_OWN_PROCESS services, the service 
    //       name in the table entry is ignored.
    //

    SERVICE_TABLE_ENTRY dispatchTable[] =
    {
        { LPTSTR(serviceName()), (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { 0, 0 }
    };

    BOOL status = StartServiceCtrlDispatcher(dispatchTable);

    if (status)
    {
        retval = 0;
    }
    else
    {
        LOG_ERROR_SERVICE(("*ERROR* StartServiceCtrlDispatcher failed: %s.", GetLastErrorText() ));
    }

    return retval;
}

int
NtService::startConsole()
{
    LOG_TRACE_SERVICE(("Starting in console mode..."));

    m_console = true;

    m_stoppedEvent.reset();
    int retval = run(m_argc, m_argv);
    m_stoppedEvent.set();

    return retval;
}

int
NtService::installService()
{
    int retval = 1;

    LOG_TRACE_SERVICE(("Installing '%s' service...", serviceName()));

    wchar_t path[0x1000];
    wchar_t* ptr = path;
    wchar_t* ptr_end = path + ARRAYSIZE(path) - 1;

    //
    // Get full name of the executeable
    //

    ptr += GetModuleFileName(0, path, ARRAYSIZE(path) - 1);
    if (ptr == path)
    {
        LOG_ERROR_SERVICE(("*ERROR* GetModuleFileName failed: %s.", GetLastErrorText() ));
        return retval;
    }

    //
    // Create the service
    //

    SC_HANDLE scmanager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if (scmanager)
    {
        SC_HANDLE service = 
            CreateService(
                scmanager,
                serviceName(),
                displayName(),
                m_params.desiredAccess,
                m_params.serviceType,
                m_params.startType,
                m_params.errorControl,
                path,
                m_delayedAutoStart ? 0 : m_params.loadOrderGroup,
                (
                    (m_params.serviceType == SERVICE_KERNEL_DRIVER || m_params.serviceType == SERVICE_FILE_SYSTEM_DRIVER) &&
                    (m_params.startType == SERVICE_BOOT_START || m_params.startType == SERVICE_SYSTEM_START)) 
                ?
                    &m_params.tagID 
                : 
                    0,
                m_params.dependencies,
                m_login,
                m_password);

        if (service)
        {
            //
            // Set description of the service.
            //
            // Note: ChangeServiceConfig2 is not available in NT 4.
            //

            HMODULE advapi32 = LoadLibrary(L"Advapi32.dll");
            if (advapi32)
            {
                typedef 
                    BOOL (WINAPI *FuncChangeServiceConfig2A)(
                        SC_HANDLE    hService,
                        DWORD        dwInfoLevel,
                        LPVOID       lpInfo
                        );

                FuncChangeServiceConfig2A changeServiceConfig2A = 
                    (FuncChangeServiceConfig2A)GetProcAddress(advapi32, "ChangeServiceConfig2A");

                if (changeServiceConfig2A)
                {
                    changeServiceConfig2A(service, SERVICE_CONFIG_DESCRIPTION, &m_params.description);
                    
                    if ( m_delayedAutoStart )
                    {
                        if (! changeServiceConfig2A(
                                    service, 
                                    SERVICE_CONFIG_DELAYED_AUTO_START_INFO,
                                    &m_delayedAutoStart))
                        {
                            LOG_ERROR_SERVICE(("*ERROR* can not set DelayedAutoStart option: %s.", GetLastErrorText() ));                            
                        }                                    
                    }
                }

                FreeLibrary(advapi32);
                advapi32 = 0;
            }

            //
            // Prepare command line parameters
            //

            ++ptr;

            for (int i = 1; (i < m_argc) && (ptr < ptr_end); ++i)
            {
                int delta = swprintf(ptr, ptr_end - ptr, L"%s%c", m_argv[i], L'\0');
                ptr = (delta >= 0) ? ptr + delta : ptr_end;
            }

            *ptr++ = '\0';

            //
            // Construct service registry path
            //

            wchar_t buf[0x1000];
            swprintf(buf, ARRAYSIZE(buf), NTSERVICE_PARAMETERS, serviceName());

            HKEY paramsKey = 0;
            bool status = false;

            if (ERROR_SUCCESS == 
                RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE, 
                    buf,
                    0,
                    NULL,
                    0,
                    KEY_WRITE,
                    NULL,
                    &paramsKey,
                    NULL))
            {
                if (ERROR_SUCCESS == 
					RegSetValueEx(
						paramsKey,
						NTSERVICE_PARAMETERS_CMDLINE,
						0, REG_MULTI_SZ,
						(const BYTE*)path,
						(ULONG)(ptr - path)
						) )
                {
                    status = true;
                }

                RegCloseKey(paramsKey);
            }

            if (!status)
            {
                LOG_ERROR_SERVICE(("*ERROR* Cannot set the parameters of the service: %s.", GetLastErrorText() ));
            }
            else
            {
                LOG_TRACE_SERVICE(("'%s' service has been installed successfully.", serviceName()));

                retval = 0;
            }

            CloseServiceHandle(service);
        }
        else
        {
            LOG_ERROR_SERVICE(("*ERROR* CreateService failed: %s.", GetLastErrorText() ));
        }

        CloseServiceHandle(scmanager);
    }
    else
    {
        LOG_ERROR_SERVICE(("*ERROR* OpenSCManager failed: %s.", GetLastErrorText() ));
    }

    return retval;
}

int
NtService::uninstallService()
{
    //
    // Try to stop the service first
    //

    int retval = stopService();

    if (retval == 0)
    {
        LOG_TRACE_SERVICE(("Uninstalling '%s' service...", serviceName()));

        SC_HANDLE scmanager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);

        if (scmanager)
        {
            SC_HANDLE service = OpenService(scmanager, serviceName(), SERVICE_ALL_ACCESS);

            if (service)
            {
                if (DeleteService(service))
                {
                    LOG_TRACE_SERVICE(("'%s' has been uninstalled successfully.", serviceName()));

                    retval = 0;
                } 
                else
                {
                    LOG_ERROR_SERVICE(("*ERROR* Failed to uninstall '%s' service: %s.", serviceName(), GetLastErrorText() ));
                }

                CloseServiceHandle(service);
            }
            else
            {
                LOG_ERROR_SERVICE(("*ERROR* OpenService failed: %s.", GetLastErrorText() ));
            }

            CloseServiceHandle(scmanager);
        }
        else
        {
            LOG_ERROR_SERVICE(("*ERROR* OpenSCManager failed: %s.", GetLastErrorText() ));
        }
    }

    return retval;
}

int
NtService::startService()
{
    int retval = 1;

    LOG_TRACE_SERVICE(("Starting '%s'...", serviceName()));

    SC_HANDLE scmanager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);

    if (scmanager)
    {
        SC_HANDLE service = OpenService(scmanager, serviceName(), SERVICE_ALL_ACCESS);

        if (service)
        {
            //
            // Try to start the service.
            //

            if (StartService(service, 0, 0))
            {
                Sleep(1000);

                while (QueryServiceStatus(service, &m_status))
                {
                    if (m_status.dwCurrentState != SERVICE_START_PENDING)
                    {
                        break;
                    }

                    Sleep(100);
                }

                if (m_status.dwCurrentState == SERVICE_RUNNING)
                {
                    LOG_TRACE_SERVICE(("'%s' has been started successfully.", serviceName()));

                    retval = 0;
                }
                else
                {
                    LOG_ERROR_SERVICE(("*ERROR* Failed to start '%s'.", serviceName()));
                }
            }
            else
            {
                DWORD lastError = GetLastError();
                if (ERROR_SERVICE_ALREADY_RUNNING == lastError)
                {
                    LOG_TRACE_SERVICE(("'%s' is already running.", serviceName()));

                    retval = 0;
                }
                else
                {
                    LOG_ERROR_SERVICE(("*ERROR* StartService failed: %s.", GetLastErrorText()));
                }
            }

            CloseServiceHandle(service);
        }
        else
        {
            LOG_ERROR_SERVICE(("*ERROR* OpenService failed: %s.", GetLastErrorText() ));
        }

        CloseServiceHandle(scmanager);
    }
    else
    {
        LOG_ERROR_SERVICE(("*ERROR* OpenSCManager failed: %s.", GetLastErrorText() ));
    }

    return retval;
}

int
NtService::stopService()
{
    int retval = 1;

    LOG_TRACE_SERVICE(("Stopping '%s'...", serviceName()));

    SC_HANDLE scmanager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);

    if (scmanager)
    {
        SC_HANDLE service = OpenService(scmanager, serviceName(), SERVICE_ALL_ACCESS);

        if (service)
        {
            //
            // Try to stop the service.
            //

            if (ControlService(service, SERVICE_CONTROL_STOP, &m_status))
            {
                Sleep(1000);

                while (QueryServiceStatus(service, &m_status))
                {
                    if (m_status.dwCurrentState != SERVICE_STOP_PENDING)
                    {
                        break;
                    }

                    Sleep(100);
                }

                if (m_status.dwCurrentState == SERVICE_STOPPED)
                {
                    LOG_TRACE_SERVICE(("'%s' has been stopped successfully.", serviceName()));

                    retval = 0;
                }
                else
                {
                    LOG_ERROR_SERVICE(("*ERROR* Failed to stop '%s': %s.", serviceName(), GetLastErrorText() ));
                }
            }
            else
            {
                DWORD lastError = GetLastError();
                if (ERROR_SERVICE_NOT_ACTIVE == lastError)
                {
                    LOG_TRACE_SERVICE(("'%s' has not beed started.", serviceName()));

                    retval = 0;
                }
                else
                {
                    LOG_ERROR_SERVICE(("*ERROR* ControlService failed: %s.", GetLastErrorText()));
                }
            }

            CloseServiceHandle(service);
        }
        else
        {
            LOG_ERROR_SERVICE(("*ERROR* OpenService failed: %s.", GetLastErrorText() ));
        }

        CloseServiceHandle(scmanager);
    }
    else
    {
        LOG_ERROR_SERVICE(("*ERROR* OpenSCManager failed: %s.", GetLastErrorText() ));
    }

    return retval;
}

bool
NtService::reportStatus(
    IN DWORD currentState,
    IN DWORD waitHint /* = 3000 */,
    IN DWORD errExit /* = 0 */
    )
{
    m_status.dwControlsAccepted = (currentState == SERVICE_START_PENDING) ? 0 : m_controlsAccepted;

    m_status.dwCurrentState = currentState;
    m_status.dwWin32ExitCode = NO_ERROR;
    m_status.dwWaitHint = waitHint;
    m_status.dwServiceSpecificExitCode = errExit;

    if (errExit != 0)
    {
        m_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }

    if (
        (currentState == SERVICE_RUNNING) ||
        (currentState == SERVICE_STOPPED))
    {
        m_status.dwCheckPoint = 0;
    }
    else
    {
        m_status.dwCheckPoint = ++m_checkPoint;
    }

    //
    // Inform watchdog about service status
    //

    if (currentState == SERVICE_STOPPED)
    {
        m_stoppedEvent.set();
    }
    else
    {
        m_stoppedEvent.reset();
    }

    //
    // Report the status of the service to the service control manager.
    //

    bool status = false;

    if (SetServiceStatus(m_statusHandle, &m_status))
    {
        status = true;
    }
    else
    {
        LOG_ERROR_SERVICE(("*ERROR* SetServiceStatus failed: %s.", GetLastErrorText() ));
    }

    return status;
}

char* NtService::GetLastErrorText() 
{ 
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR) &lpMsgBuf,
		0, NULL );

	strcpy_s(m_error,sizeof(m_error),(LPCSTR)lpMsgBuf);

	return m_error;
}
