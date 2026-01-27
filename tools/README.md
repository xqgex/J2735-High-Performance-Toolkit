# J2735 Code Generation Tools

> Development tools for the [J2735 High-Performance Toolkit](../README.md).

Python utilities for parsing J2735 ASN.1 specifications and generating C code.

> ⚠️ **Work in Progress**: These tools assist development but are not yet wired into an automated build. Generators output to stdout for manual review/integration.

## Architecture

```
┌─────────────────────────────────┐
│  J2735_*_pdf_content.txt        │  ← Spec text (input)
└────────────────┬────────────────┘
                 │
                 ▼
┌─────────────────────────────────┐
│  j2735_spec_parser.py           │  ← ASN.1 → Python objects
│  j2735_spec_constraints.py      │  ← UPER bit-width (ITU-T X.691)
└────────────────┬────────────────┘
                 │
                 ▼
┌─────────────────────────────────┐
│  j2735_c_generator_*.py         │  ← Code generators
│  templates/*.j2                 │  ← Jinja2 templates
└────────────────┬────────────────┘
                 │
                 ▼ stdout
┌─────────────────────────────────┐
│  src/J2735_*.h                  │  ← C headers (manual integration)
└─────────────────────────────────┘
```

## Quick Start

```bash
# From repository root, using the project venv:

# Parse spec, show summary
.venv/bin/python -m tools.j2735 J2735_202409_pdf_content.txt --summary

# List all types with bit-widths
.venv/bin/python -m tools.j2735 J2735_202409_pdf_content.txt --types

# Run tests
.venv/bin/python -m tools.run_tests
```

## Design Philosophy

The Python tools embody the [Unix philosophy](https://en.wikipedia.org/wiki/Unix_philosophy) for code generation:

- **One generator, one job** — Each `j2735_c_generator_*.py` produces one type of output
- **Templates separate from logic** — Jinja2 templates in `templates/` define the C syntax
- **Independently testable** — Each generator has corresponding tests in `tests/`

## Structure

```
tools/
├── j2735_spec_parser.py          # Spec text → Python objects
├── j2735_spec_constraints.py     # UPER bit-width calculations
├── j2735_c_generator_*.py        # Code generators
├── templates/                    # Jinja2 templates
├── tests/                        # Unit tests
└── run_tests.py                  # Test runner
```

## License

Apache-2.0 — See [LICENSE](../LICENSE)
