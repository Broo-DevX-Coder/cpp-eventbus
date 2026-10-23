# EventBus Usage Guide

## 1. Overview

The EventBus provides a thread-safe event communication system based on:

- `Queue<T>`: A thread-safe queue that stores messages and allows blocking message retrieval.
- `Stream<T>`: A channel that allows multiple subscribers to receive published messages.
- `subscribe()`: Creates a subscriber and returns its queue.
- `publish()`: Sends a message to all current subscribers.
- `pop()`: Waits until a message is available and retrieves it.
- `unsubscribe()`: Removes a subscriber from the stream.

Each subscriber has its own queue. Publishing an event sends a copy of that event to every subscriber.

---

## 2. Include the EventBus Header

Include the header containing your EventBus implementation.

```cpp
#include "EventsBus.hpp"
```

Replace the filename with your actual header filename.

---

## 3. Create an Event Stream

Create a `Stream<T>` object.

The template parameter represents the type of data that will be transmitted through the stream.

```cpp
EventsBus::streams::Stream<std::string> stream;
```

In this example, the stream will transmit strings.

You can also use other types:

```cpp
EventsBus::streams::Stream<int> stream;
```

Or custom event structures:

```cpp
struct Event {
    std::string name;
    int value;
};

EventsBus::streams::Stream<Event> stream;
```

---

## 4. Subscribe to the Stream

Call `subscribe()` to create a subscriber.

```cpp
auto subscriber = stream.subscribe();
```

The returned object is:

```cpp
std::shared_ptr<EventsBus::streams::Queue<std::string>>
```

This subscriber owns an independent queue.

Every message published after subscription will be pushed into its queue.

---

## 5. Publish an Event

Use `publish()` to send an event to all subscribers.

```cpp
stream.publish("Hello EventBus");
```

The stream distributes the message to every subscriber currently registered.

Example:

```cpp
auto subscriber1 = stream.subscribe();
auto subscriber2 = stream.subscribe();

stream.publish("Hello");
```

Both subscribers receive the message independently.

---

## 6. Receive an Event

Use `pop()` to retrieve a message.

```cpp
std::string message = subscriber1->pop();
```

Important:

`pop()` is a blocking operation.

If the queue is empty, the calling thread waits until a message becomes available.

Example:

```cpp
auto subscriber = stream.subscribe();

stream.publish("Hello");

std::string message = subscriber->pop();
```

The received message will be:

```text
Hello
```

---

## 7. Use EventBus with Multiple Subscribers

Each subscriber receives its own copy of every published event.

```cpp
EventsBus::streams::Stream<std::string> stream;

auto subscriber1 = stream.subscribe();
auto subscriber2 = stream.subscribe();

stream.publish("Event 1");

std::string message1 = subscriber1->pop();
std::string message2 = subscriber2->pop();
```

Both subscribers receive:

```text
Event 1
```

The first subscriber consuming its message does not remove the message from the second subscriber's queue.

---

## 8. Use EventBus with Threads

EventBus is useful for communication between different threads.

For example:

- Thread A publishes events.
- Thread B receives events.
- Thread B waits for messages without continuously checking the queue.

### Example

```cpp
#include "EventsBus.hpp"

#include <iostream>
#include <string>
#include <thread>

int main() {
    EventsBus::streams::Stream<std::string> stream;

    auto subscriber = stream.subscribe();

    std::thread worker([subscriber]() {
        std::string message = subscriber->pop();

        std::cout << "Received: "
                  << message
                  << std::endl;
    });

    stream.publish("Hello from EventBus");

    worker.join();

    return 0;
}
```

Output:

```text
Received: Hello from EventBus
```

### Execution Flow

1. Create the stream.
2. Subscribe to the stream.
3. Start the worker thread.
4. The worker waits inside `pop()`.
5. Publish an event.
6. The subscriber queue receives the event.
7. The worker wakes up and processes the message.
8. Join the worker thread.

---

## 9. Unsubscribe from the Stream

Use `unsubscribe()` to remove a subscriber.

```cpp
bool result = stream.unsubscribe(subscriber);
```

The function returns:

- `true`: The subscriber was found and removed.
- `false`: The subscriber was not registered.

Example:

```cpp
auto subscriber = stream.subscribe();

stream.publish("Event 1");

stream.unsubscribe(subscriber);
```

After unsubscribing, the subscriber will not receive newly published events.

However, messages already stored in its queue remain available.

**Important:** `unsubscribe()` does not automatically stop a thread waiting inside `pop()`.

---

## 10. Complete Example

```cpp
#include "EventsBus.hpp"

#include <iostream>
#include <string>
#include <thread>

int main() {
    // Create EventBus stream
    EventsBus::streams::Stream<std::string> stream;

    // Subscribe
    auto subscriber = stream.subscribe();

    // Create worker thread
    std::thread worker([subscriber]() {
        while (true) {
            std::string message = subscriber->pop();

            if (message == "EXIT")
                break;

            std::cout << "[EventBus] "
                      << message
                      << std::endl;
        }
    });

    // Publish events
    stream.publish("Event 1");
    stream.publish("Event 2");
    stream.publish("Event 3");

    // Stop worker
    stream.publish("EXIT");

    worker.join();

    // Unsubscribe
    stream.unsubscribe(subscriber);

    return 0;
}
```

Output:

```text
[EventBus] Event 1
[EventBus] Event 2
[EventBus] Event 3
```

---

## 11. Recommended Usage Pattern

For a typical ValtCommand application:

1. Create one stream for each event category.
2. Subscribe the components that need those events.
3. Keep each subscriber queue in a `shared_ptr`.
4. Publish events whenever something happens.
5. Let subscriber threads process events using `pop()`.
6. Unsubscribe components when they are no longer interested.
7. Ensure worker threads have a proper shutdown mechanism.

Example:

```cpp
EventsBus::streams::Stream<std::string> runtime_events;

auto debugger = runtime_events.subscribe();
auto inspector = runtime_events.subscribe();
```

Both the debugger and inspector can receive runtime events independently.

---

## 12. Important Implementation Notes

### Thread safety

`Queue<T>` protects its internal queue using a mutex.

`Stream<T>` protects its subscriber list using a mutex.

### Blocking behavior

`pop()` blocks until a message is available.

### Subscriber behavior

Each subscriber receives its own message queue.

### Unsubscribe behavior

Removing a subscriber prevents future publications from reaching it.

It does not clear its queue or wake a thread already waiting inside `pop()`.

### Shutdown behavior

The current implementation does not provide:

- Queue closing.
- Cancellation of blocked consumers.
- Automatic thread shutdown.
- Subscriber cleanup when a consumer exits.

The `"EXIT"` message in the example is a simple application-level shutdown signal.

For a production implementation, consider adding an explicit queue shutdown mechanism.

---

## 13. API Reference

| Method | Description |
|---|---|
| `subscribe()` | Creates a subscriber queue. |
| `publish(value)` | Sends a message to all subscribers. |
| `pop()` | Blocks until a message is available. |
| `unsubscribe(sub)` | Removes a subscriber from the stream. |
| `push(value)` | Adds a message directly to a queue. |

### Basic Usage

```cpp
EventsBus::streams::Stream<T> stream;

auto subscriber = stream.subscribe();

stream.publish(value);

T message = subscriber->pop();

stream.unsubscribe(subscriber);
```

This is the basic lifecycle of an EventBus subscriber.
