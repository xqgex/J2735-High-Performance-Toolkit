---
applyTo: "tests/**/*_test.c"
---

# J2735 Testing Standards

These rules govern the creation of unit tests for the J2735 High-Performance Toolkit.

## 1. Critical Technical Constraints

### Safety Padding (ASan Prevention)

Zero-copy parsers often perform wide reads (32/64-bit) that may slightly overhang the logical end of a message.

- **Rule**: ALL mock data buffers must include 8 bytes of zero-padding at the end.
- **Reason**: Prevents AddressSanitizer (ASan) "global-buffer-overflow" false positives during wide integer casts.
  ```c
  // CORRECT
  static const uint8_t buffer[] = {
      0x15, 0xBD, //... payload...
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // Safety padding
  };
  ```

### 64-Bit Assertions

J2735 uses fields > 32 bits.

- **Rule:** You MUST use `TEST_ASSERT_BITS64` and `TEST_ASSERT_EQUAL_UINT64` for any field larger than 32 bits.
- **Rule:** Do not rely on implicit promotion in `TEST_ASSERT_EQUAL_HEX`.

## 2. Mock Data Documentation (AI Job)

- **Rule**: When generating test vectors, you (the AI) MUST generate a standardized Doxygen comment block BEFORE the test function. Do not output raw hex without explaining it.
- **Reason**: Wire format documentation is the source of truth for debugging and serves as specification verification.

### Required Doxygen Structure

Every test function must have these sections in order:

1. **`@brief`** - One-line description of what this test verifies
2. **`@par ASN.1 Definition:`** - The type definition in a `@code` block
3. **`@par Test Vector:`** - Bullet list of field values being tested
4. **`@par Wire Format (N bits total):`** - Logical field layout table
5. **`@par Byte Encoding:`** - Physical byte values table

### Table Column Standards

**Wire Format Table:**
| Column | Description |
|--------|-------------|
| Offset (bits) | Bit position from start of encoding (0-indexed) |
| Width | Number of bits for this field |
| Field | Field name from ASN.1 definition |
| Value | Binary representation (no spaces, MSB first) |

**Byte Encoding Table:**
| Column | Description |
|--------|-------------|
| Byte | Byte index (0-indexed) |
| Hex | Byte value in 0xNN format |
| Binary | 8-bit binary representation |
| Fields | Which fields/bits are in this byte using `field[hi:lo]` notation |

### Inline Comments in Payload

Each byte in the payload array gets a short inline comment showing field composition:

```c
0x80, /* preamble(1) + region[15:9] */
```

### Complete Template

```c
/**
 * @brief Test TypeName with specific condition being verified.
 *
 * @par ASN.1 Definition:
 * @code
 * TypeName ::= SEQUENCE {
 *   field1  Type1,           -- width bits (signed/unsigned)
 *   field2  Type2 OPTIONAL,  -- width bits
 *   ...                      -- (if extensible)
 * }
 * @endcode
 *
 * @par Test Vector:
 * - field1: value (0xHEX)
 * - field2: PRESENT/ABSENT, value (0xHEX)
 *
 * @par Wire Format (N bits total):
 * | Offset (bits) | Width | Field    | Value            |
 * |---------------|-------|----------|------------------|
 * | 0             | 1     | preamble | 0/1              |
 * | 1             | 16    | field1   | 0000001111101000 |
 * | 17            | 8     | field2   | 01100100         |
 *
 * @par Byte Encoding:
 * | Byte | Hex  | Binary   | Fields                |
 * |------|------|----------|-----------------------|
 * | 0    | 0x01 | 00000001 | preamble + field1[15:9]|
 * | 1    | 0xF4 | 11110100 | field1[8:1]           |
 * | 2    | 0x32 | 00110010 | field1[0] + field2[7:1]|
 * | 3    | 0x00 | 00000000 | field2[0] + padding   |
 */
/* cppcheck-suppress misra-c2012-8.7 ; Unity RUN_TEST requires external linkage */
void test_typename_condition(void) {
  static const uint8_t payload[] = {
      0x01,                                          /* preamble + field1[15:9] */
      0xF4,                                          /* field1[8:1] */
      0x32,                                          /* field1[0] + field2[7:1] */
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /* safety padding */
  };
  // ... test assertions ...
}
```

## 3. Test Patterns

### Naming Convention

`void test_<Message>_<Field>_<Condition>(void)`

Examples:

- `test_bsm_latitude_valid_range(void)`
- `test_bsm_msgcount_boundary_values(void)`
- `test_spat_optional_field_absent(void)`

### Misalignment Testing

Since this is an embedded library, you MUST include at least one test case per message type that forces unaligned access.

```c
void test_bsm_misaligned_access(void) {
  // Arrange - Deliberately misalign buffer
  static const uint8_t payload[100] = { 0xFF, 0x15, 0xBD, ... };
  // Offset pointer by 1 byte to force misalignment
  const uint8_t *unaligned_ptr = &payload[1];
  // Act
  uint8_t msg_cnt = J2735_BSM_CORE_DATA_GET_MSG_COUNT(unaligned_ptr);
  // Assert
  TEST_ASSERT_EQUAL_UINT8(expected, msg_cnt);
}
```

## 4. Unity Assertion Cheat Sheet

| Data Type | Assertion Macro | Note |
|-----------|-----------------|------|
| `true` | `TEST_ASSERT_TRUE_MESSAGE(cond, msg)` | True condition |
| `false` | `TEST_ASSERT_FALSE_MESSAGE(cond, msg)` | False condition |
| `NULL` | `TEST_ASSERT_NULL_MESSAGE(ptr, msg)` | Pointer is NULL |
| `!NULL` | `TEST_ASSERT_NOT_NULL_MESSAGE(ptr, msg)` | Pointer is not NULL |
| `int32_t` | `TEST_ASSERT_EQUAL_INT32_MESSAGE(exp, act, msg)` | Signed 32-bit |
| `uint32_t` | `TEST_ASSERT_EQUAL_UINT32_MESSAGE(exp, act, msg)` | Unsigned 32-bit |
| `int64_t` | `TEST_ASSERT_EQUAL_INT64_MESSAGE(exp, act, msg)` | Signed 64-bit |
| `uint64_t` | `TEST_ASSERT_EQUAL_UINT64_MESSAGE(exp, act, msg)` | Unsigned 64-bit |
| Bitmask | `TEST_ASSERT_BITS_MESSAGE(mask, exp, act, msg)` | 32-bit masks |
| Arrays | `TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(exp, act, len, msg)` | For buffer compares |

## 5. Anti-Patterns

### ❌ NEVER ignore ASan failures. If ASan complains, the test code is wrong, not the tool.

### ❌ Undocumented Magic Numbers

```c
// WRONG - No wire format explanation
static const uint8_t buf[] = { 0x15, 0xBD, 0x5B };
```

### ❌ Naked Assertions Without Context

```c
// WRONG - Failure provides no debugging info
TEST_ASSERT(value == 42);

// CORRECT - Explains what failed
TEST_ASSERT_EQUAL_INT32_MESSAGE(42, value, "MsgCount out of range");
```
