#include "../VTThreadPool.h"

#include <iostream>

void work()
{
    VT::Thread::sleep(1000);
    std::cout << "done\n";
}

int main()
{
    VT::ThreadPool tp(2, 5);
    
    tp.run(work);
    
    VT::Thread::sleep(1000);
    
    tp.run(work);
    
    VT::Thread::sleep(1000);
    
    for (int i = 0; i < 10; ++i)
        tp.run(work);
        
    tp.wait_for_all();
    
    return 0;
}
