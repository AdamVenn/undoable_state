#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

/* The result of action lambdas passed to the undoable state
Use this to get error messages back from your action lambdas. */
using UndoableStateResult = std::expected<void, std::string>;

/* The model contained by the undoable state needs to be moved and default-initialized */
template <typename T>
concept UndoableModel = std::default_initializable<T>
    && std::copy_constructible<T>
    && std::is_nothrow_move_assignable_v<T>;

template <UndoableModel ModelType, std::size_t Capacity = 64>
class UndoableState
{
public:
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
    static_assert(Capacity != 0, "Capacity must not be 0");

    using Action = std::function<UndoableStateResult(ModelType&)>;
    using Callback = std::function<void(const ModelType&)>;
    using CallbackWithError = std::function<void(const ModelType&, const std::string& errorMessage)>;

    /* Take a snapshot. To do an action, use snapshotAnd. */
    void snapshot()
    {
        current = next(current);
        if (current == tail)
            tail = next(tail);
        head = current;
        buffer[current] = buffer[prev(current)];
    }

    /* Const access to the state */
    [[nodiscard]] const ModelType& getCurrent() const
    {
        return buffer[current];
    }

    /* Const access to the state */
    [[nodiscard]] const ModelType* operator->() const
    {
        return &buffer[current];
    }

    /* Const access to the state */
    [[nodiscard]] const ModelType& operator*() const
    {
        return buffer[current];
    }

    /*
    Take a snapshot and do the action with the state.
    Use this if you want your action to be undoable.
    If the action fails, the undo buffer will be unaffected and
    onFailedAction will be called.
    */
    void snapshotAnd(Action undoableAction)
    {
        ModelType working{ buffer[current] };

        if (auto res = undoableAction(working); res)
        {
            snapshot();
            buffer[current] = std::move(working);

            if (onAction)
                onAction(buffer[current]);
        }
        else
        {
            if (onFailedAction)
                onFailedAction(buffer[current], res.error());
            return;
        }
    }

    /*
    Mutable access to the state.
    Use this if you want your action to not be undoable.
    */
    void doWithoutSnapshot(Action actionWhichCannotBeUndone)
    {
        ModelType working{ buffer[current] };

        if (auto res = actionWhichCannotBeUndone(working); res)
        {
            buffer[current] = std::move(working);

            if (onAction)
                onAction(buffer[current]);
        }
        else
        {
            if (onFailedAction)
                onFailedAction(buffer[current], res.error());
            return;
        }
    }

    /* Revert to the previous state if available. */
    bool undo()
    {
        if (!canUndo())
            return false;
        current = prev(current);
        if (onUndo)
            onUndo(buffer[current]);
        return true;
    }

    /* Go forward to the next state if available. */
    bool redo()
    {
        if (!canRedo())
            return false;
        current = next(current);
        if (onRedo)
            onRedo(buffer[current]);
        return true;
    }

    /* Clears the undo stack and sets the model to the one provided.
    If zeroOut is false, stale entries remain in the buffer and keep any resources
    they own (e.g. shared_ptr) alive until overwritten. */
    void reset(const ModelType& initial = {}, bool zeroOut = true)
    {
        current = 0;
        head = 0;
        tail = Capacity - 1;

        if (zeroOut)
            std::fill(buffer.begin(), buffer.end(), ModelType{});

        buffer[current] = initial;
    }

    [[nodiscard]] bool canUndo() const
    {
        return current != next(tail);
    }

    [[nodiscard]] bool canRedo() const
    {
        return current != head;
    }

    /* Check the size of the undo stack. */
    [[nodiscard]] size_t numAvailableUndos() const
    {
        return (current - tail - 1 + Capacity) & (Capacity - 1);
    }

    /* Check how many undos have been performed. */
    [[nodiscard]] size_t numAvailableRedos() const
    {
        return (head - current + Capacity) & (Capacity - 1);
    }

    /* Check how many usable states are current stored. */
    [[nodiscard]] size_t numStatesStoredTotal() const
    {
        return (head - tail + Capacity) & (Capacity - 1);
    }

    void setOnUndo(Callback callback)
    {
        onUndo = std::move(callback);
    }

    void setOnRedo(Callback callback)
    {
        onRedo = std::move(callback);
    }

    void setOnAction(Callback callback)
    {
        onAction = std::move(callback);
    }

    void setOnFailedAction(CallbackWithError callback)
    {
        onFailedAction = std::move(callback);
    }

private:
    size_t prev(size_t index) const
    {
        return (index - 1) & (Capacity - 1);
    }

    size_t next(size_t index) const
    {
        return (index + 1) & (Capacity - 1);
    }

    std::array<ModelType, Capacity> buffer;
    size_t current{ 0 };
    size_t head{ 0 };
    size_t tail{ Capacity - 1 };
    Callback onUndo;
    Callback onRedo;
    Callback onAction;
    CallbackWithError onFailedAction;
};
