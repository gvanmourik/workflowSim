
#ifndef SST_CORE_INTERPROCESS_CIRCULARBUFFER_H
#define SST_CORE_INTERPROCESS_CIRCULARBUFFER_H

#include <sstmutex.h>
#include <queue>
#include <vector>

namespace SST {
namespace Core {
namespace Interprocess {

template <typename T>
class CircularBuffer {

private:
    SSTMutex bufferMutex;
    size_t buffSize;
    size_t readIndex;
    size_t writeIndex;
    T buffer[0];


public:
	CircularBuffer(size_t mSize = 0) {
		buffSize = mSize;
		readIndex = 0;
		writeIndex = 0;
	}

    void setBufferLength(const size_t bufferLength)
    {
        if ( buffSize != 0 )
        {
	            fprintf(stderr, "Already specified size for buffer\n");
        	    exit(1);
        }

        buffSize = bufferLength;
        __sync_synchronize();
    }

	//Read buffer->result vector after clearing result vector
	bool read(std::vector<T> &result) 
	{
		int loop_counter = 0;
		bool notReady = true;
		result.clear();

		while( true ) 
		{
			bufferMutex.lock();
			//printf("Loop counter value = %d\n", loop_counter);
			//printf(" readIndex = %lu\n", readIndex);
			//printf("writeIndex = %lu\n", writeIndex);

			if (readIndex == writeIndex)
			{
                //printf("Inside [r]check #1\n");
				bufferMutex.unlock();
				bufferMutex.processorPause(loop_counter++);
				
				continue;
			}

			printf("Reading data...\n");
			while( readIndex != writeIndex ) 
			{
				//Mark the end last section being read
				if ( buffer[readIndex] == 0 )
				{
                    printf("The end has been reached...\n");
                    result.push_back(-7777);
					bufferMutex.unlock();
					return true;
				}

				if ( buffer[readIndex] != 0 )
				{
					result.push_back( buffer[readIndex] );
				}				
				readIndex = (readIndex + 1) % buffSize;	
			}
			
			bufferMutex.unlock();		
			return true;
		}
	}

    //NEED TO EDIT
	bool readNB(T* result) {
        if( bufferMutex.try_lock() )
        {
            if( readIndex != writeIndex )
            {
				*result = buffer[readIndex];
				readIndex = (readIndex + 1) % buffSize;

				bufferMutex.unlock();
				return true;
			} 

			bufferMutex.unlock();
		}

		return false;
	}

	//modify so write() takes a CONST
    //Takes a queue, and empties it while transferring to buffer
	int write(std::queue<T> &v) 	
	{
		int loop_counter = 0;
		int T_count = 0;

		printf("<<< 1 >>>\n");	
		while( true ) 
		{
			printf("Inside [w]check #1\n");
			bufferMutex.lock();
			
			if ( ((writeIndex + 1) % buffSize) == readIndex )
			{
				printf("Inside [w]check #2\n");
				
				bufferMutex.unlock();
				bufferMutex.processorPause(loop_counter++);
				
				return 0;
			}
			
			printf("writing data...\n");
			while ( ((writeIndex + 1) % buffSize) != readIndex ) 
			{
				if (!v.empty())
				{
					buffer[writeIndex] = v.front();
					v.pop();
				}
				else
				{
					buffer[writeIndex] = 0;
				}
				writeIndex = (writeIndex + 1) % buffSize;
				T_count++;
			}
			
			printf("Write index = %lu\n", writeIndex);
			printf(" Read index = %lu\n", readIndex);

			__sync_synchronize();
			bufferMutex.unlock();
		
			return T_count;
		}
	}

	~CircularBuffer() {

	}

    void clearBuffer() {
		bufferMutex.lock();
		readIndex = writeIndex;
		__sync_synchronize();
		bufferMutex.unlock();
	}


};

}
}
}

#endif
