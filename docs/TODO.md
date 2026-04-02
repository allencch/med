# Introduction

I am rewriting the code with the AI.
Old code are backed up as `src_old`, `include_old`, and `CMakeLists.txt_old`.

# Memory editor

Refer to MemoryEditor.cpp

## Editing by character [done]

In the MemoryEditor, when the caret editing the digit, it should reflects in the UI and update the memory. Eg,

```
10 24
^
```

If the caret at `1`, enter `2`, it should becomes `20 24`, and it means `0x20 0x24` in memory.

## Navigation [done]

Navigtion should always skip the whitespace. Eg,

```
10 24
^
```

When I move right once, it should go to `0`. Move right again, should go to `2`.
Furthermore, editing the value should go to next non-whitespace character.

## Navigation up and down [done]

If the cursor navigates up to the first line, the memory should load the previous line and update the UI.
If the cursor navigates down to the last line, the memory should load the next line and update the UI.
So that the cursor can always explore other memory addresses.

## Navigation up and down again

Let's say I am at the line 2.

## Refreshing

When refresh, should should keep the caret at the same position
