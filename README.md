# J2735 High-Performance Toolkit

Zero-copy parser for SAE J2735 UPER-encoded messages.

> ⚠️ **Early Development**: This project is under active development. The API is not yet stable and may change frequently. Not all J2735 message types are implemented.

## Specification

Based on **SAE J2735_202409** (September 2024 revision).

## Design Philosophy & Performance

This library is designed for **High-Frequency/Low-Latency** environments (RSU/OBU firmware) where CPU cycles and memory bandwidth are at a premium. It provides the convenience of struct-like access without the penalty of serialization.

### 1. The UPER Alignment Problem

Unlike network protocols such as TCP/IP, J2735 UPER is **bit-aligned**, not byte-aligned.
- **The Consequence**: A single 7-bit field shifts the alignment of all subsequent data.
- **Why specific libraries are needed**: You cannot simply cast a C struct over the raw buffer (e.g., `(BSM*)buffer`). Doing so would read misaligned bits and produce garbage data. This forces most developers to rely on heavy serialization libraries.

### 2. vs. Standard ASN.1 Compilers (e.g., `asn1c`)

The standard approach in the open-source community is to use an ASN.1 compiler to generate C code.
- **The Workflow**: The compiler generates a parser that walks the bits stream, allocates memory (typically `calloc`/`malloc`), and **copies** the data into a fully populated C structure.
- **The Cost**: This "Deserialization" step is $O(N)$ relative to the message size. It incurs CPU overhead for copying data and memory overhead for the structure, even if you only need to read a single field (like Latitude).
- **Our Approach**: **Zero-Copy Introspection**. We do not deserialize. We calculate the bit-offset of the specific field you need and read only those bits. If you only need one field, you pay only for one read.

### 3. Complexity Analysis

| Metric | Traditional ASN.1 Parsers | **This Library** | Notes |
| :--- | :--- | :--- | :--- |
| **Runtime Complexity** | $O(N)$ | **$O(1)$** | Traditional parsers must process the full stream. We jump directly to the target bits. |
| **Space Complexity** | $O(N)$ | **$O(1)$** | Traditional parsers allocate mirrored structs (Deep Copy). We use the buffer in-place (Zero Copy). |
| **Initialization** | Slow (Allocation + Parsing) | **Instant** | No setup phase required before reading data. |

## Integration

This is a **header-only** library. Copy the `src/` directory to your project and include:

```c
#include "J2735_api.h"
```

No linking required. No dependencies beyond standard C11.

### Requirements

- C11 compiler (GCC, Clang, or MSVC)
- Little-endian host (big-endian support exists but is less tested)

### Buffer Requirements

⚠️ **Critical**: All buffers passed to parsing macros **must** include 7 bytes of readable padding beyond the logical message end. This enables safe 64-bit reads without per-access bounds checks.

```c
uint8_t buf[MESSAGE_SIZE + 7];  // Always allocate with padding
```

## API Patterns

The library follows consistent naming conventions:

| Pattern | Purpose |
|---------|---------|
| `J2735_<TYPE>_GET_<FIELD>(buf)` | Read a field value |
| `J2735_<TYPE>_HAS_<FIELD>(buf)` | Check OPTIONAL field presence |
| `J2735_<TYPE>_IS_EXTENDED(buf)` | Check extension marker |
| `J2735_<TYPE>_SIZE(buf)` | Get wire size in bits |
| `J2735_BW_<TYPE>` | Bit-width constant |
| `J2735_INTERNAL_*` | Implementation details (do not use directly) |

## Structure

```
J2735/
├── build/       # Build artifacts
├── src/         # Library headers
├── tests/       # Test suite
└── tools/       # Python code generators
```

## Build

```bash
make clean       # Clean build artifacts
make test        # Build and run tests
```

## Quality Checks

```bash
make check       # Cppcheck
make format      # Clang-Format
make sanitize    # ASan + UBSan
make tidy        # Clang-Tidy
make valgrind    # Valgrind
```

## Compiler

The project support Clang, GCC and MSVC. To set the default compiler, set the `CC` environment variable. For example, to use Clang:

```bash
CC=clang make test
```

## License

This project is licensed under the **Apache License 2.0**. See the [LICENSE](LICENSE) file for details.

```
Copyright 2026 Yogev Neumann
SPDX-License-Identifier: Apache-2.0
```

## Trademark Policy

The name **"J2735 High-Performance Toolkit"** and associated branding are
trademarks of Yogev Neumann.

You are free to:
- Fork this project and modify the code
- Distribute modified versions under the Apache 2.0 license
- Use this library in commercial and proprietary applications

**However**, if you distribute a modified version, you **must not**:
- Use the name "J2735 High-Performance Toolkit" in a way that implies your
  version is the official release
- Use the project name or branding to suggest endorsement of your derivative work

You **must** clearly state that your version is a modification of the original.

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

By submitting a Pull Request, you agree that your contribution will be licensed
under the Apache License 2.0.
