# CSE 111 — Advanced Programming (UC Santa Cruz)

Coursework from UC Santa Cruz's advanced programming course (**Fall 2020**), written in **modern C++ (C++17/20)**.
The course focused on writing large, careful programs: RAII and smart pointers, operator overloading, templates and iterators, inheritance, exceptions, and finally sockets and a client/server protocol.

> **About the dates:** this repo was put on GitHub in 2026, but the commit history was rebuilt from the original file timestamps.
> Each commit is dated when those files were last saved. The first commits are the course-provided starter files. The 2024 commits are when I came back to get the client/server assignment building on a newer toolchain.

## What's inside

| Folder | Program | What it does | Key concepts |
|---|---|---|---|
| [`asg1-dc-bigint`](asg1-dc-bigint) | `ydc` | A clone of the Unix `dc` reverse-Polish calculator with arbitrary-precision integers: add, subtract, multiply, divide, remainder, power, and stack commands | Operator overloading, `std::vector` digit storage, long multiplication/division, exceptions |
| [`asg2-shell-fnptrs-oop`](asg2-shell-fnptrs-oop) | `yshell` | An in-memory Unix-like shell and file system supporting `cd`, `ls`, `lsr`, `mkdir`, `make`, `cat`, `echo`, `pwd`, `rm`, `rmr`, `prompt` | Inheritance and polymorphism (plain file vs. directory inodes), `shared_ptr`-managed inode tree, command dispatch through a function-pointer map |
| [`asg3-listmap-templates`](asg3-listmap-templates) | `keyvalue` | A generic sorted map (`listmap<Key, Value, Less>`) on a doubly linked list with bidirectional iterators, used to process `key = value` commands from files or stdin | Class templates, custom iterators, comparator policies, Rule of Five |
| [`asg4-client-server`](asg4-client-server) | `cxi` / `cxid` | A client and a forking server that transfer files over TCP with a custom binary header protocol: `ls`, `get`, `put`, `rm` | BSD sockets, `fork`, network byte order, protocol design, error handling |

## Building and running

Each folder builds on its own (Linux or WSL with `g++` and `make`):

```bash
cd asg1-dc-bigint
make
echo "2 100 ^ p" | ./ydc        # 1267650600228229401496703205376
```

```bash
cd asg4-client-server
make
./cxid 50000 &                  # start the server on port 50000
./cxi localhost 50000           # connect, then try: ls, put file, get file, rm file
```

The `mk.build` / `mk.tests` scripts in each folder run the course's test cases.

## Tech
C++17/20 · G++ · GNU Make · Valgrind · POSIX sockets

## Notes
- Each assignment started from course-provided scaffolding (`debug.*`, `util.*`, `Makefile`, and for asg4 the socket wrappers and protocol header). The logic in files such as `ubigint.cpp`, `bigint.cpp`, `commands.cpp`, `file_sys.*`, `listmap.*`, `main.cpp`, `cxi.cpp` and `cxid.cpp` is my own.
- These are student assignments, shared as part of my portfolio. If you're taking CSE 111 now, please follow your course's academic integrity policy.
