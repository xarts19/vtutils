#include "VTCPPLogger.h"

int main()
{
    VT::LogFactory log_factory;
    VT::Logger& logger_file = log_factory.stream("main_file", std::make_shared<std::ofstream>("1.txt", std::ios_base::app));
    logger_file() << "file test";
        
    VT::Logger& logger = log_factory.cout("main_cout");
    logger() << "Hello" << 55 << "done";
    logger() << std::hex << 11 << "done";

    logger().log("smth", "is", 25, "times", std::hex, 12, "wrong");
    logger().log();
    logger().log("Nothing");

    VT::Logger& logger2 = log_factory.cerr("main_err");
    logger2() << "Error stream";

    VT::Logger& logger3 = log_factory.noop("main_noop");
    logger3() << "You should not see this";

    getchar();
    return 0;
}