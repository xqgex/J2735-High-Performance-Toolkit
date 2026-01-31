# J2735 High-Performance Toolkit

**Zero-copy, O(1) parser for SAE J2735 V2X structures.**

Designed for embedded RSU/OBU firmware where CPU cycles and memory bandwidth are critical.

> ⚠️ **Early Development**: The API is currently unstable. Refer to the `tests/` directory for the most up-to-date usage examples.

**Specification**: SAE J2735_202409 (September 2024 revision)

## ⚡ The Performance Invariant

Standard ASN.1 parsers (like `asn1c`) deserialize the entire message into a struct before you can read a single field. This toolkit does not deserialize. It calculates bit-offsets on the fly.

**Why can't you just cast a C struct over the buffer?** Because J2735 UPER is **bit-aligned**, not byte-aligned:

```
Byte:     [  0   ] [  1   ] [  2   ]
Bits:     76543210 76543210 76543210
Fields:   [ A   ][ B          ]
          7 bits  12 bits      <-- Spans across byte boundaries
```

Traditional parsers copy everything to a struct. We jump directly to the bits you need.

| Metric      | Standard ASN.1 Parsers | This Toolkit |
| ----------- | ---------------------- | ------------ |
| **Complexity** | `O(N)` (Must parse everything)    | `O(1)` (Jump to specific bits) |
| **Memory**     | `O(N)` (Allocates mirror structs) | `O(1)` (Reads buffer in-place) |
| **Startup**    | Slow (Allocation + Init)          | **Instant** (Stateless macros) |

## 🛠️ Integration

**Header-only**. No libraries to link.

1. Copy the `src/` folder to your project.
2. Include `J2735_api.h` or specific internal headers.

### Critical Requirement: Buffer Padding

You **must** allocate **7 extra bytes** at the end of your input buffer.

- **Why?** The parser performs safe 64-bit unaligned loads. Without padding, reading the last byte of a message could trigger a memory access violation.
- **Invariant**: `uint8_t buf[MSG_SIZE + 7];`

## 💻 Usage & Documentation

Because this library is in active alpha, the code itself is the source of truth.

1. **For API Examples**: See `tests/`.
   - The test files (e.g., `tests/J2735_internal_DF_BSMcoreData_test.c`) contain compiled, verified examples of how to access fields.
2. **For Available Types**: See `src/`.
   - Header files define the exact macros available for each J2735 Data Frame.

## Build & Test

```sh
make check     # Static analysis (cppcheck)
make clean     # Clean build artifacts
make format    # Code formatting (clang-format)
make pre-push  # Run ALL checks (recommended before pushing)
make sanitize  # ASan + UBSan
make test      # Build and run tests
make tidy      # Static analysis (clang-tidy)
make valgrind  # Memory leak check
```

Supports GCC, Clang, and MSVC. To use a specific compiler:

```sh
CC=clang make test
```

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

[Apache 2.0](LICENSE) - Copyright 2026 Yogev Neumann
