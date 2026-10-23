# C++ EventBus Documentation

This repository contains the EventBus header, a runnable example, and documentation describing how the implementation currently works. The documentation reflects the code as it exists today, including blocking behavior and known limitations.

Start here:

- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — how the pieces fit together: thread-safe queues, streams, subscribers, and message delivery. Read this first.
- [`include/EventBus.hpp`](include/EventBus.hpp) — the header-only implementation.
- [`examples/basic_usage.cpp`](examples/basic_usage.cpp) — a complete example using a custom event type and two consumer threads.

Then browse by area:

## `include/` — public interface

- [`docs/include/EventBus.md`](docs/include/EventBus.md) — API reference for `EventsBus::Queue<T>` and `EventsBus::Stream<T>`, including parameters, return values, concurrency behavior, and limitations.

## `examples/` — practical usage

- [`docs/examples/basic_usage.md`](docs/examples/basic_usage.md) — walkthrough of the example program, from creating a stream to stopping and joining consumer threads.
- [`examples/basic_usage.cpp`](examples/basic_usage.cpp) — complete source code that can be compiled and run.

## Root-level guides

- [`CONTRIBUTION.md`](CONTRIBUTION.md) — guidelines for reporting issues, proposing changes, documenting behavior, and testing contributions.

## Quick start

Requirements: a C++17-compatible compiler and standard-library thread support.

Compile the example from the repository root:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread \
    -Iinclude examples/basic_usage.cpp -o eventbus_example
```

Run it:

```bash
./eventbus_example
```

The order of output from the two consumer threads may vary because they run concurrently. Each subscriber receives its own queued copy of every message published while it is subscribed.

## Current limitations

- `Queue<T>::pop()` blocks while its queue is empty; there is no built-in close, cancellation, or timeout operation.
- `Stream<T>::unsubscribe()` does not wake a consumer blocked in `pop()`.
- Messages are copied into each subscriber queue, so the message type must be copyable for the current `push()` and `publish()` interfaces.
- A publication already in progress may still reach a subscriber that is concurrently unsubscribed, because `publish()` uses a snapshot of the subscriber list.

These behaviors are documented explicitly in the API reference and architecture guide.
