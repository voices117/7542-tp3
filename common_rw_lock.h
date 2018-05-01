#ifndef RW_LOCK_H_
#define RW_LOCK_H_

#include <condition_variable>
#include <mutex>

namespace Concurrency {
/**
 * @brief Read/write lock used for the readers and writers problem.
 * To apply RAII using this class, see WriteLock and ReadLock.
 */
class RWLock {
   public:
    RWLock() {
    }
    ~RWLock() {
    }

    /** api */
    std::unique_lock<std::mutex> wait_write_access();
    void wait_read_access();
    void release_read_access();

   private:
    /** Number of readers holding the lock. */
    int readers{0};
    /** Internal mutex. */
    std::mutex mutex;
    /** Condition variable to notify writers when there are no readers left. */
    std::condition_variable cv;
};

/**
 * @brief Takes the RWLock in read only mode.
 * The lock may be taken by several threads in thi mode, but no thread in
 * write mode may take this lock while there's at least 1 reader.
 */
class ReadLock {
   public:
    explicit ReadLock(RWLock& lock) : lock(lock) {
        this->lock.wait_read_access();
    }
    ~ReadLock() {
        this->lock.release_read_access();
    }

   private:
    RWLock& lock;
};

/**
 * @brief Takes the RWLock in write mode.
 * No other thread may take the lock (any neither mode) while it's taken.
 */
class WriteLock {
   public:
    explicit WriteLock(RWLock& lock) {
        this->lock = lock.wait_write_access();
    }
    ~WriteLock() {
    }

   private:
    std::unique_lock<std::mutex> lock;
};
}  // namespace Concurrency

#endif
