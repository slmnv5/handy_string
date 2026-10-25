#include "handy_string.h"

static void report(const char *test_name, bool condition) {
  printf("[%s] %s\n", condition ? "PASS" : "FAIL", test_name);
}

int main(void) {
  // -------------------------------------------------------------
  // TEST SECTION 1: STRVIEW CREATION & UTILITIES
  // -------------------------------------------------------------
  printf("--- 1. Testing str_view_t Creation ---\n");

  // Macro tests
  str_view_t v_lit = STR_VIEW("Hello World");
  report("STR_VIEW length test", v_lit.len == 11);
  report("STR_VIEW data match test", memcmp(v_lit.data, "Hello World", 11) == 0);

  // Bounded raw C string scanning
  const char raw_string[] = "Test String";
  str_view_t v_cstr1 = str_view(raw_string, 50);
  report("str_view length test", v_cstr1.len == 11);

  const char unterminated_payload[] = {'A', 'B', 'C', 'D'};
  str_view_t v_cstr2 = str_view(unterminated_payload, 4);
  report("str_view scan unterminated string", v_cstr2.len == 4);

  str_view_t v_null = str_view(NULL, 100);
  report("str_view NULL test", v_null.data == NULL && v_null.len == 0);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 2: SEARCH UTILITIES (str_view_find)
  // -------------------------------------------------------------
  printf("--- 2. Testing str_view_find ---\n");

  str_view_t haystack = STR_VIEW("The quick brown fox jumps over the lazy dog");
  str_view_t needle_normal = STR_VIEW("brown");
  str_view_t needle_missing = STR_VIEW("cat");
  str_view_t needle_too_long =
      STR_VIEW("The quick brown fox jumps over the lazy dog match length overflow");

  str_view_t match1 = str_view_find(haystack, needle_normal);
  report("str_view_find located existing match", match1.data != NULL && match1.len == 5);
  if (match1.data) {
    size_t index = (size_t)(match1.data - haystack.data);
    report("str_view_find calculated correct match offset", index == 10);
  }

  str_view_t match2 = str_view_find(haystack, needle_missing);
  report("str_view_find yields empty view on mismatch",
         compare_str_view(match2, STR_VIEW_EMPTY) == 0);

  str_view_t match3 = str_view_find(haystack, needle_too_long);
  report("str_view_find yields empty view when needle exceeds haystack length",
         compare_str_view(match3, STR_VIEW_EMPTY) == 0);

  str_view_t match4 = str_view_find(haystack, STR_VIEW(""));
  report("str_view_find handles empty needle find safely",
         compare_str_view(match4, STR_VIEW_EMPTY) == 0);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 3: BOUNDED COPYING (str_buf_copy)
  // -------------------------------------------------------------
  printf("--- 3. Testing str_buf_copy (Safety Controls) ---\n");

  str_buf_t dest_buf = STR_BUF(16);

  // Standard safe copy scenario
  str_view_t valid_src = STR_VIEW("Fits perfectly");
  bool cpy_status1 = str_buf_copy(&dest_buf, valid_src);
  report("str_buf_copy status success", cpy_status1);
  report("str_buf_copy length is correct", dest_buf.len == 14);
  report("str_buf_copy null-termination correct", dest_buf.data[dest_buf.len] == '\0');
  report("str_buf_copy text payload correct", strcmp(dest_buf.data, "Fits perfectly") == 0);

  str_view_t oversized_src =
      STR_VIEW("This sentence is way too long for a sixteen byte buffer boundary");
  bool cpy_status2 = str_buf_copy(&dest_buf, oversized_src);
  report("str_buf_copy error on capacity overflow", !cpy_status2);
  report("str_buf_copy empties buffer on error", dest_buf.len == 0);
  report("str_buf_copy null termination on error", dest_buf.data[0] == '\0');
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 4: MARKER SUBSTRING EXTRACTION (str_view_between)
  // -------------------------------------------------------------
  printf("--- 4. Testing str_view_between ---\n");

  str_view_t text_payload = STR_VIEW("Data containing [TARGET_VALUE] inside");
  str_view_t mark_left = STR_VIEW("[");
  str_view_t mark_right = STR_VIEW("]");

  str_view_t v1 = str_view_between(text_payload, mark_left, mark_right);
  report("str_view_between match", strncmp(v1.data, "TARGET_VALUE", strlen("TARGET_VALUE")) == 0);

  str_view_t bad_right_mark = STR_VIEW("}");
  str_view_t v2 = str_view_between(text_payload, mark_left, bad_right_mark);
  report("str_view_between not found", v2.len == 0 && v2.data == NULL);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 5: CASE UPDATES (str_buf_to_upper_case)
  // -------------------------------------------------------------
  printf("--- 5. Testing str_buf_to_upper_case ---\n");

  str_buf_t case_buf = STR_BUF(32);
  str_buf_copy(&case_buf, STR_VIEW("MixedCase_123_stRInG"));

  str_buf_to_upper(&case_buf);
  report("str_buf_to_upper_case length remains constant", case_buf.len == 20);
  report("str_buf_to_upper_case conversion result check",
         strcmp(case_buf.data, "MIXEDCASE_123_STRING") == 0);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 6: VIEW TRIMMING (str_view_strip)
  // -------------------------------------------------------------
  printf("--- 6. Testing str_view_strip ---\n");

  str_view_t raw_spaces_view = STR_VIEW("   \t  Clean data wrapper   \n ");
  str_view_t stripped_view = str_view_strip(raw_spaces_view);

  report("str_view_strip shrinks length properly", stripped_view.len == 18);

  // Copy the stripped view results into a mutable container to verify its content via string
  // matching
  str_buf_t strip_final_buf = STR_BUF(32);
  str_buf_copy(&strip_final_buf, stripped_view);

  report("str_view_strip exact match", strcmp(strip_final_buf.data, "Clean data wrapper") == 0);

  // Check extreme case of parsing an entire line of whitespace characters
  str_view_t all_whitespaces = STR_VIEW("    \t\n   ");
  str_view_t clean_empty_view = str_view_strip(all_whitespaces);
  report("str_view_strip on pure whitespace returns empty view", clean_empty_view.len == 0);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 7: ARRAY SPLITTING (str_view_array_fill)
  // -------------------------------------------------------------
  printf("--- 7. Testing str_view_array_fill ---\n");

  str_view_t arr[5];
  size_t cap = 5;
  int res;

  // Test A: Standard Multi-character Delimiter Split
  str_view_t input_normal = {.data = "user||pass||admin", .len = 17};
  str_view_t delim_normal = {.data = "||", .len = 2};

  res = str_view_array_fill(arr, cap, input_normal, delim_normal);
  report("Split count validation", res == 3);

  report("Split (token 1)", strncmp(arr[0].data, "user", strlen("user")) == 0);
  report("Split (token 2)", strncmp(arr[1].data, "pass", strlen("pass")) == 0);
  report("Split (token 3)", strncmp(arr[2].data, "admin", strlen("admin")) == 0);

  str_view_t input_edge = {.data = ",hello,", .len = 7};
  str_view_t delim_edge = {.data = ",", .len = 1};

  res = str_view_array_fill(arr, cap, input_edge, delim_edge);
  report("Split count validation", res == 3);
  report("Split empty slice", arr[0].len == 0);
  report("Split data", strncmp(arr[1].data, "hello", strlen("hello")) == 0);
  report("Split empty slice", arr[2].len == 0);

  str_view_t input_overflow = {.data = "1,2,3,4,5,6", .len = 11};
  str_view_t delim_overflow = {.data = ",", .len = 1};
  str_view_t tiny_arr[3];
  res = str_view_array_fill(tiny_arr, 3, input_overflow, delim_overflow);
  report("Split handles buffer overflow bounds failure", res == -1);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 8: EDGE CASE SPLITTING
  // -------------------------------------------------------------
  printf("--- 8. Testing Edge Case Splitting ---\n");

  str_view_t edge_arr[4];
  str_view_t pure_delim_input = STR_VIEW(",,,");
  str_view_t comma_delim = STR_VIEW(",");

  res = str_view_array_fill(edge_arr, 4, pure_delim_input, comma_delim);
  report("Split consecutive delimiters and count", res == 4);

  str_view_t missing_delim = STR_VIEW("NoDelimHere");
  res = str_view_array_fill(edge_arr, 4, missing_delim, comma_delim);
  report("Split returns single token when delimiter is empty", res == 1);
  report("Token matches original string length", edge_arr[0].len == 11);
  printf("\n");

  // -------------------------------------------------------------
  // TEST SECTION 9: NULL POINTER HANDLING
  // -------------------------------------------------------------
  printf("--- 9. Testing Null Pointer Handling ---\n");

  str_view_t empty_view1 = {.data = NULL, .len = 0};

  report("Compare two empty views", compare_str_view(empty_view1, STR_VIEW_EMPTY) == 0);
  report("Compare empty and non empty views", compare_str_view(empty_view1, STR_VIEW("A")) < 0);

  printf("\n");

  return 0;
}
