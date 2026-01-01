#!/usr/bin/env python3
# Copyright 2026 Yogev Neumann
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2026 Yogev Neumann
"""
J2735 Python Test Runner.

Single entry point for all Python tests:
    1. Doctests - from all tools/*.py modules
    2. Unit tests - from tools/tests/**/test_*.py
    3. Integration tests - subset of unit tests in tools/tests/integration/

Usage:
    python -m tools.run_tests           # Run with minimal output
    python -m tools.run_tests -v        # Run with verbose output
    python tools/run_tests.py           # Direct execution
"""

from argparse import ArgumentParser
from doctest import testmod
from importlib import import_module
from io import StringIO
from pathlib import Path
from sys import exit as sys_exit
from sys import path as sys_path
from unittest import TestLoader, TextTestRunner

# =============================================================================
# Constants
# =============================================================================

_TOOLS_DIR = Path(__file__).parent
_ROOT_DIR = _TOOLS_DIR.parent
_TESTS_DIR = _TOOLS_DIR / "tests"

# Ensure workspace root is in sys.path for proper imports
if str(_ROOT_DIR) not in sys_path:
    sys_path.insert(0, str(_ROOT_DIR))


# =============================================================================
# Doctest Runner
# =============================================================================


def discover_python_modules() -> list[str]:
    """Discover all Python modules in the tools directory.

    Returns:
        List of module names (e.g., ["tools.j2735_spec_parser", ...]).
    """
    module_names: list[str] = []
    for py_file in _TOOLS_DIR.glob("j2735*.py"):
        # Convert file path to module name
        module_name = f"tools.{py_file.stem}"
        module_names.append(module_name)
    return sorted(module_names)


def run_doctests(verbose: bool = False) -> tuple[int, int]:
    """Run doctests from all tools modules.

    Dynamically discovers all j2735*.py modules in the tools directory.

    Args:
        verbose: If True, print each test as it runs.

    Returns:
        Tuple of (failures, total_tests).
    """
    print("=" * 60)
    print("Running Doctests")
    print("=" * 60)

    total_failed = 0
    total_tests = 0

    for mod_name in discover_python_modules():
        if verbose:
            print(f"\n--- {mod_name} ---")

        try:
            mod = import_module(mod_name)
        except ImportError as e:
            print(f"  SKIP: Cannot import {mod_name}: {e}")
            continue

        results = testmod(mod, verbose=verbose)
        total_failed += results.failed
        total_tests += results.attempted

    return total_failed, total_tests


# =============================================================================
# Unit Test Runner
# =============================================================================


def run_unit_tests(verbose: bool = False) -> tuple[bool, int]:
    """Run all unit tests via discovery.

    Args:
        verbose: If True, print each test as it runs.

    Returns:
        Tuple of (success, test_count).
    """
    print("\n" + "=" * 60)
    print("Running Unit Tests")
    print("=" * 60)

    loader = TestLoader()
    suite = loader.discover(
        start_dir=str(_TESTS_DIR),
        pattern="test_*.py",
        top_level_dir=str(_TOOLS_DIR.parent),
    )

    if verbose:
        runner = TextTestRunner(verbosity=2)
        result = runner.run(suite)
    else:
        # Capture output, only display if there are failures
        stream = StringIO()
        runner = TextTestRunner(verbosity=2, stream=stream)
        result = runner.run(suite)
        if not result.wasSuccessful():
            print(stream.getvalue())

    return result.wasSuccessful(), result.testsRun


# =============================================================================
# Main Entry Point
# =============================================================================


def main() -> int:
    """Main entry point."""
    parser = ArgumentParser(
        prog="run_tests",
        description="Run J2735 Python tests (doctests + unit tests + integration)",
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")
    args = parser.parse_args()

    all_pass = True

    # Run doctests
    doc_failures, doc_tests = run_doctests(verbose=args.verbose)
    if doc_failures == 0:
        print(f"  ✓ All {doc_tests} doctest tests passed")
    else:
        print(f"  ✗ {doc_failures}/{doc_tests} doctest tests failed")
        all_pass = False

    # Run unit tests (includes integration tests via discovery)
    unit_pass, unit_count = run_unit_tests(verbose=args.verbose)
    if unit_pass:
        print(f"  ✓ All {unit_count} unit tests passed")
    else:
        print("  ✗ Some unit tests failed")
        all_pass = False

    # Summary
    print("\n" + "=" * 60)
    if all_pass:
        print("✓ ALL TESTS PASSED")
    else:
        print("✗ SOME TESTS FAILED")
    print("=" * 60)

    return 0 if all_pass else 1


if __name__ == "__main__":
    sys_exit(main())
