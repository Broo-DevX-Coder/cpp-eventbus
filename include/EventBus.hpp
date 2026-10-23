#pragma once

/**
 * @file EventBus.hpp
 * @brief A lightweight, thread-safe event streaming system for C++.
 *
 * @details
 * This header provides a simple event communication mechanism based on
 * thread-safe queues and streams.
 *
 * Features:
 * - Thread-safe message queues.
 * - Blocking message retrieval.
 * - Multiple independent subscribers per stream.
 * - Message broadcasting to all active subscribers.
 * - Dynamic subscription and unsubscription.
 *
 * @note
 * This implementation uses C++11 standard library concurrency primitives.
 *
 * @author Broo DevX
 */

// ============================================================================
// Library Export / Import
// ============================================================================

/**
 * @brief Controls symbol visibility when building or consuming a shared library.
 */
#ifdef BUILDING_COMPILER_DLL
    #if defined(_WIN32)
        #define API __declspec(dllexport)
    #else
        #define API __attribute__((visibility("default")))
    #endif
#else
    #if defined(_WIN32)
        #define API __declspec(dllimport)
    #else
        #define API
    #endif
#endif

// ============================================================================
// Standard Library Dependencies
// ============================================================================

#include <algorithm>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

// ============================================================================
// Namespace: EventsBus
// ============================================================================

namespace EventsBus {

    /**
     * @class Queue
     * @brief A thread-safe FIFO queue that supports blocking message retrieval.
     *
     * @tparam QueueType The type of messages stored in the queue.
     *
     * @details
     * Queue provides synchronized access to an internal FIFO queue.
     *
     * Multiple threads can safely push messages into the queue.
     * A consumer can retrieve messages using pop(), which blocks until
     * a message becomes available.
     *
     * @note
     * The queue does not currently support cancellation or explicit shutdown.
     * A thread blocked inside pop() will remain blocked until a message
     * becomes available.
     */
    template<typename QueueType>
    class Queue {
    private:
        std::queue<QueueType> queue_;
        std::mutex mutex_;
        std::condition_variable cv_;

    public:

        /**
         * @brief Adds a message to the end of the queue.
         *
         * @param Value The message to enqueue.
         *
         * @details
         * The queue is locked while the message is inserted.
         * After insertion, one waiting consumer is notified.
         *
         * @note
         * This overload copies the supplied message.
         */
        void push(const QueueType& Value) {
            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                this->queue_.push(Value);
            }

            this->cv_.notify_one();
        }

        /**
         * @brief Retrieves and removes the next message from the queue.
         *
         * @return QueueType The oldest available message.
         *
         * @details
         * If the queue is empty, the calling thread blocks until a message
         * becomes available.
         *
         * Once a message is available, it is moved out of the queue
         * and removed from the internal storage.
         *
         * @warning
         * This function can block indefinitely if no message is ever pushed.
         */
        [[nodiscard]]
        QueueType pop() {
            std::unique_lock<std::mutex> lock(this->mutex_);

            this->cv_.wait(lock, [this] {
                return !this->queue_.empty();
            });

            QueueType value = std::move(this->queue_.front());
            this->queue_.pop();

            return value;
        }
    };


    /**
     * @class Stream
     * @brief A thread-safe message stream with multiple independent subscribers.
     *
     * @tparam QueueT The type of messages transmitted through the stream.
     *
     * @details
     * Stream allows multiple consumers to subscribe to the same source
     * of messages.
     *
     * Each subscriber owns an independent Queue. When publish() is called,
     * the message is delivered to every subscriber registered at the time
     * the publication takes its subscriber snapshot.
     *
     * Subscribers can be added dynamically using subscribe() and removed
     * using unsubscribe().
     *
     * @note
     * Each subscriber receives its own queued copy of each published message.
     * Messages published before a subscriber joins are not delivered to it.
     */
    template<typename QueueT>
    class Stream {
    private:
        using QueuePtr = std::shared_ptr<Queue<QueueT>>;

        std::vector<QueuePtr> subscribers_;
        std::mutex mutex_;

    public:

        /**
         * @brief Registers a new subscriber to the stream.
         *
         * @return QueuePtr A shared pointer to the subscriber's message queue.
         *
         * @details
         * A new independent queue is created and registered with the stream.
         * The returned queue can be used by a consumer to retrieve messages
         * published after the subscription becomes active.
         *
         * @code
         * EventsBus::Stream<std::string> stream;
         * auto subscriber = stream.subscribe();
         * @endcode
         */
        QueuePtr subscribe() {
            std::lock_guard<std::mutex> lock_(this->mutex_);

            auto sub = std::make_shared<Queue<QueueT>>();
            this->subscribers_.push_back(sub);

            return sub;
        }

        /**
         * @brief Removes a subscriber from the stream.
         *
         * @param sub The shared pointer identifying the subscriber to remove.
         *
         * @return true if the subscriber was found and removed; false otherwise.
         *
         * @details
         * After removal, future publications that take their subscriber
         * snapshot will not include this subscriber.
         *
         * Messages already queued remain available through the subscriber's
         * shared pointer.
         *
         * @warning
         * Unsubscribing does not wake a thread blocked inside the subscriber's
         * pop() function.
         */
        bool unsubscribe(const QueuePtr& sub) {
            std::lock_guard<std::mutex> lock_(this->mutex_);

            auto it = std::find(
                this->subscribers_.begin(),
                this->subscribers_.end(),
                sub
            );

            if (it != this->subscribers_.end()) {
                this->subscribers_.erase(it);
                return true;
            }

            return false;
        }

        /**
         * @brief Publishes a message to all registered subscribers.
         *
         * @param Value The message to publish.
         *
         * @details
         * A snapshot of the subscriber list is created while holding the
         * stream mutex. The message is then pushed into each subscriber's
         * queue.
         *
         * The stream mutex is not held while messages are being delivered,
         * allowing subscriptions to change without holding up the entire
         * delivery loop.
         *
         * @note
         * Each subscriber's queue is updated independently. This operation
         * does not guarantee that all subscribers receive the message
         * simultaneously.
         *
         * @note
         * This overload copies the supplied message into each subscriber's
         * queue.
         */
        void publish(const QueueT& Value) {
            std::vector<QueuePtr> subscribers_copy;

            {
                std::lock_guard<std::mutex> lock_(this->mutex_);
                subscribers_copy = subscribers_;
            }

            for (const auto& sub : subscribers_copy) {
                sub->push(Value);
            }
        }
    };

} // namespace EventsBus