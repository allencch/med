# Med (Memory Editor)

There was a memory editor in Windows, that was Game Master. But it was not a freeware. And there is a freeware memory editor, it is ArtMoney. But it is also for Windows only. In Linux, there is only one memory editor, **scanmem** with GameConqueror as the GUI. However, it does not fulfil my needs. Thus, I decided to create one which can fit my needs.

Med is still in the development, it is not stable. Anyone can help to improve the program.

![Memory editing](http://i.imgur.com/6gSR0WI.png)


## Usage

`med-ui` is a GUI memory editor. In order to use it, please use **sudo**.

Before scanning or opening a JSON file, one must select a target process.


## Interface

![UI](https://i.imgur.com/CGvLdwp.png)

The interface can briefly separated into two panes and a bottom "Named Scans".

* Left pane is the result from the scan.
* Right pane is the memory addresses that we intended to store and save, or open from the file.
* Bottom area is allows manage the named scans. We can temporary save the scan results according to the target named scan.


## Scanning & filtering

1. Before scanning, please click "Process" button to choose the process that we want to scan for the memory.
2. After choosing the process, you can type in the value that you want to **scan**. (For the current stage, the only data types allowed are int8, int16, int32, float32, float64, and string.) For example, we can scan for the gold amount.
3. After we make some changes of the gold in the game, you can **filter** it.

## Last digit

The small field besides the scan value input is the "Last Digit" of the target address.
Some games may have consistent last digit like 0x12341230, where the "0" is the last digit.
The allowed values are "0" to "f" (case insensitive).
Other than that the value will be ignored.

## Fast scan

The Option "Fast Scan" is to allow faster scanning based on scan type.
Let's say we are scanning for "int32", which is 4 bytes long.
The scanning will target on the addresses with the last digit 0, 4, 8, c instead of 0 to f for every byte.
Disable it if willing to do an exhaustive scanning.

## Manage address

The scanned or stored memory addresses, you can

* edit the data type.
* edit the value.

At the right pane, you can

* Use menu to add **new** memory address and edit manually.
* When you select (highlight) a row, you can use `Next` or `Previous` to create next/previous memory address based on the row you selected.
* **delete** the selected memory address with `DEL` key.

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


## Scan by array

Let's say we know a hero has the attributes like Max HP, HP, Max MP, and MP, with each 16-bits (2 bytes), then we can scan by array choosing `int16` and enter the values with comma,

`3000, 2580, 1500, 1500`

where the Max HP is 3000, current HP is 2580, Max MP is 1500, and current MP is 1500.

NOTE: There is a known issue. If the array to be scan involves two `pages` (4096 bytes), then the array will not be found.

## Scan by operators

There are several operators can be used for scanning,

* `=` - equal, default
* `>` - greater than
* `>=` - greater than or equal to
* `<` - less than
* `<=` - less than or equal to
* `!` - not
* `<>` - between (inclusive)
* `~` - around

For `<>`, it requires two operands, example `<> 10 20`, which means scan for the value in the range of [10, 20].
This feature is useful to search for the value such as floating point (float or double) that contains decimal places which is not shown in the game, such as Forager.

For `~`, it requires one operand with second optional operand, example `~ 10`, which will scan for the value [9, 11]. It is translated as 10 ± 1. If the input is `~ 10 2`, it will be 10 ± 2, then will search for the value [8, 12]. This is useful to search the floating point.


## Save/open file

The JSON file is used. Please save the file in the JSON extension.


## Memory Editor

We can view and edit the memory of a process as hexadecimal values.

1. Go to menu Address > Editor. You should get a popup window.
2. Get the memory address you are interested with, eg: 0x7ffdb8979b90.
3. Paste the address to the **Base** field in the popup window.
4. When your cursor is away from the **Base** field, the hex data should be displayed as in the screenshot.

In the memory editor, **Base** field is the base address of the memory that we are interested.
**Cursor** field is the memory address according to the cursor that is moving.
**Value** is currently read-only value of the cursor.
Left pane is the memory address.
Middle pane is the hex reprensentation of the memory. We can directly make the changes to the memory of the process.
Right pane is the ASCII representation of the memory. It is useful for viewing the string.


## Encoding

Menu View > Encoding allows to change the encoding that we want to read and scan.
It will affect the Memory Editor as well.
Currently only support Big5 where the Default is actually UTF8.

### Usage

For example, if a game uses Big5 encoding, we can change the encoding to Big5 and search the text like "臺灣" (Traditional Chinese).

Note: Qt5 application run as root doesn't support IME like Fcitx. Please use copy-paste instead.


## Search for unknown value (experimental v2)

If we are interested on a value of the game, but it is not a numerical value, such as a hero of the game is poisoned or normal. We can use unknown search.

1. It requires at least one address in the store (the right panel) for heuristic search.
2. Enter "?", and press "Scan" button. You should get the statusbar showing "Snapshot saved".
3. Now make the changes of the status, like from poisoned to normal or vice versa.
4. Enter "!" which indicates "changed", and press "Filter" button.
5. The scanner will scan for the memory address with the value changed.
6. Continue to filter until you get the potential memory address that handles the status.

Other operators are ">" and "<".

* ">" with "Filter" will scan for the value that is increased.
* "<" with "Filter" will scan for the value that is decreased.

Notes: This feature is tested on Dosbox game.

## Scope search

To reduce the search space, we can specify the scope start and scope end by entering address in hexadecimal format. After entering both fields, make sure your cursor leave the field, so that the scope will take affect.

## Custom search (experimental, Good!)

Custom search allows to search string and wildcard. To do custom search, one can choose the `custom` type for searching. Then using the following input

```
s:'1', w:3, s:'2'
```

which will look for hexadecimal pattern `31 xx xx xx 32`. Where `s:` is the string to search, and `w:` is the number of wildcard.

Other scan types are supported: `i8`, `i16`, `i32`, `i64`, `f32`, and `f64`.
Besides that, operator such as `~`, `<>`, `>`, `<`, `>=`, `<=` are supported as well. Eg `i8:~ 4, f32: 10.5`

Read [here](https://allencch.wordpress.com/2020/05/07/med-experimental-feature/) for the example usage.


# Build Instruction

This program requires **GCC** (C++ compiler) (or **clang**), **Qt5**, **JSONPP**, and **ICU**.

1. In the directory that contains the source code including `CMakeLists.txt`,

```
mkdir build && cd build
cmake ../
make
```

1. To run the GUI, make sure the `*.ui` files are together with the compiled binary files, and enter

`sudo ./med-ui`


# TODO

1. ~~Scan by array.~~
2. ~~Memory editor dialog (view any memory as block).~~
3. ~~Scan by string.~~
4. ~~Scan by changes (scan unknown).~~
5. ~~Scan within memory block range.~~
6. Arithmetic operation with prefix notation.
7. ~~Scan by struct, supports data type syntax~~
8. Scan by pointer(?)

# Developer notes

For the process maps, read `man procfs`. To view maps,

```
sudo cat /proc/[pid]/maps
```

To build with `clang`,

```
export CC=/usr/bin/clang CXX=/usr/bin/clang++
mkdir build && cd build
cmake ..
make
```
