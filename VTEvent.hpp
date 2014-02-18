#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <assert.h>

namespace VT
{
    static const int VT_INFINITE = -1;

    class Event
    {
    public:
		explicit Event(bool manual_reset = true)
			: lock_()
			, cond_()
			, is_signaled_(false)
            , manual_reset_(manual_reset)
		{
		}

        ~Event() { }

		void signal()
		{
			if (is_signaled_)
				return;
                
            std::unique_lock<std::mutex> lk(lock_);

            if (manual_reset_)
				is_signaled_ = true;

			cond_.notify_all();
		}

		void reset()
		{
			is_signaled_ = false;
		}
        
        // return true if event is signaled and false on timeout
        // if timeout == -1, waits infinitely
		bool wait(int timeout_ms = VT_INFINITE)
		{
			assert( (timeout_ms >= 0 || timeout_ms == VT_INFINITE) && "Incorrect timeout value" );
            
			std::unique_lock<std::mutex> lk(lock_);
            
            if (is_signaled_)
                return true;

			if (timeout_ms != -1)
			{
				bool res = cond_.wait_for(lk, std::chrono::milliseconds(timeout_ms), [&]{ return is_signaled_ == true; });
				return res;  // res is true if event was signaled before timeout
			}
			else
			{
				cond_.wait(lk, [&]{ return is_signaled_ == true; });
				return true;
			}
		}

        bool is_signaled() const { return is_signaled_; }

    private:
        Event(const Event&);
        Event& operator=(const Event&);

        std::mutex lock_;
        std::condition_variable cond_;
        std::atomic<bool> is_signaled_;
        bool manual_reset_;
    };
}

