#include "VTCPPLogger.h"

int main()
{
    VT::LogFactory log_factory;
    const VT::Logger& logger_file = log_factory.stream("main_file", std::make_shared<std::ofstream>("1.txt", std::ios_base::app), VT::LogLevel::Debug);
    logger_file() << "file test";
        
    const VT::Logger& logger = log_factory.cout("main_cout", VT::LogLevel::Debug);
    logger() << "Hello" << 55 << "done";
    logger() << std::hex << 11 << "done";

    logger().log("smth", "is", 25, "times", std::hex, 12, "wrong");
    logger().log();
    logger(VT::LogLevel::Warning).log("WARNING!!!");

    const VT::Logger& logger2 = log_factory.cerr("main_err", VT::LogLevel::Debug);
    logger2() << "Error stream";

    const VT::Logger& logger3 = log_factory.noop("main_noop");
    logger3() << "You should not see this";

    getchar();
    return 0;
}