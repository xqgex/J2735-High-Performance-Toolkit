---
applyTo: "tools/tests/**/*.py,tools/run_tests.py"
---

# Python Test Organization

Rules for organizing Python tests in the `tools/` directory.

## 1. Structure

```
tools/
├── run_tests.py                        # Single entry point
└── tests/
    ├── __init__.py
    ├── conftest.py                     # Shared fixtures
    ├── spec/                           # Tests for j2735_spec_*.py
    │   ├── __init__.py
    │   └── test_<class_or_concept>.py
    ├── c_generator/                     # Tests for j2735_c_generator_*.py
    │   ├── __init__.py
    │   └── test_<class_or_concept>.py
    ├── cli/                            # Tests for j2735.py
    │   ├── __init__.py
    │   └── test_<feature>.py
    └── integration/                    # End-to-end tests
        ├── __init__.py
        └── test_<scenario>.py
```

## 2. The Scaling Invariant

**Each test file tests ONE class or ONE closely-related function group.**

- `IntegerConstraint` → `tests/spec/test_integer_constraint.py`
- `get_sequence_variants()` → `tests/c_generator/test_wire_format_variants.py`
- Wire format templates → `tests/c_generator/test_wire_format_templates.py`
- New class added? → New test file created.

This ensures files can remain in the target of ~300-500 lines.

## 3. File Naming

- Prefix: `test_<name>.py` (pytest/unittest standard)
- Name describes the tested unit, not the source file
- Examples:
  - ✓: `test_integer_constraint.py` - good
  - ✗: `test_j2735_spec_constraints.py` - bad (too broad)

## 4. Entry Point

`tools/run_tests.py` runs all test types with one command:

1. **Doctests**: Extracted from all `tools/*.py` modules
2. **Unit tests**: Discovered from `tools/tests/**/test_*.py`
3. **Integration tests**: Subset of unit tests in `tools/tests/integration/`

Usage: `python tools/run_tests.py [-v]`

## 5. Imports

Tests import from parent package:

```python
from tools.j2735_spec_constraints import IntegerConstraint
```

Shared fixtures go in `conftest.py`:

```python
# conftest.py
SPEC_FILE_PATH = Path(__file__).parent.parent.parent / "J2735_202409_pdf_content.txt"
```

## 6. When Creating Tests

1. Identify the class or function group being tested
2. Find or create the appropriate file in the matching subdirectory
3. Keep tests focused—split file if it exceeds 500 lines
4. Add shared fixtures to `conftest.py`, not inline
