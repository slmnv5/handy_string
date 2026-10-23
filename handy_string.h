#ifndef HANDY_STRING_H
#define HANDY_STRING_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**

* A read-only, non-owning reference to a string slice.
* Its lifetime depends entirely on the backing string memory.
*/
typedef struct {
  const char *data;
  size_t len;
} str_view_t;

/**

* A mutable string buffer that owns or manages a fixed-size or pre-allocated array.
*/
typedef struct {
  char *data;
  size_t len;
  size_t capacity;
} str_buf_t;

#define STR_VIEW_T(lit) ((str_view_t){.data = (lit), .len = sizeof(lit) - 1})

#define STR_VIEW_EMPTY ((str_view_t){.data = NULL, .len = 0})

#define STR_BUF_T(size) ((str_buf_t){.data = (char[size]){0}, .len = 0, .capacity = size})

/**

* Safely creates a str_view_t from a raw C string using a bounded scan to avoid crashes.
*/
str_view_t str_view_from_cstr(const char *c_str, size_t max_scan);

/**
 * Comparator function.
 */
int compare_str_view(str_view_t a, str_view_t b);

/**

* Finds the first occurrence of a needle view inside a haystack view.
* @return A new str_view_t pointing to the match, or an empty view if not found.
*/
str_view_t str_view_find(str_view_t haystack, str_view_t needle);

/**

* Copies a read-only view into a mutable str_buf_t. Check bounds and null-termination.
* @return true on success, false on failure/overflow.
*/
bool str_buf_copy(str_buf_t *dest, str_view_t src);

/**
 * Extracts a substring bounded between two marker views.
 * @return A new str_view_t representing the inner slice, or an empty view if not found.
 */
str_view_t str_view_between(str_view_t src, str_view_t m1, str_view_t m2);

/**

* Converts all alphabetic characters in the mutable string buffer to uppercase.
*/
void str_buf_to_upper_case(str_buf_t *str);

/**

* Trims leading and trailing whitespace from a view without modifying original memory.
* @return A new str_view_t representing the stripped slice.
*/
str_view_t str_view_strip(str_view_t view);

/**
 * Appends a read-only string view onto the end of a mutable str_buf_t.
 * Check bounds and null-termination.
 * @return true on success, false on failure/overflow.
 */
bool str_buf_append(str_buf_t *dest, str_view_t src);

/**
 * Return count of string views in array. -1 if error (e.g., invalid list pointer).
 */
int str_view_array_fill(str_view_t *arr, size_t arr_size, str_view_t input, str_view_t delim);

/**
 * Returns true if equal.
 */
bool str_view_eq_cstr(str_view_t view, const char *cstr);

#endif // HANDY_STRING_H

#ifdef HANDY_STRING_IMPLEMENTATION

str_view_t str_view_from_cstr(const char *c_str, size_t max_scan) {
  if (!c_str) return (str_view_t){.data = NULL, .len = 0};
  size_t l = 0;
  while (l < max_scan && c_str[l] != '\0') {
    l++;
  }
  return (str_view_t){.data = c_str, .len = l};
}

int compare_str_view(str_view_t a, str_view_t b) {
  if (a.len != b.len) {
    return (a.len < b.len) ? -1 : 1;
  }
  if (a.len == 0) {
    return 0; // avoid call memcmp on NULL
  }
  return memcmp(a.data, b.data, a.len);
}

str_view_t str_view_find(str_view_t haystack, str_view_t needle) {
  if (haystack.data == NULL || needle.data == NULL || needle.len == 0 ||
      needle.len > haystack.len) {
    return (str_view_t){.data = NULL, .len = 0};
  }

  size_t max_idx = haystack.len - needle.len;
  for (size_t i = 0; i <= max_idx; i++) {
    if (memcmp(&haystack.data[i], needle.data, needle.len) == 0) {
      return (str_view_t){.data = &haystack.data[i], .len = needle.len};
    }
  }
  return (str_view_t){.data = NULL, .len = 0};
}

