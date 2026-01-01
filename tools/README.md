# J2735 Code Generation Tools

> Sub-module of the [J2735 High-Performance Toolkit](../README.md).

Python tools that parse J2735 ASN.1 specifications and generate the C headers in `src/`.

## Architecture

```
┌─────────────────────────────────┐
│  J2735_*_pdf_content.txt        │  ← Spec text (input)
└────────────────┬────────────────┘
                 │
                 ▼
┌─────────────────────────────────┐
│  j2735_spec_parser.py           │  ← ASN.1 → Python objects
└────────────────┬────────────────┘
                 │
                 ▼
┌─────────────────────────────────┐
│  j2735_spec_constraints.py      │  ← UPER bit-width calculation
└────────────────┬────────────────┘
                 │
                 ▼
┌─────────────────────────────────┐
│  j2735_c_generator_*.py         │  ← C code generators
│  + templates/*.j2               │
└────────────────┬────────────────┘
                 │
                 ▼
┌─────────────────────────────────┐
│  src/J2735_*.h                  │  ← Generated C headers
└─────────────────────────────────┘
```

### Key Modules

| Module | Responsibility |
|--------|----------------|
| `j2735_spec_parser.py` | Parse ASN.1 text into `ASN1TypeDefinition` objects |
| `j2735_spec_constraints.py` | Constraint classes with UPER bit-width formulas (ITU-T X.691) |
| `j2735_c_generator_data_element.py` | Generate `DE_*.h` headers (BIT STRING types) |
| `j2735_c_generator_jinja.py` | Jinja2 environment and filters (`screaming_snake`, `c_type`, etc.) |
| `templates/` | Jinja2 templates for C code generation |

## Design Philosophy

The Python tools embody the [Unix philosophy](https://en.wikipedia.org/wiki/Unix_philosophy) for code generation:

- Each generator does ONE thing
- Each is independently testable
- Assembly is a separate concern
- Reorganization is trivial (just change the assembly, not the generators)

## Usage

```bash
# From repository root:

# Parse spec, show summary
python -m tools.j2735 J2735_202409_pdf_content.txt

# Run tests
python -m tools.run_tests -v
```

## Development

```bash
pip install -e ".[dev,test]"   # Install with dev dependencies
python -m tools.run_tests -v   # Run tests
```

## License

Apache-2.0 — See [LICENSE](../LICENSE)
