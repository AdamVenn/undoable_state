#include "undoable_state.hpp"
#include <cstddef>
#include <print>

struct TestState
{
    int x{ 0 };
    int y{ 0 };

    bool operator<=>(const TestState& other) const = default;
};

int test_stack()
{
    UndoableState<TestState> state;

    TestState blankState{};

    if (state.numAvailableUndos() != 0)
        return -1;

    if (state.getCurrent() != blankState)
        return -1;

    if (state.canUndo())
        return -1;

    if (state.canRedo())
        return -1;

    // Fill the undo buffer
    for (int i{ 1 }; i < 63; ++i)
    {
        state.snapshotAnd([i](TestState& s) { s.x = i; });

        if (state.numAvailableUndos() != std::min(static_cast<size_t>(i), 62ul))
            return -1;

        if (state.getCurrent() != TestState{ i, 0 })
            return -1;

        if (!state.canUndo())
            return -1;

        if (state.canRedo())
            return -1;
    }

    state.snapshotAnd([](TestState& s) { s.x = 64; });

    // At this point, tail has advanced one, since we filled the buffer.

    if (state.getCurrent() != TestState{ 64, 0 })
        return -1;

    if (state.numAvailableUndos() != 62ul)
        return -1;

    if (!state.canUndo())
        return -1;

    if (state.canRedo())
        return -1;

    // Undo back to the bottom of the stack
    for (auto i = 62u; i > 0; --i)
    {
        auto result = state.undo();

        if (!result)
            return -1;

        if (state->x != static_cast<int>(i))
            return -1;

        if (!state.canRedo())
            return -1;

        if (state.numAvailableRedos() != 63u - i)
            return -1;

        if (state.numAvailableUndos() != i - 1u)
            return -1;
    }

    // Now at the start of recorded history
    {
        if (state.canUndo())
            return -1;

        auto result = state.undo();

        if (result)
            return -1;

        if (!state.canRedo())
            return -1;

        if (state.numAvailableRedos() != 62ul)
            return -1;

        if (state.numAvailableUndos() != 0ul)
            return -1;

        if (state->x != 1)
            return -1;
    }

    // Redo back to the present
    for (auto i = 1ul; i < 62; ++i)
    {
        auto result = state.redo();

        if (!result)
            return -1;

        if (!state.canUndo())
            return -1;

        if (!state.canRedo())
            return -1;

        if (state.numAvailableRedos() != 62u - i)
            return -1;

        if (state.numAvailableUndos() != i)
            return -1;
    }

    {
        auto result = state.redo();

        if (!result)
            return -1;

        if (!state.canUndo())
            return -1;

        if (state.canRedo())
            return -1;

        if (state.numAvailableRedos() != 0ul)
            return -1;

        if (state.numAvailableUndos() != 62ul)
            return -1;
    }

    // Now at the head of the buffer
    {
        auto result = state.redo();

        if (result)
            return -1;

        if (!state.canUndo())
            return -1;

        if (state.canRedo())
            return -1;

        if (state.numAvailableRedos() != 0ul)
            return -1;

        if (state.numAvailableUndos() != 62ul)
            return -1;
    }

    // Check you cannot redo after snapshot
    for (int i{ 0 }; i < 10; ++i)
    {
        state.undo();
    }

    if (!state.canRedo())
        return -1;

    state.snapshotAnd([](TestState& s) { s.y = 5; });

    if (!state.canUndo())
        return -1;

    if (state.canRedo())
        return -1;

    if (state.numAvailableUndos() != 53)
        return -1;

    if (state.numAvailableRedos() != 0)
        return -1;

    return 0;
}

int main()
{
    return test_stack();
}