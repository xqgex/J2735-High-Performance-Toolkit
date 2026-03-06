---
applyTo: "**/*.c,**/*.h"
---

# J2735 Library Architecture

This document outlines the "Lens Architecture" used to achieve zero-copy parsing. Use this context when modifying headers or implementing parser logic.

## Design Pattern: The Lens Architecture

The library splits the concept of a "Message" into two layers to map high-level concepts onto raw memory without moving bytes.

### 1. The Topology Layer (`J2735_toolkit.h`)

- **Concept**: Defines the "Shape" of the data on the wire.
- **Usage**:
  - Use `J2735_BW_*` constants to define bit-widths from the ASN.1 spec.
  - Use "Container Structs" (byte arrays) only for pointer sizing, not for member access.
  - **Anti-Pattern**: Do not define standard C structs with members for data access.

### 2. The Access Layer (`J2735_internal_common.h`)

- **Concept**: Acts as the optical lens to read the data.
- **Usage**:
  - Use `J2735_INTERNAL_LOAD_U64` for safe unaligned loads.
  - Use `BSM_OFF_*` constants for `O(1)` random access in fixed frames.

## Core Algorithms

### The "Packed Cast" Trick

We use a compiler-intrinsic pattern to handle unaligned memory safely without `memcpy`.

- **Code**: `typedef struct { uint64_t v; } __attribute__((packed)) j2735_aligned_u64_t;`
- **Why**: Accessing `v` forces the compiler to emit safe unaligned instructions (e.g., `LDR` on ARM, `MOV` on x86).

### "Onion Layer" Navigation

For variable-length or optional fields (like SPAT/MAP), we cannot use fixed offsets.

1. **Cursor**: Maintain a bit-cursor or pointer.
2. **Peel**: Read the "Presence Bit" (1 bit).
3. **Advance**: If present, read the data width and advance the cursor. If absent, skip.

### UPER Arithmetic Rules

1. **Fixed-Frame (BSM Core)**: Calculate cumulative `OFFSET_FIELD` constants relative to the start of the frame.
2. **Dynamic-Frame**: Calculate offsets at runtime based on preceding presence bits.
