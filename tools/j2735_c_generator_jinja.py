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

from functools import lru_cache
from pathlib import Path
from re import sub

from jinja2 import (
    Environment,
    FileSystemLoader,
    StrictUndefined,
    Template,
    select_autoescape,
)

from .j2735_spec_constraints import IntegerConstraint, SequenceField

# =============================================================================
# Constants
# =============================================================================


_FILTER_BYTES_FROM_BITS = "bytes_from_bits"
_FILTER_C_TYPE = "c_type"
_FILTER_FORMAT_RANGE = "format_range"
_FILTER_IS_SIGNED = "is_signed"
_FILTER_SCREAMING_SNAKE = "screaming_snake"
_FILTER_SNAKE_CASE = "snake_case"
_TEMPLATES_DIR: Path = Path(__file__).parent / "templates"


# =============================================================================
# Filters
# =============================================================================


def filter_bytes_from_bits(bits: int) -> int:
    """Convert bit count to byte count using ceiling division.

    This is registered as a Jinja filter for use in templates.
    Single source of truth for bits -> bytes conversion.

    Args:
        bits: The number of bits.

    Returns:
        The number of bytes needed (ceiling of bits/8).

    Examples:
        >>> filter_bytes_from_bits(7)
        1
        >>> filter_bytes_from_bits(8)
        1
        >>> filter_bytes_from_bits(9)
        2
        >>> filter_bytes_from_bits(290)
        37
    """
    return (bits + 7) // 8


def filter_c_type(bits: int, is_signed: bool = False) -> str:
    """Return the smallest C integer type that can hold the given bit-width.

    This is registered as a Jinja filter for use in templates.

    Args:
        bits: The number of bits to store.
        is_signed: Whether the value is signed.

    Returns:
        C type string (e.g., "uint8_t", "int32_t").

    Examples:
        >>> filter_c_type(7)
        'uint8_t'
        >>> filter_c_type(7, False)
        'uint8_t'
        >>> filter_c_type(16, True)
        'int16_t'
        >>> filter_c_type(31, True)
        'int32_t'
        >>> filter_c_type(32, False)
        'uint32_t'
        >>> filter_c_type(48, False)
        'uint64_t'
    """
    prefix = "int" if is_signed else "uint"
    if bits <= 8:
        return f"{prefix}8_t"
    if bits <= 16:
        return f"{prefix}16_t"
    if bits <= 32:
        return f"{prefix}32_t"
    return f"{prefix}64_t"


def filter_format_range(field: SequenceField) -> str:
    """Format the value range for a field (e.g., "0..127").

    This is registered as a Jinja filter for use in templates.

    Args:
        field: The SequenceField to format.

    Returns:
        Range string like "0..127" or "" if no range available.

    Examples:
        >>> from tools.j2735_spec_constraints import SequenceField, IntegerConstraint
        >>> f = SequenceField(
        ...     name="lat", type_name="Latitude",
        ...     type=IntegerConstraint(min_value=-900000000, max_value=900000001),
        ...     is_optional=False, section_comment="", inline_comment="",
        ... )
        >>> filter_format_range(f)
        '-900000000..900000001'
    """
    if isinstance(field.type, IntegerConstraint):
        return f"{field.type.min_value}..{field.type.max_value}"
    return ""


def filter_is_signed(field: SequenceField) -> bool:
    """Check if a field's type is signed (min_value < 0).

    This is registered as a Jinja filter for use in templates.

    Args:
        field: The SequenceField to check.

    Returns:
        True if the field's type has min_value < 0.

    Examples:
        >>> from tools.j2735_spec_constraints import SequenceField, IntegerConstraint
        >>> f = SequenceField(
        ...     name="lat", type_name="Latitude",
        ...     type=IntegerConstraint(min_value=-900000000, max_value=900000001),
        ...     is_optional=False, section_comment="", inline_comment="",
        ... )
        >>> filter_is_signed(f)
        True
    """
    if isinstance(field.type, IntegerConstraint):
        return field.type.min_value < 0
    return False


