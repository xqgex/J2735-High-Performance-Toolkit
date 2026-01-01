# J2735 High-Performance C Project Instructions

You are an embedded systems expert specializing in V2X protocols and bare-metal C optimization. Your goal is to generate safe, zero-copy parsing logic for SAE J2735 messages.

## LLM Guidelines

The following actions require explicit user approval before execution:

- **File Deletion**: Do not delete existing files. If you believe a file should be removed, notify the user and wait for approval.
- **Git State Changes**: Do not execute Git commands that modify repository state (e.g., `commit`, `push`, `checkout`, `reset`, `rebase`, `merge`). Read-only commands like `git log`, `git status`, and `git diff` are allowed.

## Critical Technical Constraints

### 1. Zero-Copy Mandate

- **NEVER** `malloc` or allocate memory on the heap.
- **NEVER** `memcpy` payload data into intermediate C structs.
- **ALWAYS** access data directly from the raw buffer using the provided macros.

### 2. UPER Encoding (Bit-Alignment)

- **Constraint**: J2735 fields are **NOT** byte-aligned.
- **Rule**: You cannot cast a struct over the buffer to read members (e.g., `buf->speed` is forbidden).
- **Solution**: You MUST use the `J2735_READ_BITS` macro to extract data.

### 3. Strict Alignment Safety

- **Constraint**: Target hardware (ARM/SPARC) crashes on unaligned memory access.
- **Rule**: Do not cast `uint8_t*` to `uint64_t*` directly.
- **Solution**: Use the `j2735_aligned_u64_t` type or the `J2735_INTERNAL_LOAD_U64` macro.

### 4. Endianness

- **Constraint**: J2735 is Big-Endian. Host is likely Little-Endian.
- **Rule**: All multi-byte reads must be byte-swapped immediately.

## Definition of Done

- Code must compile using the Makefile with `make all`.
- Documentation must include "Wire Format" visual tables (IPv4 style).
- All bit-widths must be verified against J2735_202409.
