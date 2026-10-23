#include "EventBus.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <thread>

/** A simple message type shared by publishers and subscribers. */
struct Event {
    std::string message;
    bool stop;

    Event(const std::string& text, bool should_stop = false)
        : message(text), stop(should_stop) {}
};

/** Consumes events from one subscriber queue until it receives a stop event. */
void consumeEvents(
    const std::string& subscriberName,
    std::shared_ptr<EventsBus::Queue<Event>> subscriber
) {
    while (true) {
        Event event = subscriber->pop();

        if (event.stop) {
            break;
        }

        std::cout << '[' << subscriberName << "] received: "
                  << event.message << '\n';
    }
}

int main() {
    // 1. Create a stream that carries Event objects.
    EventsBus::Stream<Event> stream;

    // 2. Register two independent subscribers.
    auto loggerSubscriber = stream.subscribe();
    auto monitorSubscriber = stream.subscribe();

    // 3. Start one consumer thread for each subscriber.
    std::thread loggerThread(
        consumeEvents, "Logger", loggerSubscriber
    );
    std::thread monitorThread(
        consumeEvents, "Monitor", monitorSubscriber
    );

    // 4. Publish events. Each subscriber receives its own queued copy.
    stream.publish(Event("Runtime started"));
    stream.publish(Event("Function called: calculate_sum"));
    stream.publish(Event("Runtime finished"));

    // 5. Send a stop event so each worker can leave its blocking pop() loop.
    stream.publish(Event("", true));

    // 6. Wait for both consumer threads to finish before cleaning up.
    loggerThread.join();
    monitorThread.join();

    // 7. Unregister the subscribers.
    stream.unsubscribe(loggerSubscriber);
    stream.unsubscribe(monitorSubscriber);

    return 0;
}
