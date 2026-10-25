# handy_string - single header C library for strings

It implements strings using "fat string pointers" holding pointer to a buffer, size, capacity. 

### Fat Pointers for Strings in C

Standard C strings (`char*`) store only memory address of the first character and rely on a null terminator `\0` byte to mark the end of the string. This design frequently leads to safety issues and performance bottlenecks.

A **fat pointer** pattern wraps the raw string memory address with its metadata (such as length, capacity) inside a unified structure.

| Feature | Standard C String (`char*`) | Fat Pointer String |
| :--- | :--- | :--- |
| **Length Retrieval** | **O(N)** Must scan sequentially via `strlen`. | **O(1)** Kept as metadata. |
| **Slicing / Substrings** | **Hard.** Requires allocating new memory or mutating the string with `\0`. | **Easy.** Adjust the pointer and length without touching memory. |
| **Safety** | **Low.** Risk of buffer overflows and out-of-bounds reads. | **High.** Functions check length before touching memory. |
| **Binary Data** | **No.** One `\0` byte truncates the string early. | **Yes.** Keeping length eliminates the need of `\0` byte. |


### Running Test Cases

Script test.sh compiles and runs unit tests. File test.c has example of how to use the library.

### How to Link Your Application

Include header handy_string.h and handy_string.c in your project. 
To see examples refer to unit tests.


