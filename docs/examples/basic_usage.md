# Basic Usage Example

Source: [`examples/basic_usage.cpp`](../../examples/basic_usage.cpp)

This example demonstrates a stream carrying a custom `Event` type, two independent subscribers, one consumer thread per subscriber, and a stop event for orderly thread completion.

## Event type

```cpp
struct Event {
    std::string message;
    bool stop;

    Event(const std::string& text, bool should_stop = false)
        : message(text), stop(should_stop) {}
};
```

The `message` field contains the event text. The `stop` flag tells a consumer to exit its loop. This stop flag is application logic; the EventBus itself does not provide a built-in shutdown mechanism.

## Consumer function

Each consumer repeatedly calls `pop()`. Because `pop()` blocks on an empty queue, the thread waits for a new event instead of polling in a loop.

When the consumer receives an event with `stop == true`, it exits.

## Program flow

1. Create `EventsBus::Stream<Event>`.
2. Call `subscribe()` twice to create independent queues for the logger and monitor.
3. Start one worker thread for each subscriber.
4. Publish regular events. The stream places a copy in both queues.
5. Publish a stop event. Each consumer receives its own copy and exits.
6. Join both threads, ensuring they finish before `main()` returns.
7. Unsubscribe both queues.

## Build and run

From the repository root:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread \
    -Iinclude examples/basic_usage.cpp -o eventbus_example
./eventbus_example
```

Output lines from the `Logger` and `Monitor` threads may interleave in different orders between runs. Each subscriber still receives the published messages in its own queue order.

## Why the stop event matters

Joining a thread that is blocked in `pop()` will not wake it. The example publishes a stop event before calling `join()` so each consumer can retrieve the event and leave its loop. If a subscriber thread can terminate for other reasons, design a corresponding queue wake-up or cancellation strategy in the application.
