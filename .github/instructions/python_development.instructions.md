---
applyTo: "tools/**/*.py"
---

# Python Development Standards

Rules for developing and modifying Python code in the `tools/` directory.

## Verification Requirements

After creating or modifying any Python file, run these checks:

1. **Formatting**: `black tools/ --config tools/pyproject.toml`
2. **Style (ruff)**: `ruff check tools/ --fix --config tools/pyproject.toml`
3. **Type checking**: `python -m mypy tools/ --strict`
4. **Linting**: `python -m pylint tools/`
5. **Style (flake8)**: `python -m flake8 tools/ --config tools/.flake8`
6. **Spelling**: `codespell tools/ --toml tools/pyproject.toml`
7. **Tests**: `python -m tools.run_tests`

All checks must pass with **exit code 0**, no error is minor.

## Code Quality Standards

### Type Annotations

- All functions must have complete type annotations
- Prefer `TypedDict` over `dict[str, Any]` for structured data

### Docstrings

- All functions require docstrings with:
  - One-line summary
  - Args section (if parameters exist)
  - Returns section (if non-None return)
  - Examples section (doctests preferred)

### Error Handling

- Use specific exception types, not bare `Exception`
- Include context in error messages
- Validate inputs at function boundaries
