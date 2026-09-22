#include <array>
#include <cstddef>
#include <functional>

template <typename ModelType, size_t Capacity = 64>
class UndoableState
{
public:
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

    using Action = std::function<void(ModelType&)>;
    using Callback = std::function<void(const ModelType&)>;

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
    */
    void snapshotAnd(Action undoableAction)
    {
        snapshot();
        undoableAction(buffer[current]);
    }

    /*
    Mutable access to the state.
    Use this if you want your action to not be undoable.
    */
    void doWithoutSnapshot(Action actionWhichCannotBeUndone)
    {
        actionWhichCannotBeUndone(buffer[current]);
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
        onUndo = callback;
    }

    void setOnRedo(Callback callback)
    {
        onRedo = callback;
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
};
