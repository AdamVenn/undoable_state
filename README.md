# undoable_state
A simple snapshot-based undoable container

## Example Usage
```cpp
#include "undoable_state.hpp"

#include <filesystem>
#include <map>
#include <string>

// Classes/function used for the example
class guiLibraryTable;
std::vector<std::pair<std::string, std::string>> parseFile(const std::filesystem::path& f);

// The application state will be a simple map of strings
using Model = std::map<std::string, std::string>;
using UndoableModel = UndoableState<Model>;

// The application model will hold the data for a table widget on screen
class ApplicationModel
{
public:
    ApplicationModel(guiLibraryTable& table) :
        guiTable{ table }
    {
        // You can set a callback to perform an action after an undo/redo
        model.setOnUndo([&](const Model& m) {
            guiTable.updateContent(m);
        });
        model.setOnRedo([&](const Model&) {
            guiTable.updateContent(m);
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
            guiTable.failureFlash();
        }

        // The onUndo callback takes care of refreshing the table view
    }

    void redo()
    {
        if (!model.redo())
        {
            // No prior states exist, give visual feedback to the user
            guiTable.failureFlash();
        }

        // The onRedo callback takes care of refreshing the table view
    }

private:
    UndoableModel model{};
    guiLibraryTable& guiTable;
};

```
