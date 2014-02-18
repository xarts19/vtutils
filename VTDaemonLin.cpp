#include "VTDaemonLin.h"

#include "VTUtil.h"

#include <stdexcept>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sys/resource.h>

#define LOCKFILE_DIRECTORY "/var/lock/"
#define MAX_LOCKFILE_PATH 256
#define FLAG_CONSOLE_LONG "--console"
#define FLAG_HELP_LONG "--help"
#define FLAG_HELP_SHORT "-h"

volatile sig_atomic_t g_quit = 0;


static void term_handler(int signum)
{
    if (signum == SIGTERM || signum == SIGINT)
    {
        g_quit = 1;
    }
}

static void child_handler(int signum)
{
    switch(signum) {
    case SIGALRM: exit(EXIT_FAILURE); break;
    case SIGUSR1: exit(EXIT_SUCCESS); break;
    case SIGCHLD: exit(EXIT_FAILURE); break;
    }
}

static void daemonize(const char* lockname)
{
    /* already a daemon
     * if this is a daemon, it means that its parent process was killed,
     * so its parent process is 'init', so getppid() will return id of 'init'
     * which equals 1.
     */
    if (getppid() == 1)
        return;

    if (strlen(lockname) + sizeof(LOCKFILE_DIRECTORY) > MAX_LOCKFILE_PATH)
        throw std::runtime_error("Lockfile name is too long");

    /* Create the lock file as the current user */
    char lockfile[MAX_LOCKFILE_PATH];
    strcat(lockfile, LOCKFILE_DIRECTORY);
    strcat(lockfile, lockname);
    int lfp = open(lockfile, O_RDWR|O_CREAT, 0640);
    if (lfp < 0)
    {
        syslog( LOG_ERR, "unable to create lock file %s, code=%d (%s)",
                lockfile, errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }

    /* Trap signals that we expect to recieve */
    signal(SIGCHLD, child_handler);
    signal(SIGUSR1, child_handler);
    signal(SIGALRM, child_handler);

    /* Fork off the parent process */
    pid_t pid = fork();
    if (pid < 0)
    {
        syslog( LOG_ERR, "unable to fork daemon, code=%d (%s)",
                errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then we can exit the parent process. */
    if (pid > 0)
    {

        /* Wait for confirmation from the child via SIGTERM or SIGCHLD, or
           for two seconds to elapse (SIGALRM).  pause() should not return. */
        alarm(2);
        pause();

        exit(EXIT_FAILURE);
    }

    /* At this point we are executing as the child process */
    pid_t parent = getppid();

    /* Cancel certain signals */
    signal(SIGCHLD, SIG_DFL); /* A child process dies */
    signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP,  SIG_IGN); /* Ignore hangup signal */

    /* Change the file mode mask */
    umask(0);

    /* Create a new SID for the child process */
    pid_t sid = setsid();
    if (sid < 0)
    {
        syslog( LOG_ERR, "unable to create a new session, code %d (%s)",
                errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }

    /* Change the current working directory.  This prevents the current
       directory from being locked; hence not being able to remove it. */
    if ((chdir("/")) < 0)
    {
        syslog( LOG_ERR, "unable to change directory to %s, code %d (%s)",
                "/", errno, strerror(errno) );
        exit(EXIT_FAILURE);
    }

    /* Redirect standard files to /dev/null */
    FILE* unused;
    unused = freopen("/dev/null", "r", stdin);
    unused = freopen("/dev/null", "w", stdout);
    unused = freopen("/dev/null", "w", stderr);
    VT_UNUSED(unused);

    /* Tell the parent process that we are A-okay */
    kill(parent, SIGUSR1);
}

VT::Daemon* VT::Daemon::instance_ = nullptr;

namespace VT
{
    Daemon::Daemon()
    {
        if (instance_ != nullptr)
            throw std::logic_error("Only 1 instance of VT::Daemon is allowed");

        instance_ = this;
    }

    Daemon::~Daemon()
    {
        instance_ = nullptr;
    }

    int Daemon::main(int argc, char *argv[])
    {
        // core dumps may be disallowed by parent of this process; change that
        struct rlimit core_limits;
        core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_CORE, &core_limits);

        // Initialize the logging interface
        openlog(syslogname(), LOG_PID, LOG_LOCAL5);
        syslog(LOG_INFO, "Starting service...");

        bool is_daemon = false;

        try
        {
            std::vector<std::string> args(argv, argv + argc);

            bool run_as_daemon = true;
            if (argc > 1)
            {
                if (std::find(args.begin(), args.end(), FLAG_HELP_SHORT) != args.end() ||
                    std::find(args.begin(), args.end(), FLAG_HELP_LONG) != args.end() )
                {
                    printUsage();
                    std::cout << "Daemon options:\n";
                    std::cout << "  " FLAG_CONSOLE_LONG "    - run as usual concole application\n";
                    std::cout << "  " FLAG_HELP_SHORT ", " FLAG_HELP_LONG "   - show help message\n";
                    return 0;
                }
                else if (std::find(args.begin(), args.end(), FLAG_CONSOLE_LONG) != args.end())
                {
                    run_as_daemon = false;
                }
            }

            if (run_as_daemon)
            {
                syslog(LOG_INFO, "Switching to daemon mode...");
                daemonize(lockname());
                is_daemon = true;
            }
            else
            {
                syslog(LOG_INFO, "Running as console application");
            }

            // Exit gracefully on SIGTERM and SIGINT
            signal(SIGINT, term_handler);
            signal(SIGTERM, term_handler);

            return run(argc, argv);
        }
        catch (const std::exception& ex)
        {
            if (!is_daemon)
                std::cout << "Fatal error: " << ex.what();

            syslog(LOG_ERR, "Fatal error: %s", ex.what());
            return 1;
        }
        catch (...)
        {
            if (!is_daemon)
                std::cout << "Unknown error occured";

            syslog(LOG_ERR, "Unknown error occured");
            return 1;
        }
    }

    void Daemon::on_signal(std::function<void()> handler)
    {
        std::thread th([=](){
            instance_->wait_for_signal();
            handler();
        });
        th.detach();
    }

    void Daemon::wait_for_signal()
    {
        while (true)
        {
            // do real work here...
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if(g_quit)
                break;    // exit normally after SIGTERM or SIGINT
        }
    }
}
