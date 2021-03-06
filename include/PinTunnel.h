#ifndef MY_IPCTUNNEL_H
#define MY_IPCTUNNEL_H

#include <fcntl.h>
#include <cstdio>
#include <vector>
#include <queue>
#include <string>
#include <errno.h>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <typeinfo> //remove

#include <macros.h>

/**
 * Tunneling class between two processes, connected by shared memory.
 * Supports multiple circular-buffer queues, and a generic region
 * of memory for shared data.
 *
 * @tparam SharedDataType  Type to put in the shared data region
 */
//template<typename SharedDataType>
class PinTunnel {

    struct TunnelData {
        volatile uint32_t expectedChildren;
        size_t shmSegSize;
        size_t numBuffers;
        size_t offsets[0];
    };

// protected:
//     SharedDataType *sharedData;

private:
    bool master;
    int fd;
    const char *filename;
    void *shmPtr;
    uint8_t *nextAllocPtr;
    size_t shmSize;
    TunnelData *td;
    std::vector<cir_buf_t *> circBuffs;


    template <typename T>
    std::pair<size_t, T*> reserveSpace(size_t extraSpace = 0)
    {
        size_t space = sizeof(T) + extraSpace;
        if ( ((nextAllocPtr + space) - (uint8_t*)shmPtr) > shmSize )
            return std::make_pair<size_t, T*>(0, NULL);
        T *ptr = (T*)nextAllocPtr;
        nextAllocPtr += space;
        new (ptr) T();  // Call constructor if need be
        auto spaceToNextAlloc = (uint8_t*)ptr - (uint8_t*)shmPtr;
        return std::make_pair(spaceToNextAlloc, ptr);
    }

    size_t static calculateShmemSize(size_t numOfBuffers, size_t bufferLength)
    {
        long page_size = sysconf(_SC_PAGESIZE);

        /* Count how many pages are needed, at minimum */
        size_t td = 1 + ((sizeof(TunnelData) + (1+numOfBuffers)*sizeof(size_t)) / page_size);
        size_t buffer = 1 + ( (CIR_BUF_SIZE + bufferLength) / page_size );
        //size_t shdata = 1 + ( sizeof(TunnelData) / page_size );

        /* NO extra pages */
        return ( td + (numOfBuffers * buffer) ) * page_size;
    }

    // size_t getShmSegSize()
    // {
    //     shmPtr = mmap(NULL, sizeof(TunnelData), PROT_READ, MAP_SHARED, fd, 0);
    //     if ( shmPtr == MAP_FAILED ) 
    //     {
    //         // Not using Output because IPC means Output might not be available
    //         // fprintf(stderr, "mmap 0 failed: %s\n", strerror(errno));
    //         printf("MMAP 0 failed: %s\n", strerror(errno));
    //         exit(1);
    //     }

    //     td = (TunnelData*)shmPtr;
    //     size_t result = td->shmSegSize;
    //     printf("shmSize2 [getShmSegSize() 2nd] = %zu\n", shmSize);
    //     if ( munmap(shmPtr, sizeof(TunnelData)) == -1 )
    //     {
    //         printf("MUNMAP 0 failed\n");
    //         //exit(1);
    //     }