bool str_buf_copy(str_buf_t *dest, str_view_t src) {
  if (dest == NULL || dest->data == NULL || src.data == NULL) {
    return false;
  }

  if (src.len >= dest->capacity) {
    fprintf(stderr, "str_buf_copy: destination buffer is small\n");
    dest->data[0] = '\0';
    dest->len = 0;
    return false;
  }

  memcpy(dest->data, src.data, src.len);
  dest->data[src.len] = '\0';
  dest->len = src.len;
  return true;
}

str_view_t str_view_between(str_view_t src, str_view_t m1, str_view_t m2) {
  str_view_t match1 = str_view_find(src, m1);
  if (match1.len == 0) return (str_view_t){.data = NULL, .len = 0};

  // Advance past the first marker
  const char *start = match1.data + m1.len;
  size_t remaining_len = (src.data + src.len) - start;
  str_view_t remainder = {.data = start, .len = remaining_len};

  // Find the second marker in the remainder
  str_view_t match2 = str_view_find(remainder, m2);
  if (match2.len == 0) return (str_view_t){.data = NULL, .len = 0};

  // Return the view cleanly pinned between them
  size_t inner_len = match2.data - start;
  return (str_view_t){.data = start, .len = inner_len};
}

void str_buf_to_upper_case(str_buf_t *str) {
  if (!str || !str->data) return;
  for (size_t i = 0; i < str->len; i++) {
    str->data[i] = (char)toupper((unsigned char)str->data[i]);
  }
}

str_view_t str_view_strip(str_view_t view) {
  if (!view.data) return view;

  while (view.len > 0 && isspace((unsigned char)*view.data)) {
    view.data++;
    view.len--;
  }
  while (view.len > 0 && isspace((unsigned char)view.data[view.len - 1])) {
    view.len--;
  }
  return view;
}

bool str_buf_append(str_buf_t *dest, str_view_t src) {
  if (!dest || !dest->data || src.len == 0 || !src.data) {
    return false; // Nothing to append, or invalid handles
  }

  size_t available = dest->capacity - dest->len - 1;

  if (src.len > available) {
    fprintf(stderr, "str_buf_append: remaining buffer capacity is small\n");
    return false;
  }

  memcpy(dest->data + dest->len, src.data, src.len);

  dest->len += src.len;
  dest->data[dest->len] = '\0';

  return true;
}

int str_view_array_fill(str_view_t *arr, size_t arr_size, str_view_t input, str_view_t delim) {
  if (!arr) {
    fprintf(stderr, "str_view_array_fill: array pointer is NULL\n");
    return -1;
  }
  if (!arr_size) {
    fprintf(stderr, "str_view_array_fill: array size is zero\n");
    return -1;
  }

  int count = 0;
  if (delim.len == 0 || !delim.data) {
    arr[count++] = input;
    return (int)count;
  }
  str_view_t remaining = input;
  while (1) {
    if ((size_t)count >= arr_size) {
      fprintf(stderr, "str_view_array_fill: array is full, size: %zu\n", arr_size);
      return -1;
    }

    if (remaining.len == 0 || !remaining.data) { // empty node at the end
      arr[count++] = (str_view_t){.data = remaining.data, .len = 0};
      return count;
    }

    str_view_t found = str_view_find(remaining, delim);

    if (found.len == 0 || !found.data) {
      arr[count++] = remaining;
      break;
    }

    str_view_t chunk = {.data = remaining.data, .len = found.data - remaining.data};
    arr[count++] = chunk;

    size_t consumed = chunk.len + delim.len;
    remaining.data += consumed;

    remaining.len -= consumed;
  }

  return count;
}

bool str_view_eq_cstr(str_view_t view, const char *cstr) {
  if (cstr == NULL) {
    return false;
  }

  if (view.len != strlen(cstr)) {
    return false;
  }

  return memcmp(view.data, cstr, view.len) == 0;
}

#endif // HANDY_STRING_IMPLEMENTATION
