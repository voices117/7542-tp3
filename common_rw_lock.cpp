#include "common_rw_lock.h"
#include <mutex>

/**
 * @brief Blocks until the lock is available for writing.
 * Once unblocked, the lock will be held until the returned `unique_lock` is
 * destructed.
 * @return Write lock that must be held until no writing access is required.
 */
std::unique_lock<std::mutex> Concurrency::RWLock::wait_write_access() {
    std::unique_lock<std::mutex> lock(this->mutex);
    while (this->readers) {
        this->cv.wait(lock);
    }
    return lock;
}

/**
 * @brief Blocks until the lock is available for reading.
 * The read lock will be held until `release_read_access` is called.
 * Readers do not need to check for the existance of writers, because a
 * writer will hold the lock, so the fact that the mutex was taken means
 * that there is no current writer.
 */
void Concurrency::RWLock::wait_read_access() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->readers += 1;
}

/**
 * @brief Releases a read lock.
 */
void Concurrency::RWLock::release_read_access() {
    std::unique_lock<std::mutex> lock(this->mutex);
    if (this->readers > 0) {
        this->readers -= 1;
    }
    if (this->readers == 0) {
        /* if there are no readers, notifies for any thread that may be
         * blocked waiting for write access */
        this->cv.notify_one();
    }
}