def filter_screaming_snake(name: str) -> str:
    """Convert CamelCase or mixedCase name to SCREAMING_SNAKE_CASE.

    Handles abbreviations correctly using a multi-phase uppercase split:

    1. Single uppercase letter before CamelCase word (``DDate`` → ``D_DATE``).
       A start-of-string anchor ``^`` limits this to names that begin
       with a single-letter prefix, so mid-name abbreviations like
       ``LL`` in ``Node-LLmD-64b`` stay intact.
    2. Long abbreviation (4+ letters) before CamelCase word
       (``MUTCDCode`` → ``MUTCD_CODE``).  The min-4 threshold protects
       short abbreviations like BSM (3) unconditionally.  GNSS (4) and
       DSRC (4) would match but are safe because no spec name pairs them
       with a CamelCase suffix (they use lowercase: GNSSstatus, DSRCmsgID).
    3. Remaining abbreviation run (2+ uppercase) before lowercase
       (``GNSSstatus`` → ``GNSS_STATUS``).

    This is registered as a Jinja filter for use in templates.

    Args:
        name: The name to convert.

    Returns:
        SCREAMING_SNAKE_CASE version of the name.

    Examples:
        Standard CamelCase:

        >>> filter_screaming_snake("msgCnt")
        'MSG_CNT'
        >>> filter_screaming_snake("MsgCount")
        'MSG_COUNT'
        >>> filter_screaming_snake("AccelerationSet4Way")
        'ACCELERATION_SET_4_WAY'
        >>> filter_screaming_snake("LATITUDE")
        'LATITUDE'

        ASN.1 hyphenated names:

        >>> filter_screaming_snake("Offset-B10")
        'OFFSET_B_10'
        >>> filter_screaming_snake("Node-LL-24B")
        'NODE_LL_24_B'
        >>> filter_screaming_snake("NMEA-MsgType")
        'NMEA_MSG_TYPE'

        Short abbreviation + lowercase continuation (must stay together):

        >>> filter_screaming_snake("BSMcoreData")
        'BSM_CORE_DATA'
        >>> filter_screaming_snake("GNSSstatus")
        'GNSS_STATUS'
        >>> filter_screaming_snake("DSRCmsgID")
        'DSRC_MSG_ID'

        Single uppercase prefix + CamelCase (D-prefix date/time types):

        >>> filter_screaming_snake("DDate")
        'D_DATE'
        >>> filter_screaming_snake("DMonthDay")
        'D_MONTH_DAY'
        >>> filter_screaming_snake("DFullTime")
        'D_FULL_TIME'

        Long abbreviation (4+) before CamelCase word:

        >>> filter_screaming_snake("MUTCDCode")
        'MUTCD_CODE'
        >>> filter_screaming_snake("RTCMPackage")
        'RTCM_PACKAGE'

        Doubled letter after hyphen (LL = Lat/Lon abbreviation stays together):

        >>> filter_screaming_snake("Node-LLmD-64b")
        'NODE_LL_M_D_64_B'
        >>> filter_screaming_snake("Node-LLdms-48b")
        'NODE_LL_DMS_48_B'
    """
    # Step 1: Replace hyphens with underscores (ASN.1 names like Offset-B10, Node-LL-24B)
    name = name.replace("-", "_")
    # Step 2: Split single uppercase prefix before CamelCase word
    #   DDate -> D_Date, DHour -> D_Hour
    #   Anchored to ^ so it only fires at the very start of the name.
    #   This keeps mid-name abbreviations like LL in Node_LLmD intact,
    #   while still splitting D-prefix types (DDate, DHour, etc.).
    result = sub(r"^([A-Z])([A-Z][a-z])", r"\1_\2", name)
    # Step 3: Split long abbreviation (4+ uppercase) before CamelCase word
    #   MUTCDCode -> MUTCD_Code, RTCMPackage -> RTCM_Package
    #   Min-4 threshold protects BSM (3) unconditionally. GNSS (4) and DSRC (4)
    #   would match the {4,} pattern, but are safe because no spec name pairs
    #   them with a CamelCase suffix (they use lowercase: GNSSstatus, DSRCmsgID).
    result = sub(r"([A-Z]{4,})([A-Z][a-z])", r"\1_\2", result)
    # Step 4: Split remaining abbreviation run (2+ uppercase) before lowercase
    #   GNSSstatus -> GNSS_status, BSMcoreData -> BSM_coreData
    result = sub(r"([A-Z]{2,})([a-z])", r"\1_\2", result)
    # Step 5: Insert underscore between lowercase and uppercase
    result = sub(r"([a-z])([A-Z])", r"\1_\2", result)
    # Step 6: Insert underscore between letter and digit
    result = sub(r"([a-zA-Z])([0-9])", r"\1_\2", result)
    # Step 7: Insert underscore between digit and letter
    result = sub(r"([0-9])([a-zA-Z])", r"\1_\2", result)
    return result.upper()


def filter_snake_case(name: str) -> str:
    """Convert CamelCase or mixedCase name to snake_case (lowercase).

    Uses the same logic as filter_screaming_snake but returns lowercase.

    This is registered as a Jinja filter for use in templates.

    Args:
        name: The name to convert.

    Returns:
        snake_case version of the name.

    Examples:
        >>> filter_snake_case("msgCnt")
        'msg_cnt'
        >>> filter_snake_case("MsgCount")
        'msg_count'
        >>> filter_snake_case("BSMcoreData")
        'bsm_core_data'
        >>> filter_snake_case("AccelerationSet4Way")
        'acceleration_set_4_way'
        >>> filter_snake_case("Offset-B10")
        'offset_b_10'
        >>> filter_snake_case("Node-LL-24B")
        'node_ll_24_b'
    """
    return filter_screaming_snake(name).lower()


# =============================================================================
# Template Rendering
# =============================================================================


@lru_cache(maxsize=1)
def create_jinja_env() -> Environment:
    """Create and configure the Jinja2 environment.

    The result is cached - the Environment and its filters are never mutated
    after construction, so a single shared instance is safe.
    """
    env = Environment(
        autoescape=select_autoescape(default=False),
        keep_trailing_newline=True,
        loader=FileSystemLoader(_TEMPLATES_DIR),
        lstrip_blocks=True,
        trim_blocks=True,
        undefined=StrictUndefined,
    )
    env.filters[_FILTER_BYTES_FROM_BITS] = filter_bytes_from_bits
    env.filters[_FILTER_C_TYPE] = filter_c_type
    env.filters[_FILTER_FORMAT_RANGE] = filter_format_range
    env.filters[_FILTER_IS_SIGNED] = filter_is_signed
    env.filters[_FILTER_SCREAMING_SNAKE] = filter_screaming_snake
    env.filters[_FILTER_SNAKE_CASE] = filter_snake_case
    return env


def get_template(env: Environment, name: str) -> Template:
    """Get a template by name."""
    return env.get_template(name)
