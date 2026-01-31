# Contributing to J2735 High-Performance Toolkit

Thank you for your interest in contributing to this project! We welcome
contributions from the community.

## How to Contribute

### Reporting Issues

- Check existing issues before creating a new one
- Provide clear reproduction steps
- Include your compiler version and platform

### Submitting Changes

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/my-improvement`)
3. Make your changes following our coding standards
4. Run all checks (`make pre-push`)
5. Commit with a signed-off message (see DCO below)
6. Submit a Pull Request to the `main` branch

## Developer Certificate of Origin (DCO)

By submitting a contribution to this project, you certify that:

1. The contribution was created in whole or in part by you and you have
   the right to submit it under the Apache License 2.0; or

2. The contribution is based upon previous work that, to the best of your
   knowledge, is covered under an appropriate open source license and you
   have the right to submit that work with modifications; or

3. The contribution was provided directly to you by some other person who
   certified (1) or (2) and you have not modified it.

**To sign off your commits**, add `-s` to your git commit command:

```bash
git commit -s -m "Add new feature"
```

This adds a `Signed-off-by` line to your commit message, indicating your
agreement with the DCO.

## Coding Standards

### Zero-Copy Mandate

- **NEVER** use `malloc` or heap allocation
- **NEVER** use `memcpy` on payload data
- **ALWAYS** access data directly from raw buffers using provided macros

### Code Style

- Follow existing code formatting (use `make format`)
- All code must compile with by both GCC and Clang
- Add documentation for new macros and functions
- Include "Wire Format" visual tables for new message types

### Testing

- Add tests for new functionality
- Ensure existing tests pass
- Run sanitizers before submitting

## Design Decisions

Understanding the rationale behind key architectural choices helps maintain consistency across contributions.

### Packed-Cast for Unaligned Access

We use `__attribute__((packed))` structs instead of `memcpy` for unaligned 64-bit loads. This compiles to a single instruction (MOV on x86, LDR on ARM) while maintaining alignment safety on strict architectures (ARM, SPARC). The compiler handles the complexity.

### Single-Read Optimization

For extensible types like `VehicleEventFlags`, we read the maximum possible bits in one operation, then use pure arithmetic to extract fields. This avoids conditional I/O operations and reduces branch mispredictions in hot paths.

### Macro-Based Public API

The public API uses macros rather than inline functions to ensure zero overhead and allow the compiler to fully optimize away unused code paths. Internal complexity is hidden behind `J2735_INTERNAL_*` prefixes that users should never call directly.

### Generator Architecture

Each Python generator handles one concern (bit positions, offsets, accessors). This follows the Unix philosophy: small tools that do one thing well. Assembly into complete headers is a separate step, making reorganization trivial.

## License

By contributing, you agree that your contributions will be licensed under
the Apache License 2.0. See the [LICENSE](LICENSE) file for details.

All new source files must include the standard Apache 2.0 header with
SPDX identifier.
