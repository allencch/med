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

## 4. Proposed Solution & Architecture
*(User to fill in specific solution details here)*

## 5. Rewrite Roadmap

### Phase 1: Core Foundation & Memory IO
- [ ] Define core data structures (`MedTypes`, `SizedBytes`).
- [ ] Implement robust `Process` and `MemIO` layers.
- [ ] Unit tests for core memory operations.

### Phase 2: Scanning Engine
- [ ] Rewrite `MemScanner` and `ScanParser`.
- [ ] Implement thread-safe `ThreadManager` for parallel scanning.
- [ ] Integration tests for scanning logic.

### Phase 3: CLI & Command Layer
- [ ] Rebuild `ScanCommand` and `SubCommand` infrastructure.
- [ ] CLI main loop and user interaction.

### Phase 4: UI Layer (Qt)
- [ ] Port/Rewrite Qt UI components.
- [ ] Connect UI to the new core logic via an event/listener pattern.
- [ ] Stress testing and deadlock verification.

## 6. Verification & Validation
- Continuous integration with existing and new tests.
- Targeted stress tests for concurrency.
- Manual verification of memory editing workflows.


# Proposal for rewrite for resolving deadlock

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

---

Concrete rules to follow
1. No shared mutexes between UI thread and worker thread.
If you find yourself writing mutex.lock() in a widget slot, that's a red flag.
2. All cross-thread calls go through queued signals.
Worker → UI: emit resultReady(data) — Qt serialises it through the event loop.
UI → Worker: emit writeRequested(addr, val) — same.
3. The worker owns all memory-touching operations.
Reads, writes, scans — all happen in the worker. The UI only sends commands and receives results.
4. Use QMetaObject::invokeMethod with Qt::QueuedConnection if you ever need to call a method cross-thread without a direct signal connection.


## Refresh thread (existing feature)

Problem

The problem you have now
Refresh Thread                   UI Thread
──────────────                   ─────────────────
loop:
  acquire Lock (read value)      user edits field
                                   acquire Lock (write value)
                                   → blocks, refresh holds it
  → try to update UI
    → Qt internally needs
      UI thread to be free
      → DEADLOCK


The fix: make the refresh thread a worker too
The refresh thread should not be a std::thread that touches Qt. It should be a QTimer living on the same worker thread as your write operations.
Worker Thread (single QThread)
──────────────────────────────
QTimer fires every 500ms
  → onRefreshTick()
      values = core.readWatchedValues()
  → emit valuesRefreshed(values)

UI Thread
──────────────────────────────
onValuesRefreshed(values)
  → update table/fields

user edits field
  → emit writeRequested(addr, val)

Worker Thread
──────────────────────────────
onWriteRequested(addr, val)
  → core.writeMemory(addr, val)
  → emit writeComplete(ok)


---

The refresh thread is not a separate thread problem — it's a **serialisation** problem.

In your current code, refresh and write run concurrently and fight over a lock. The fix is not a better lock — it's making them run **sequentially on the same thread**, which `QTimer` + `QThread` gives you for free via the event loop.
```
Before:  RefreshThread ──┐
                         ├── fight over lock ── deadlock
         UI Thread ──────┘

After:   Worker QThread: [refresh tick] [write] [refresh tick] [refresh tick]
         UI Thread:      send signals, receive results, never blocks
