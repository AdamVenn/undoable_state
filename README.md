# undoable_state
A simple snapshot-based undoable container

## Good to know
 - Copies each version of the state.
 - It has no awareness of threads.
 - 64 states by default, giving 63 undos.

## Example Usage
```cpp
#include "undoable_state.hpp"

#include <filesystem>
#include <map>
#include <string>

// The application state will be a simple map of strings
using Model = std::map<std::string, std::string>;
using UndoableStateModel = UndoableState<Model>;

// Classes/function used for the example
class GuiLibraryTable
{
public:
    void updateContent(const Model&) {}
    void failureFlash(const Model&) {}
};
std::vector<std::pair<std::string, std::string>> parseFile(const std::filesystem::path& f);

// The application model will hold the data for a table widget on screen
class ApplicationModel
{
public:
    ApplicationModel(GuiLibraryTable& table) :
        guiTable{ table }
    {
        // You can set callbacks to perform an action after the operations
        model.setOnUndo([&](const Model& m) {
            guiTable.updateContent(m);
        });
        model.setOnRedo([&](const Model& m) {
            guiTable.updateContent(m);
        });
        model.setOnAction([&](const Model& m) {
            guiTable.updateContent(m);
        });
        model.setOnFailedAction([&](const Model& m) {
            guiTable.failureFlash(m);
        });
    }

    std::string getEntry(const std::string& key) const
    {
        // Dereference the model for const access to the current state snapshot
        return model->at(key);

        // Alternatively:
        // return model.getCurrent().at(key);

        // Not allowed (std::map [] not const):
        // return model.getCurrent()[key];
    }

    void modifyEntry(const std::string& updatedKey, const std::string& updatedValue)
    {
        // The default way to modify the state:
        // Take a snapshot, then perform the modification
        model.snapshotAnd([&](Model& m) {
            m[updatedKey] = updatedValue;
            return UndoableStateResult::success;
        });
    }

    void importLists(const std::vector<std::filesystem::path> sources)
    {
        // To group multiple actions together...
        for (const auto& f : sources)
        {
            // Take a snapshot manually
            model.snapshot();

            const auto entries = parseFile(f);
            for (const auto& entry : entries)
            {
                // Then perform the actions
                model.doWithoutSnapshot([&](Model& m) {
                    m.insert(entry);
                    return UndoableStateResult::success;
                });
            }
        }

        // Now you have one undo per file import, instead of per line import
    }

    void undo()
    {
        if (!model.undo())
        {
            // No prior states exist, give visual feedback to the user
            guiTable.failureFlash(*model);
        }

        // The onUndo callback takes care of refreshing the table view
    }

    void redo()
    {
        if (!model.redo())
        {
            // No prior states exist, give visual feedback to the user
            guiTable.failureFlash(*model);
        }

        // The onRedo callback takes care of refreshing the table view
    }

private:
    UndoableStateModel model{};
    GuiLibraryTable& guiTable;
};

```
