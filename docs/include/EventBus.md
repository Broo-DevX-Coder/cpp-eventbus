# EventBus Header Reference

Source: [`include/EventBus.hpp`](../../include/EventBus.hpp)

The public API is defined in the `EventsBus` namespace. The header provides `Queue<T>` and `Stream<T>` templates.

## `EventsBus::Queue<T>`

A thread-safe FIFO queue for values of type `T`.

### `void push(const T& value)`

Adds a copy of `value` to the back of the queue and notifies one waiting consumer.

- **Thread safety:** internal queue access is protected by a mutex.
- **Copying:** `value` is copied into the queue.
- **Notification:** one thread waiting in `pop()` is notified after insertion.

### `T pop()`

Waits until the queue is non-empty, removes its oldest value, and returns it.

- **Blocking:** this call waits indefinitely if no value is available.
- **Shutdown:** the current queue has no close, timeout, or cancellation mechanism.
- **Return:** the queued value is moved out of the internal queue.

## `EventsBus::Stream<T>`

A broadcaster that creates an independent queue for each subscriber.

### `std::shared_ptr<Queue<T>> subscribe()`

Creates and registers a subscriber queue, then returns a shared pointer to that queue.

New subscribers only receive messages published after they subscribe.

### `bool unsubscribe(const std::shared_ptr<Queue<T>>& subscriber)`

Removes the specified subscriber from the stream's subscriber list.

- Returns `true` when the subscriber was found and removed.
- Returns `false` when the subscriber was not registered.
- Messages already queued remain accessible through the shared pointer.
- This operation does not wake a consumer blocked in `pop()`.

### `void publish(const T& value)`

Copies `value` into each subscriber queue contained in a snapshot of the subscriber list.

- Each subscriber gets an independent queued copy.
- Delivery to subscribers is sequential, not simultaneous.
- A publish operation that has already taken its snapshot may still deliver to a subscriber concurrently removed by `unsubscribe()`.

## Minimal example

```cpp
#include "EventBus.hpp"
#include <iostream>
#include <string>

int main() {
    EventsBus::Stream<std::string> stream;
    auto subscriber = stream.subscribe();

    stream.publish("Hello, EventBus!");
    std::cout << subscriber->pop() << '\n';

    stream.unsubscribe(subscriber);
}
```

Compile with C++17 or newer:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread \
    -Iinclude minimal.cpp -o minimal
```

## Export macro

The header defines an `API` macro controlled by `BUILDING_COMPILER_DLL` and `_WIN32`. In the current code, the macro is not applied to `Queue<T>` or `Stream<T>`. Since these are templates defined in the header, consumers typically include the header directly. Review this macro if you later change the library's shared-library packaging model.
