# handy_string - single header C library for strings

It implements strings using "fat string pointers" holding pointer to a buffer, size, capacity. 

### Fat Pointers for Strings in C

Standard C strings (`char*`) are **thin pointers**. They only store the memory address of the first character and rely on a null terminator (`'\0'`) to mark the end of the string. This design frequently leads to safety issues and performance bottlenecks.

A **fat pointer** pattern wraps the raw string memory address with its metadata (such as length or capacity) inside a unified structure.

### Thin vs. Fat Pointers

| Feature | Standard C String (`char*`) | Fat Pointer String (`FatString`) |
| :--- | :--- | :--- |
| **Length Retrieval** | O(N) — Must scan sequentially via `strlen`. | **O(1) — Kept as metadata.** |
| **Slicing / Substrings** | Requires allocating new memory or destructively mutating the string with `\0`. | **O(1) — Adjust the pointer and length variables without touching memory.** |
| **Safety** | High risk of buffer overflows and out-of-bounds reads. | **High safety — Functions can cross-reference `len` before reading memory.** |
| **Binary Data** | Impossible. Null bytes truncate the string early. | **Supported. Explicit length overrides the need for null bytes.** |


### 🧪 Running Test Cases

Script test.sh compiles and runs unit tests. File test.c has example of how to use the library.

### 📦 How to Link Your Application

Include header handy_string.h and handy_string.c (optional) in your project. 
To see examples refer to unit tests.


