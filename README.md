# Med (Memory Editor)

There was a memory editor in Windows, that was Game Master. But it was not a freeware. And there is a freeware memory editor, it is ArtMoney. But it is also for Windows only. In Linux, there is only one memory editor, scanmem with GameConqueror as the GUI. However, there is a lot of limitation from scanmem and GameConqueror. Thus, I decided to create one which can fit my needs.

Med is still in the development, it is not stable. Anyone can help to improve the program.

![Memory editing](http://i.imgur.com/6gSR0WI.png)


## Usage

`med-qt` is a GUI memory editor. In order to use it, please use **sudo**.

Before scanning or opening a JSON file, one must select a target process.


## Interface

The interface can briefly separated into two panes. Left pane is the result from the scan; right pane is the memory addresses that we intended to store and save, or open from the file.


## Scanning & filtering

1. Before scanning, please click "Process" button to choose the process that we want to scan for the memory.
2. After choosing the process, you can type in the value that you want to **scan**. (For the current stage, the only data types allowed are int8, int16, int32, float32, and float64.) For example, we can scan for the gold amount.
3. After we make some changes in the game, you can **filter** it.

## Manage address

The scanned or stored memory addresses, you can

1. edit the data type.
2. edit the value.

At the right pane, you can

1. Use menu to add **new** memory address and edit manually.
2. When you select (highlight) a row, you can use `Next` or `Previous` to create next/previous memory address based on the row you selected.
3. **delete** the selected memory address with `DEL` key.

## Shifting memory address

Memory are usually dynamically allocated, the memory address will change whenever you start a process. Therefore, we need to shift our saved memory to the new location.

In order to solve this problem, two input fields: `Shift from` and `Shift to / byte` are provided. And three buttons `Shift`, `Unshift`, and `Move` works with the fields.


### Example 1

For example, one of the item, namely Gold, memory address that you stored is 0x20de9b94. After you restart the game, the memory address you scan is changed to 0x20c3cb80. 

1. In order to shift the memory, copy-paste 0x20de9b94 to the `Shift from` and 0x20c3cb80 to the `Shift to / byte`. 
2. Select the memory address (the Gold) that you want to shift. Multiple selection is allowed.
3. Press `Shift` button.
4. Then all your selected address will be shifted.

`Unshift` is a reverse of `Shift`.

### Example 2

Similar to `Shift` and `Unshift`, let's say you have first character HP memory address located at 0x20de9b90, and the second character HP is located at 0x20de9ba2. Use a calculator that supports hexadecimal, then we can get the difference of 18 bytes.

If you have the memory addresses like HP, MP, strength, wisdom, agility, etc of the first character, then you can move these addresses to the second character location.

1. Fill the 18 at `Shift to / bytes`.
2. Select the rows that we want to move.
3. Press `Move` button.

If we want to move back, fill in with negative value, and press move.


## Save/open file

The JSON file is used. Please save the file in the JSON extension.


# Build Instruction

This program requires **GCC** (C++ compiler), **Qt5**, and **JSONPP**.

1. In the directory that contains the source code including `CMakeLists.txt`,

```
mkdir build && cd build  
cmake ../  
make  
```

1. To run the GUI, make sure the `*.ui` files are together with the compiled binary files, and enter

`sudo ./med-qt`


# TODO

1. Scan by array.
2. Memory editor dialog (view any memory as block).
3. Scan within memory block range.
4. Arithmetic operation with prefix notation.
5. Scan by struct, supports data type syntax
6. Scan by string.
7. Scan by changes.
8. Scan by pointer(?)

