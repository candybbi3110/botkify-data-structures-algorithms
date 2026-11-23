# BotKify — One App, Three Data Structures

A music-streaming playlist app written in C++, rebuilt three times. Each version swaps the core data structure for a more capable one:

| Stage | Data structure | What it enables | Score |
|---|---|---|---|
| [A1](a1-linked-list/) | Doubly linked list | Playlist with next/previous playback and score analytics | 9.44 / 10 |
| [A2](a2-threaded-avl/) | AVL tree + **Threaded** AVL | Songs kept sorted by title; next/previous in O(1) through in-order threads | 9.64 / 10 |
| [A3](a3-music-graph/) | Weighted directed graph | Recommendations (BFS), playlist clustering, smoothest transition (Dijkstra), hub song, loop detection (DFS) | 9.00 / 10 |

> Individual assignments for *Data Structures and Algorithms (CO2003)*, HCMUT, Semester 252 (Spring 2026). All data structures are implemented from scratch.

---

## A1 — Linked list

`BotkifyLinkedList<T>` is a templated list with `head`/`tail`. The spare `extra` pointer is used as a back-link, which makes it doubly linked, so `playPrevious()` is O(1). It includes a deep-copy constructor, copy assignment, and a destructor that frees every node.

The analytics go beyond brute force:

| Method | Problem | Approach | Complexity |
|---|---|---|---|
| `getTotalScore` | Σ over all contiguous groups of `min(group) × sum(group)` | Monotonic stack (each song's range as the minimum) + prefix sums of prefix sums | **O(n)** (brute force O(n²)) |
| `compareTo` | Average of the maximum score over every window of size k | Sliding-window maximum with a monotonic deque | **O(n)** |
| `playApproximate` | Minimum total duration change from first to last song, skipping ≤ `step` songs | Dynamic programming | O(n · step) |
| `playRandom` | Chain of nearest songs with a longer duration | Direct scan | O(n) per step |

## A2 — AVL tree and Threaded AVL tree

- `AVL<K,V>` implements the `IBST` interface: insert/erase with single and double rotations, plus hook methods (`onInserted`, `onErasing`, `onReplaceBySuccessor`) that a subclass can override.
- `ThreadedAVL<K,V>` extends it through those hooks. Each node carries `prev`/`next` pointers that link all nodes in sorted order, so the tree is also a doubly linked list. The predecessor and successor are tracked on the way down during insert, and the links are repaired on erase, including the case where a node with two children is replaced by its successor.
- `Playlist` orders songs by `(title, id)` and plays next/previous through a threaded-tree `Iterator` in O(1).
- Build with `-DUSE_THREADED_AVL` to switch the playlist to the threaded version.

## A3 — Music graph

The graph is an adjacency list, and the course required linear-search lookups and hand-made auxiliary structures (no STL queue, map or priority_queue).

| Feature | Algorithm |
|---|---|
| Recommend related songs | BFS with a vector + head-pointer queue |
| Playlists by cluster | Connected components on the graph treated as undirected (follows incoming and outgoing edges) |
| Smoothest transition | Dijkstra with an O(V²) array scan, path rebuilt from a predecessor array |
| Network hub song | Maximum in-degree |
| Music loop | DFS with a recursion stack; prints the cycle itself |

---

## Testing

```bash
cmake -B build -DSANITIZE=ON      # AddressSanitizer + UBSan (or omit for a normal build)
cmake --build build
cd build && ctest --output-on-failure
```

| Test | What it checks |
|---|---|
| `a1_linked_list` | 38 checks, including fuzz tests against brute-force reference implementations for every analytics method |
| `a2_avl`, `a2_threaded` | Course sample cases in both build modes, compared with `expected_output.txt` |
| `a2_property_test` | 200 rounds × 300 random inserts/erases checked against `std::set`: BST order, balance factors in {−1, 0, 1}, `prev`/`next` threads, forward/backward iterators |
| `a3_demo` | All sample cases from the assignment, compared with `expected_output.txt` |
| `a3_property_test` | 300 random graphs: Dijkstra vs Bellman–Ford, loop detection vs Kahn's algorithm, cluster count vs union–find, hub in-degree vs direct count |

All 6 tests pass under AddressSanitizer + UndefinedBehaviorSanitizer. The property tests and the build setup were added for this repository. The assignment code is unchanged from what was submitted.

## Known issues (left as submitted)

- **A2 — insert is O(n), not O(log n).** `nodeHeight()` recomputes heights by recursing through the whole subtree, and every rebalance step calls it. Measured: 2 000 / 4 000 / 8 000 sequential inserts take 3.6 / 13.1 / 48.3 ms, about 4× per doubling of n, which is O(n²) in total. The fix is to store `height` in each node and update it in O(1) after a rotation. `size()` also counts the whole tree (O(n)) instead of keeping a counter.
- **A2 — `Playlist` has no destructor.** It deletes songs in `clear()` and when a duplicate is rejected, so it owns them, but songs leak when a playlist goes out of scope. Leak detection is turned off for those two tests only (see `a2-threaded-avl/CMakeLists.txt`).
- **A2 — playback edge cases differ from the spec.** `playNext()`/`playPrevious()` wrap around instead of returning `nullptr` at the ends, and removing the current song moves playback to the next song instead of resetting it.
- **A1 — `playRandom`** builds two monotonic-stack arrays (`L`, `R`) that are never used.
- **A3 — performance was capped by the spec's linear lookups.** With a hash map from id to index and a binary-heap priority queue, Dijkstra would drop from O(V²) to O((V + E) log V), and clustering from O(V · E) to O(V + E).

## Layout
```
a1-linked-list/   BotkifyLinkedList.h, Playlist.{h,cpp}, main.cpp (test suite)
a2-threaded-avl/  IBST.h, AVL.h, ThreadedAVL.h, Playlist.{h,cpp}, main.cpp, property_test.cpp
a3-music-graph/   MusicGraph.{h,cpp}, main.h (graph base class), main.cpp, property_test.cpp
```
