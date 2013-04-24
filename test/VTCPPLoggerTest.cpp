#include "../VTCPPLogger.h"

#include "../VTThread.h"

#include <fstream>

struct CTest : public VT::Thread
{
    static VT::Logger* log;

    void run()
    {
        for (int i = 0; i < 40; ++i)
        {
            VT::Thread::sleep(0);
            log->debug() << i << "We" << "are" << "spamming" << std::endl
                         << "and" << "splitting" << "the lines" << i;
        }
    }
};

VT::Logger* CTest::log = nullptr;

void concurrent_test()
{
    auto logger = VT::Logger::cout("concurrent");

    CTest thread1;
    CTest thread2;
    thread1.log = &logger;
    thread2.log = &logger;

    thread1.start();
    thread2.start();

    thread1.join();
    thread2.join();
}

int main()
{
    auto logger = VT::Logger::cout("default");

    logger.debug() << "Hello" << 55 << "done";
    logger.debug() << std::hex << 11 << "done";

    logger.debug() << "smth" << "is" << 25 << "times" << std::hex << 12 << "wrong";
    logger.debug() << "";
    logger.log(VT::LL_Debug) << "DEBUG!!!";
    logger.log(VT::LL_Info) << "Info!!!";
    logger.log(VT::LL_Warning) << "WARNING!!!";
    logger.log(VT::LL_Error) << "ERROR!!!";
    logger.log(VT::LL_Critical) << "CRITICAL!!!";
    logger.info() << "Info!!!";
    logger.warning() << "WARNING!!!";
    logger.error() << "ERROR!!!";
    logger.critical() << "CRITICAL!!!";

    logger.debug() << "NoSpace" << "is" << "not" << "set";

    logger.set(VT::LO_NoEndl);
    logger.set(VT::LO_NoSpace);

    logger.debug() << "NoSpace" << "and" << "NoEndl";
    logger.debug() << "is" << "set";

    logger.reset();
    logger.debug() << "Ok." << "Options" << "are" << "set" << "to" << "default";

    concurrent_test();

    // test file logging
    auto file_logger = VT::Logger::stream("file", "log.log");
    file_logger.debug() << "Test1";
    file_logger.warning() << "Warning!";

    return 0;
}
