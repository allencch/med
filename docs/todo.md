# Introduction

I am rewriting the code with the AI.
Old code are backed up as `src_old`, `include_old`, and `CMakeLists.txt_old`.

---

# Task: Memory Editor - Character Editing

Refer to MemoryEditor.cpp

## Status
Done

## Goal

In the MemoryEditor, when the caret editing the digit, it should reflects in the UI and update the memory. Eg,

## Behaviour

```
10 24
^
```

If the caret at `1`, enter `2`, it should becomes `20 24`, and it means `0x20 0x24` in memory.

## Constraints
- Must go through worker thread (no direct memory write)
- UI must not block

## Notes
- Format: "10 24" → maps to bytes [0x10, 0x24]

---

# Task: Memory Editor - Navigation

Refer to MemoryEditor.cpp

## Status
Done

## Goal

Navigtion should always skip the whitespace. Eg,

```
10 24
^
```

When I move right once, it should go to `0`. Move right again, should go to `2`.
Furthermore, editing the value should go to next non-whitespace character.

## Navigation up and down

If the cursor navigates up to the first line, the memory should load the previous line and update the UI.
If the cursor navigates down to the last line, the memory should load the next line and update the UI.
So that the cursor can always explore other memory addresses.


# Task: Memory Editor - Value update

Refer to MemoryEditor.cpp

## Status
Done

## Goal

After edit the value in the memArea, the `viewInt32_`, `viewFloat32_`, `viewFlaot64_`, and `valueEdit_` should have the value from the memory.

# Task: Scanner - Filter function

Refer to MainWindow.cpp

## Status
Done

## Goal

The filter button should filter the list of the scan area.

## Behaviour

When filter, it just filter based on the addresses int the scan area.
Meaning, if the filter change the type from int32 to float32, it should still work.


# Task: Encoding - Change encoding to Big5

Refer to old EncodingManager.cpp

## Status
Done

## Goal
Allows to change encoding to Big5 (as toggle)


# Task: Named scan

Refer to old NamedScans.cpp and NamedScansController.cpp

## Status
In progress

## Goal
Allows named scan, so that we can store the scan result temporary in the memory


# Task: File save

## Status
Done

## Goal
Save the store data into JSON file.
Also, the save file should be loadable, and Ctrl+R allows to reload the save file as an approch to revert the changes.
