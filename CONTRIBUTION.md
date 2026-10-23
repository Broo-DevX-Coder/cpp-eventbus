# Contribution Guide

Thank you for considering a contribution to C++ EventBus. Contributions can include bug reports, documentation fixes, example improvements, tests, and changes to the implementation.

## Before making a change

1. Read [`README.md`](README.md) for the repository overview and build command.
2. Read [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) to understand the current design and known limitations.
3. Check whether an issue or discussion already describes the problem you want to address.

## Reporting a bug

When opening an issue, include:

- A short, descriptive title.
- Your operating system, compiler, compiler version, and C++ standard.
- The smallest code sample that reproduces the problem.
- The exact build/run command and the output or error message.
- What you expected to happen and what happened instead.

Do not include unrelated application code or secrets in a reproducer.

## Proposing a change

1. Create a branch for your change.
2. Keep the change focused on one bug, feature, or documentation improvement.
3. Preserve the existing public API unless the change specifically requires an API update.
4. Update the relevant documentation whenever observable behavior or API behavior changes.
5. Add or update an example when it makes a new behavior easier to understand.
6. Open a pull request describing the motivation, implementation, and any compatibility implications.

## Code conventions

- Use clear, descriptive names.
- Keep public API documentation in Doxygen style, including `@brief`, `@tparam`, `@param`, `@return`, `@note`, and `@warning` where relevant.
- Make concurrency behavior explicit. Document blocking calls, lock scope, event delivery semantics, and shutdown behavior.
- Avoid claiming stronger guarantees than the implementation provides.
- Keep examples small and compilable with the stated compiler standard.

## Validation

Compile and run the example from the repository root before submitting a change:

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -pthread \
    -Iinclude examples/basic_usage.cpp -o eventbus_example
./eventbus_example
```

When changing concurrency behavior, also review the implementation for data races, lock ordering, missed wake-ups, and thread shutdown. The repository currently has no dedicated automated test suite, so successful compilation of the example alone should not be treated as complete concurrency validation.

## Documentation updates

If you add, remove, or rename source files or documentation pages, update the links and file index in [`README.md`](README.md). Document known limitations honestly rather than describing intended or planned behavior as if it already exists.

## License and contribution terms

No license file or formal contributor agreement is currently included in this repository. Before accepting external code contributions or distributing the project, the maintainer should add an appropriate `LICENSE` file and clarify the terms under which contributions may be used.
