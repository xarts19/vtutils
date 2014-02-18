#pragma once

#include <functional>

namespace VT
{
    // only single instance allowed
    class Daemon
    {
    public:
        Daemon();

        int main(int argc, char* argv[]);

    protected:
        ~Daemon();

        void on_signal(std::function<void()> handler);
        void wait_for_signal();

        virtual int run(int argc, char* argv[]) = 0;
        virtual void printUsage() = 0;
        virtual const char* syslogname() const = 0;
        virtual const char* lockname() const = 0;

    private:
        static Daemon* instance_;
    };
}
