# EventBus Architecture

This page describes the current implementation of the C++ EventBus. The implementation is header-only and lives in [`../include/EventBus.hpp`](../include/EventBus.hpp).

## Overview

The EventBus is composed of two template classes inside the `EventsBus` namespace:

- `Queue<T>` is a thread-safe FIFO queue for values of type `T`.
- `Stream<T>` manages subscriber queues and broadcasts each published value to its registered subscribers.

```text
                         Stream<T>
                             |
             +---------------+---------------+
             |               |               |
        Subscriber A    Subscriber B    Subscriber C
          Queue<T>        Queue<T>        Queue<T>
             |               |               |
         Consumer A      Consumer B      Consumer C
           thread          thread          thread
```

Each subscriber owns an independent queue. Reading from one subscriber does not remove the corresponding message from any other subscriber's queue.

## Message flow

1. A component calls `stream.subscribe()`.
2. The stream creates a new `Queue<T>`, registers it, and returns a `std::shared_ptr` to it.
3. A publisher calls `stream.publish(value)`.
4. The stream copies its subscriber list while holding its mutex, then releases that mutex.
5. The value is pushed into each queue in the snapshot.
6. A consumer calls `subscriber->pop()`. If the queue is empty, the consumer blocks until a value is pushed.

A subscriber only receives messages published after it has been registered. The stream does not retain history for future subscribers.

## Synchronization

### `Queue<T>`

- A `std::mutex` protects the internal `std::queue<T>`.
- `push()` locks the queue, inserts a copy of the supplied value, releases the lock, then calls `notify_one()`.
- `pop()` waits on a `std::condition_variable` until the queue is non-empty, then removes and returns the front value.

### `Stream<T>`

- A `std::mutex` protects the subscriber list.
- `subscribe()` and `unsubscribe()` modify the list while holding the mutex.
- `publish()` creates a snapshot of the list under the mutex, then delivers messages after releasing it. This avoids holding the stream mutex while locking each subscriber queue.

## Ordering and delivery semantics

- Each individual subscriber queue is FIFO: messages enqueued in sequence are popped in that sequence.
- Delivery to different subscribers happens sequentially inside `publish()`; it is not simultaneous.
- With concurrent calls to `publish()`, the implementation does not establish a single global event order across all publishers and subscribers.
- Because `publish()` works from a snapshot, a subscriber removed concurrently may still receive a message from a publication that already captured it.
- The current API copies each published value into every subscriber queue.

## Shutdown behavior

`Queue<T>` has no close or cancellation state. Calling `pop()` on an empty queue blocks until a message arrives. Unsubscribing does not wake a thread already waiting in `pop()`.

Applications should use an explicit application-level stop event or implement a queue shutdown mechanism before joining consumer threads. See [`examples/basic_usage.cpp`](../examples/basic_usage.cpp), which uses a stop flag in its event structure.

## Known implementation boundaries

- No queue capacity limit or back-pressure policy.
- No queue close, cancellation, or timed pop operation.
- No retained event history for new subscribers.
- No guarantee that an already-started publication stops delivering to a concurrently removed subscriber.
- Message types used with the current `push()` and `publish()` methods must be copyable.
