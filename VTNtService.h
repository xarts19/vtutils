#ifndef __NTSERVICE_H_INCLUDED
#define __NTSERVICE_H_INCLUDED

#include "fn_win32.h"
#include <dbt.h>

class NtService
{
public:

    NtService();
    ~NtService();

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
    //                --delayed                 - turn on DelayedAutoStart option, service group is not registered;              
    //            --uninstall               - uninstalls the service "name";
    //            --start                   - starts the service "name";
    //            --stop                    - stops the service "name";
    //            --name <name>             - changes service name to "name"
    //            --display <display name>  - changes service display name to "display name"
    //            -h, --help                - show help message.
    //

    int
    main(
        IN int argc, 
        IN wchar_t* argv[]
        );

protected:

	static Event m_stoppedEvent;

    struct ServiceParameters
    {
        ServiceParameters():
            serviceName(0),
            displayName(0),
            desiredAccess(0),
            serviceType(0),
            startType(0),
            errorControl(0),
            loadOrderGroup(0),
            tagID(0),
            dependencies(0)
        {
            description.lpDescription = 0;
        }

        LPTSTR  serviceName;
        LPTSTR  displayName;

        SERVICE_DESCRIPTION description;

        DWORD   desiredAccess;  // default: SERVICE_ALL_ACCESS
        DWORD   serviceType;    // default: SERVICE_WIN32_OWN_PROCESS
        DWORD   startType;      // default: SERVICE_AUTO_START
        DWORD   errorControl;   // default: SERVICE_ERROR_NORMAL
        LPTSTR  loadOrderGroup; // default: NULL
        DWORD   tagID;          // retrieves the tag identifier
        LPTSTR  dependencies;   // default: NULL
    };

    virtual
    const ServiceParameters&
    parameters() const = 0;

    virtual
	void
    printUsage() = 0;

    virtual
    int
    run(
        IN int argc,
        IN const wchar_t* argv[]
        ) = 0;

    virtual
    void
    stop();

    virtual
    void
    pause() = 0;

    virtual
    void
    cont() = 0;

    virtual
    void
    shutdown() = 0;

    bool
    isConsole() const
    {
        return m_console;
    }

    //
    // Function:  reportStatus
    //
    // Purpose:   Reports service of status.
    //

    bool
    reportStatus(
        IN DWORD currentState,
        IN DWORD waitHint = 3000,
        IN DWORD errExit = 0
        );

	virtual
	DWORD 
		OnDeviceEvent(
            IN DWORD	/*dwEventType*/,
            IN PDEV_BROADCAST_HDR	/*pDevBcastHeader*/
			) 
	{
		return NO_ERROR;
	}

	SERVICE_STATUS_HANDLE 
		statusHandle() {return  m_statusHandle;}

private:

    const wchar_t*
    serviceName() const
    {
        return m_name ? m_name : m_params.serviceName;
    }

    const wchar_t*
    displayName() const
    {
        return m_display ? m_display : m_params.displayName;
    }

    //
    // entry points
    //

    int
    startDispatcher();

    int
    startConsole();

    int
    installService();

    int
    uninstallService();

    int
    startService();

    int
    stopService();

    //
    // The service control
    //
    static
	DWORD
	WINAPI
	ServiceCtrlHandlerEx(
		DWORD	dwCtrlCode,
		DWORD	dwEventType,
		LPVOID	lpEventData,
		LPVOID	lpContext
		);

    static
    VOID 
    WINAPI
    ServiceMain(
        IN DWORD argc,
        IN LPTSTR* argv
        );

    static
    DWORD 
    WINAPI 
    watchdogProc(
        IN LPVOID arg
        );

	char* GetLastErrorText() ;

private:
    static NtService* ms_instance;

    int m_argc;
    const wchar_t** m_argv;

    //
    // 'true' if running in console mode
    //

    bool m_console;

    //
    // Service name and description.
    //

    const wchar_t* m_name;
    const wchar_t* m_display;

    //
    // Login and password for the service's account
    //

    const wchar_t* m_login;
    const wchar_t* m_password;
    
    //
    // DelayedAutoStart option (for Vista and higher)
    //
    BOOL m_delayedAutoStart;

    SERVICE_STATUS m_status;
    SERVICE_STATUS_HANDLE m_statusHandle;

    //
    // Bit-field of what control requests the service will accept 
    // (default: SERVICE_ACCEPT_STOP)
    //

    DWORD m_controlsAccepted; 

    DWORD m_checkPoint;

    ServiceParameters m_params;

	char m_error[512];
};

#endif __NTSERVICE_H_INCLUDED
