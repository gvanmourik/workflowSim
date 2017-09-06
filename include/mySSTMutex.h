#ifndef MY_MUTEX_H
#define MY_MUTEX_H

#include <sched.h>
#include <time.h>


#define SST_CORE_INTERPROCESS_LOCKED 1
#define SST_CORE_INTERPROCESS_UNLOCKED 0

class SSTMutex {

private:
    volatile int lockVal;

public:
	SSTMutex() {
		lockVal = SST_CORE_INTERPROCESS_UNLOCKED;
	}

	void processorPause(int currentCount) {
		if( currentCount < 64 ) {
#if defined(__x86_64__)
			asm volatile ("pause" : : : "memory");
#else
			// Put some pause code in here
#endif
		} else if( currentCount < 256 ) {
			sched_yield();
		} else {
			struct timespec sleepPeriod;
			sleepPeriod.tv_sec = 0;
			sleepPeriod.tv_nsec = 100;

			struct timespec interPeriod;
			nanosleep(&sleepPeriod, &interPeriod);
		}
	}

	void lock() {
		int loop_counter = 0;

		while( ! __sync_bool_compare_and_swap( &lockVal, SST_CORE_INTERPROCESS_UNLOCKED, SST_CORE_INTERPROCESS_LOCKED) ) {
			processorPause(loop_counter);
			loop_counter++;
		}
	}

	void unlock() {
		lockVal = SST_CORE_INTERPROCESS_UNLOCKED;
		__sync_synchronize();
	}

	bool try_lock() {
		return __sync_bool_compare_and_swap( &lockVal, SST_CORE_INTERPROCESS_UNLOCKED, SST_CORE_INTERPROCESS_LOCKED );
	}

};


class lockGuard {

public:
    explicit lockGuard(SSTMutex &_m) : _m_device(_m)
    {
        _m_device.lock();
    }

    ~lockGuard()
    {
        _m_device.unlock();
    }

    lockGuard(const lockGuard &) = delete;
    lockGuard& operator=(const lockGuard &) = delete;

private:
    SSTMutex &_m_device;

};


#endif