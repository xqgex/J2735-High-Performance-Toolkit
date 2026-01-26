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
J2735 Jinja2 Template Utilities.

Provides Jinja2 environment setup and template loading for C code generation.
"""

from pathlib import Path
from re import sub

from jinja2 import Environment, FileSystemLoader, Template, select_autoescape

# =============================================================================
# Constants
# =============================================================================


_FILTER_BYTES_FROM_BITS = "bytes_from_bits"
_FILTER_C_TYPE = "c_type"
_FILTER_SCREAMING_SNAKE = "screaming_snake"
_FILTER_SNAKE_CASE = "snake_case"
_TEMPLATES_DIR: Path = Path(__file__).parent / "templates"


# =============================================================================
# Filters
# =============================================================================


def bytes_from_bits(bits: int) -> int:
    """Convert bit count to byte count using ceiling division.

    This is registered as a Jinja filter for use in templates.
    Single source of truth for bits -> bytes conversion.

    Args:
        bits: The number of bits.

    Returns:
        The number of bytes needed (ceiling of bits/8).

    Examples:
        >>> bytes_from_bits(7)
        1
        >>> bytes_from_bits(8)
        1
        >>> bytes_from_bits(9)
        2
        >>> bytes_from_bits(290)
        37
    """
    return (bits + 7) // 8


def c_type(bits: int, is_signed: bool = False) -> str:
    """Return the smallest C integer type that can hold the given bit-width.

    This is registered as a Jinja filter for use in templates.

    Args:
        bits: The number of bits to store.
        is_signed: Whether the value is signed.

    Returns:
        C type string (e.g., "uint8_t", "int32_t").

    Examples:
        >>> c_type(7)
        'uint8_t'
        >>> c_type(7, False)
        'uint8_t'
        >>> c_type(16, True)
        'int16_t'
        >>> c_type(31, True)
        'int32_t'
        >>> c_type(32, False)
        'uint32_t'
        >>> c_type(48, False)
        'uint64_t'
    """
    # TODO: Verify the function with unit tests for the edge cases
    prefix = "int" if is_signed else "uint"
    if bits <= 8:
        return f"{prefix}8_t"
    if bits <= 16:
        return f"{prefix}16_t"
    if bits <= 32:
        return f"{prefix}32_t"
    return f"{prefix}64_t"


def screaming_snake(name: str) -> str:
    """Convert CamelCase or mixedCase name to SCREAMING_SNAKE_CASE.

    Handles abbreviations correctly: when 2+ uppercase letters are followed
    by a lowercase letter, the abbreviation stays together.

    This is registered as a Jinja filter for use in templates.

    Args:
        name: The name to convert.

    Returns:
        SCREAMING_SNAKE_CASE version of the name.

    Examples:
        >>> screaming_snake("msgCnt")
        'MSG_CNT'
        >>> screaming_snake("MsgCount")
        'MSG_COUNT'
        >>> screaming_snake("BSMcoreData")
        'BSM_CORE_DATA'
        >>> screaming_snake("AccelerationSet4Way")
        'ACCELERATION_SET_4_WAY'
    """
    # Step 1: Insert underscore after abbreviation (2+ uppercase) before lowercase
    result = sub(r"([A-Z]{2,})([a-z])", r"\1_\2", name)
    # Step 2: Insert underscore between lowercase and uppercase
    result = sub(r"([a-z])([A-Z])", r"\1_\2", result)
    # Step 3: Insert underscore between letter and digit
    result = sub(r"([a-zA-Z])([0-9])", r"\1_\2", result)
    # Step 4: Insert underscore between digit and letter
    result = sub(r"([0-9])([a-zA-Z])", r"\1_\2", result)
    return result.upper()


def snake_case(name: str) -> str:
    """Convert CamelCase or mixedCase name to snake_case (lowercase).

    Uses the same logic as screaming_snake but returns lowercase.

    This is registered as a Jinja filter for use in templates.

    Args:
        name: The name to convert.

    Returns:
        snake_case version of the name.

    Examples:
        >>> snake_case("msgCnt")
        'msg_cnt'
        >>> snake_case("MsgCount")
        'msg_count'
        >>> snake_case("BSMcoreData")
        'bsm_core_data'
        >>> snake_case("AccelerationSet4Way")
        'acceleration_set_4_way'
    """
    return screaming_snake(name).lower()


# =============================================================================
# Template Rendering
# =============================================================================


def create_jinja_env() -> Environment:
    """Create and configure the Jinja2 environment."""
    env = Environment(
        loader=FileSystemLoader(_TEMPLATES_DIR),
        autoescape=select_autoescape(default=False),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True,
    )
    # TODO: Fix pylance `Type of "filters" is partially unknown`
    env.filters[_FILTER_BYTES_FROM_BITS] = bytes_from_bits
    env.filters[_FILTER_C_TYPE] = c_type
    env.filters[_FILTER_SCREAMING_SNAKE] = screaming_snake
    env.filters[_FILTER_SNAKE_CASE] = snake_case
    return env


def get_template(env: Environment, name: str) -> Template:
    """Get a template by name."""
    return env.get_template(name)