    //     return result;
    // }


public:
    /**
     * Constructor
     * New tunnel for IPC Communications
     *
     * @param numBuffers Number of buffers for which we should tunnel
     * @param bufferLength How large each core's buffer should be
     */
    PinTunnel(size_t numBuffers, size_t bufferLength, uint32_t expectedChildren = 1) : master(true), shmPtr(NULL), fd(-1)
    {
        printf("\nCreating PinTunnel (parameterized constructor to initiate a tunnel)...\n\n");

        filename = MMAP_PATH;
        fd = open(filename, O_RDWR|O_CREAT, 0660);
        if ( fd < 0 ) {
            // Not using Output because IPC means Output might not be available
            fprintf(stderr, "Failed to create IPC region '%s': %s\nCheck path in 'macros.h'\n", filename, strerror(errno));
            exit(1);
        }

        shmSize = calculateShmemSize(numBuffers, bufferLength);
        if ( ftruncate(fd, shmSize) ) {
            // Not using Output because IPC means Output might not be available
            fprintf(stderr, "Resizing shared file '%s' failed: %s\n", filename, strerror(errno));
            exit(1);
        }

        shmPtr = mmap(NULL, shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if ( shmPtr == MAP_FAILED ) {
            // Not using Output because IPC means Output might not be available
            fprintf(stderr, "mmap failed: %s\n", strerror(errno));
            exit(1);
        }
        nextAllocPtr = (uint8_t*)shmPtr;
        memset(shmPtr, '\0', shmSize);

        /* Construct our private buffer first.  Used for our communications */
        auto commBufSize = (1+numBuffers)*sizeof(size_t);
        auto resResult = reserveSpace<TunnelData>(commBufSize);
        td = resResult.second;
        td->expectedChildren = expectedChildren;
        td->shmSegSize = shmSize;
        td->numBuffers = numBuffers;

        // /* Construct user's shared-data region */
        // auto shareResult = reserveSpace<SharedDataType>(0);
        // td->offsets[0] = shareResult.first;
        // sharedData = shareResult.second;

        /* Construct the circular buffers */
        const size_t cbSize = TRACE_ENTRY_SIZE * bufferLength;
        for ( size_t c = 0 ; c < td->numBuffers ; c++ )
        {
            cir_buf_t* cPtr = NULL;

            auto resResult = reserveSpace<cir_buf_t>(cbSize);
            td->offsets[c] = resResult.first;

            cPtr = resResult.second;
            cPtr->setBufferLength(bufferLength);
            circBuffs.push_back(cPtr);
        }

    }

    /**
     * Constructor
     * Access an existing Tunnel
     */
    PinTunnel() : master(false), shmPtr(NULL), fd(-1)
    {
        printf("\nCreating PinTunnel (default constructor for existing tunnel)...\n\n");

        filename = MMAP_PATH;
        fd = open(filename, O_RDWR, 0660);
        if ( fd < 0 ) 
        {
            // Not using Output because IPC means Output might not be available
            // fprintf(stderr, "Failed to open file '%s': %s\n",
            // filename, strerror(errno));
            printf("Failed to open IPC region '%s': %s\n",
                    filename, strerror(errno));
            exit(1);
        }

        // shmSize = getShmSegSize();
        //-----------------------
        // Find shmSize
        //-----------------------
        shmPtr = mmap(NULL, sizeof(TunnelData), PROT_READ, MAP_SHARED, fd, 0);
        if ( shmPtr == MAP_FAILED ) 
        {
            // Not using Output because IPC means Output might not be available
            // fprintf(stderr, "mmap 0 failed: %s\n", strerror(errno));
            printf("MMAP 0 failed: %s\n", strerror(errno));
            exit(1);
        }

        td = (TunnelData*)shmPtr;
        shmSize = td->shmSegSize;
        if ( munmap(shmPtr, sizeof(TunnelData)) == -1 )
        {
            printf("MUNMAP 0 failed\n");
            //exit(1);
        }

        //-----------------------
        // Map PinTunnel
        //-----------------------
        shmPtr = mmap(NULL, shmSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if ( shmPtr == MAP_FAILED ) 
        {
            // Not using Output because IPC means Output might not be available
            // fprintf(stderr, "mmap 1 failed: %s\n", strerror(errno));
            printf("MMAP 1 failed: %s\n", strerror(errno));
            exit(1);
        }
        td = (TunnelData*)shmPtr;
        //sharedData = (SharedDataType*)((uint8_t*)shmPtr + td->offsets[0]);

        for ( size_t c = 0 ; c < td->numBuffers ; c++ ) 
        {
            cir_buf_t *toAdd = (cir_buf_t*)( (uint8_t*)shmPtr + td->offsets[c] );

            circBuffs.push_back( toAdd );
        }

        /* Clean up if we're the last to attach */
        if ( --td->expectedChildren == 0 ) 
        {
            // printf("Closing file...\n");
            close(fd);
        }
    }

    /**
     * Destructor
     */
    virtual ~PinTunnel()
    {
        // printf("PinTunnel Destructor...\n");
        shutdown(true);
    }

    /**
     * Shutdown
     */
    void shutdown(bool all = false)
    {
        if ( master ) {
            for ( cir_buf_t *cb : circBuffs ) {
                cb->~cir_buf_t();
            }
        }
        if ( shmPtr ) {
            munmap(shmPtr, shmSize);
            shmPtr = NULL;
            shmSize = 0;
        }
        if ( fd >= 0 ) {
            close(fd);
            fd = -1;
        }
    }

    // /** return a pointer to the SharedDataType region */
    // SharedDataType* getSharedData()
    // {
    //     return sharedData;
    // }

    size_t getTunnelBufferLen(size_t buffer)
    {
        return circBuffs[buffer]->getBufferLength();
    }

    size_t getTunnelReadIndex(size_t buffer)
    {
        return circBuffs[buffer]->getReadIndex();
    }

    /** Writes a queue of traces to buffer. Blocks until space is available. **/
    int writeTraceSegment(size_t buffer, std::queue<trace_entry_t> &traceSegment)
    {
        return circBuffs[buffer]->write(traceSegment);
    }

    /** Returns a  Blocks until a command is available **/
    void readTraceSegment(size_t buffer, std::vector<trace_entry_t> &traceSegment)
    {
        circBuffs[buffer]->read(traceSegment);
    }

    /**
     * NEED TO EDIT! (and readNB() in myCircularBuffer.h)
     *
     * Non-blocking version of readMessage
    **/
    bool readMessageNB(size_t buffer, trace_entry_t *result)
    {
        return circBuffs[buffer]->readNB(result);
    }

    /** Empty the messages in the buffer **/
    void clearBuffer(size_t buffer)
    {
       circBuffs[buffer]->clearBuffer();
    }

};

#endif /* MYIPCTUNNEL_H */
