# Architect Plan: `med-rewrite` Project

## 1. Project Overview
`med-rewrite` is a memory editor/scanner tool designed for process memory manipulation. It includes both a CLI and a Qt-based UI, providing features like memory scanning, editing, and process management.

## 2. Current Status & Motivation for Rewrite
The current codebase suffers from a difficult-to-reproduce deadlock. The decision has been made to rewrite the project to:
- Resolve concurrency and synchronization issues (deadlocks).
- Improve codebase maintainability and readability.
- Modernize the architecture and ensure robust error handling.

## 3. Rewrite Goals
- **Stability:** Eliminate deadlocks and race conditions through clear thread ownership and synchronization patterns.
- **Modularity:** Decouple the core logic (memory scanning, process interaction) from the UI and CLI layers.
- **Testability:** Ensure high test coverage for core components.
- **Performance:** Maintain or improve scanning and memory operation efficiency.


# Project Contract (STRICT RULES)

## Threading Model
- UI thread: UI only
- Worker QThread: ALL memory operations
- No shared mutex between UI and worker

## Communication
- Qt::QueuedConnection ONLY
- No direct cross-thread calls

## Ownership
- Worker owns memory operations
- UI never holds raw memory buffers

## Forbidden
- UI calling core directly
- std::thread touching Qt
- Blocking UI thread

## Refresh Model
- QTimer inside worker thread
- No separate refresh thread

## Compilation
- Use `clang`, as `CC=/usr/bin/clang CXX=/usr/bin/clang++` to run `cmake`


# Architecture


(This proposal is extracted from the conversation with Claude.)


┌─────────────────────────────────────┐
│           Qt UI Layer               │  QWidgets, QML, signals/slots
├─────────────────────────────────────┤
│         Qt Adapter Layer            │  QThread, QObject wrappers, model classes
├─────────────────────────────────────┤
│       Pure C++ Core (no Qt)         │  Memory scanning, process I/O, data structures
└─────────────────────────────────────┘

---

UI Thread                    Worker QThread
─────────────────            ──────────────────────────
User clicks Scan
  → emit scanRequested()  →  slot: runScan()
                               core.scan(params)  // pure C++
                             ← emit scanComplete(results)
  ← slot: onScanComplete()
     model.setResults()
     tableView.update()


All cross-thread communication goes through Qt::QueuedConnection signals. The core never touches Qt. The worker thread never touches widgets. This architecture makes the deadlock class of bug structurally much harder to write by accident — there is no shared mutex between UI and worker, only message passing through the Qt event loop.

---

Use QThread with worker QObjects exclusively. Use std::thread only inside the pure C++ core, never touching anything Qt.

Pure C++ core          →   may use std::thread internally
Qt adapter/UI layer    →   QThread + QObject only, no std::mutex on shared state

---

UI Thread                          Worker QThread
─────────────────────              ──────────────────
User edits field, presses Write
  → emit writeRequested(addr, val) → slot: onWriteRequested()
                                        core.writeMemory(addr, val)
                                     ← emit writeComplete(success)
  ← slot: onWriteComplete()
      show status indicator

emit writeRequested(...) with a Qt::QueuedConnection returns immediately — the UI thread is never blocked. The worker handles it on its own thread at its own pace. No locks cross thread boundaries because no locks are needed — ownership of the task transfers via the signal queue.


# Code Map

## Core
- Process
- MemIO
- MemScanner

## Adapter (Qt)
- Worker (QObject)
- Thread manager

## UI
- MainWindow
- MemoryEditor


# Roadmap

## Essential rewrite

### Phase 1: Core Foundation & Memory IO
- [x] Define core data structures (`MedTypes`, `SizedBytes`).
- [x] Implement robust `Process` and `MemIO` layers.
- [x] Unit tests for core memory operations.

### Phase 2: Scanning Engine
- [x] Rewrite `MemScanner` and `ScanParser`.
- [x] Implement thread-safe `ThreadPool` for parallel scanning.
- [x] Integration tests for scanning logic.

### Phase 3: CLI & Command Layer
- [ ] Rebuild `ScanCommand` and `SubCommand` infrastructure.
- [ ] CLI main loop and user interaction. (Skip CLI, can be ignored)

### Phase 4: UI Layer (Qt)
- [x] Port/Rewrite Qt UI components (MainWindow, ProcessDialog).
- [x] Connect UI to the new core logic via an event/listener (Worker QObject) pattern.
- [x] Implement File IO (JSON Save/Load) and Notes support.
- [x] Implement Address locking and periodic refreshing.
- [x] Stress testing and deadlock verification (via architecture).

## Verification & Validation
- Continuous integration with existing and new tests.
- Targeted stress tests for concurrency.
- Manual verification of memory editing workflows.

## Extra

- [x] Should follow the old code for the initialize state after window is show
  + [x] Note is hidden
  + [x] Default type is int32
- [x] Scan area should refresh like Store area
- [x] Scan area value should be editable
- [x] Scan and Store area, the Type should be editable using dropdown, follow old code
- [x] Process list should by reverse order. The process string contains box-like whitespace, including the end of line.
- [x] Process list should allow Enter key to select
- [x] Hotkey like Ctrl-Q should quit. Other hotkeys should follow the old code. (Connected basic ones)
- [x] Address > Editor shouled trigger the memory editor
- [x] Scan area refresh whenever there is value, no need depends on Store area
- [x] Read/write JSON file
  + [x] Window title should show the JSON file name, `mainWindow->setWindowTitle(MAIN_TITLE + ": " + filename)`
- [x] Scan (last digit, allows comma separated)
- [x] Scan area and store area should allow multiple rows selection
- [x] Store area should be able to delete the row by Delete key
- [x] Scan area listing, the Address, Type, and Value should be editable as well
- [ ] ICU encoding, Big5
- [ ] Named scan

## Memory editor

- [x] The cursor should be able to update the Hex area.
- [x] The Enter value should fill in the value.

