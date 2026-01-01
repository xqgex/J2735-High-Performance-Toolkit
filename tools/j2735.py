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
J2735 Specification Parser CLI.

Command-line interface for parsing SAE J2735 specification text files
and displaying summary information about the parsed types.

Usage:
    python j2735.py <spec_file> [--summary | --types]
"""

from argparse import ArgumentParser
from sys import exit as sys_exit
from sys import stderr

from tools.j2735_spec_parser import parse_spec_file


def main() -> None:
    """Command-line interface for the parser."""
    parser = ArgumentParser(
        prog="j2735",
        description="Parse SAE J2735 specification text files and display type information.",
    )
    parser.add_argument(
        "spec_file",
        metavar="SPEC_FILE",
        help="Path to the J2735 specification text file",
    )
    parser.add_argument(
        "--summary",
        action="store_true",
        default=True,
        help="Display summary counts (default)",
    )
    parser.add_argument(
        "--types",
        action="store_true",
        help="Display detailed type dump for debugging",
    )
    args = parser.parse_args()

    try:
        spec = parse_spec_file(args.spec_file)
    except FileNotFoundError as e:
        print(f"Error: {e}", file=stderr)
        sys_exit(1)

    # TODO: Make it proper: either "--summary" or "--types"
    if args.types:
        # Detailed type dump for debugging
        for name, typedef in sorted(spec.type_registry.items()):
            bw = typedef.uper_bit_width
            bw_str = f"{bw} bits" if bw else "variable"
            print(f"{name}: {typedef.type_class.name} ({bw_str})")
    else:
        # Summary mode (default)
        print(f"J2735 Specification Version: {spec.version}")
        print(f"  Messages:      {len(spec.messages)}")
        print(f"  Data Frames:   {len(spec.data_frames)}")
        print(f"  Data Elements: {len(spec.data_elements)}")
        print(f"  Type Registry: {len(spec.type_registry)} types")


if __name__ == "__main__":
    main()
