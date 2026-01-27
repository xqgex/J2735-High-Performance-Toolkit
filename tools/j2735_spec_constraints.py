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
J2735 ASN.1 Constraint Types.

Defines constraint classes for ASN.1 types used in J2735 specifications.
Each constraint class provides UPER bit-width calculation per ITU-T X.691.

This module provides:
    - UPERConstraint: Abstract base class for all constraints
    - BitStringConstraint: BIT STRING SIZE constraints
    - ChoiceType: CHOICE alternative types
    - EnumeratedType: ENUMERATED value sets
    - IA5StringConstraint: IA5String SIZE constraints
    - IntegerConstraint: INTEGER (min..max) range constraints
    - OctetStringConstraint: OCTET STRING SIZE constraints
"""

from abc import ABC, abstractmethod
from collections.abc import Mapping
from dataclasses import dataclass, replace
from math import ceil, log2
from re import DOTALL, Pattern
from re import compile as re_compile
from typing import Annotated, ClassVar, Final, Self

# These type aliases document constraints that are validated at runtime
# via __post_init__. They help communicate intent to readers and tools.
_PositiveInt = Annotated[int, "Value must be >= 1"]
_NonNegativeInt = Annotated[int, "Value must be >= 0"]

# ASN.1 parsing constants
_ASN1_COMMENT_PREFIX: Final[str] = "--"
_ASN1_EXTENSION_MARKER: Final[str] = "..."
_ASN1_FIELD_SEPARATOR: Final[str] = ","
_ASN1_OPTIONAL_KEYWORD: Final[str] = "OPTIONAL"
_ASN1_SEQUENCE_KEYWORD: Final[str] = "SEQUENCE"
_BITS_PER_BYTE: Final[int] = 8  # 8 bits per byte/octet in OCTET STRING

# J2735-specific constants
# In J2735 CHOICE types, "regional" fields contain SEQUENCE OF RegionalExtension
# which are variable-length and should be excluded from bit-width calculations
_J2735_REGIONAL_FIELD_NAME: Final[str] = "regional"


def _extract_outermost_braces(text: str) -> str | None:
    """Extract content between the outermost matching braces.

    Unlike regex [^{}]+, this properly handles nested braces like
    {{Reg-Position3D}} found in J2735 regional extension fields.

    Args:
        text: The text containing braces.

    Returns:
        The content between the first '{' and its matching '}',
        or None if no matching braces found.

    Examples:
        >>> _extract_outermost_braces("SEQUENCE { a A, b B }")
        ' a A, b B '
        >>> _extract_outermost_braces("SEQUENCE { a A, regional {{Reg-X}} }")
        ' a A, regional {{Reg-X}} '
        >>> _extract_outermost_braces("no braces")
    """
    start = text.find("{")
    if start == -1:
        return None

    depth = 0
    for i, char in enumerate(text[start:], start=start):
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                # Found matching closing brace
                return text[start + 1 : i]  # noqa: E203  # Black VS flake8

    return None  # No matching closing brace


class UPERConstraint(ABC):
    """Abstract base class for ASN.1 UPER-encodable constraints.

    All constraint types must implement:
        - PATTERN: ClassVar regex pattern for parsing ASN.1 definitions
        - from_asn1(): Factory classmethod to parse from raw ASN.1 string
        - uper_bit_width: Property to calculate encoding size per ITU-T X.691

    Subclasses that fail to define PATTERN or from_asn1 will raise
    NotImplementedError at class definition time or first use.
    """

    # Subclasses MUST override this with their parsing regex
    PATTERN: ClassVar[Pattern[str]]

    def __init_subclass__(cls, **kwargs: object) -> None:
        """Validate that subclasses define required class variables."""
        super().__init_subclass__(**kwargs)
        # Check PATTERN is defined (not inherited from ABC)
        if "PATTERN" not in cls.__dict__:
            raise NotImplementedError(
                f"{cls.__name__} must define a PATTERN class variable "
                f"with a compiled regex for parsing ASN.1 definitions"
            )

    @property
    @abstractmethod
    def uper_bit_width(self) -> int | None:
        """Calculate UPER bit-width for this constraint.

        Returns:
            Number of bits required, or None if variable-length encoding.
        """
        raise NotImplementedError

    @classmethod
    @abstractmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse constraint from an ASN.1 type definition string.

        Args:
            raw_def: The raw ASN.1 type definition.

        Returns:
            An instance of the constraint if parsing succeeds, None otherwise.
        """
        raise NotImplementedError


@dataclass(frozen=True, kw_only=True, slots=True)
class BitStringConstraint(UPERConstraint):
    """Represents an ASN.1 BIT STRING size constraint with named bits.

    Supports three SIZE constraint patterns:
        - SIZE(n): Fixed size, not extensible
        - SIZE(n, ...): Extensible with open upper bound
        - SIZE(n, ..., m): Extensible with known extension size

    Attributes:
        root_size: Root component bit count (must be >= 1).
        is_extensible: Whether the SIZE has an extension marker (...).
        extension_size: Extended bit count, or None if open/not extensible.
        named_bits: Mapping of bit names to their positions (0-indexed).
            Must have exactly max_size entries (extension_size or root_size).

    Raises:
        ValueError: If root_size < 1.
        ValueError: If extension_size is provided but <= root_size.
        ValueError: If named_bits count doesn't match max_size.
        ValueError: If named_bits contains invalid or duplicate positions.

    Examples:
        >>> c = BitStringConstraint(
        ...     root_size=2, is_extensible=False, extension_size=None,
        ...     named_bits={"a": 0, "b": 1}
        ... )
        >>> c.uper_bit_width
        2
        >>> c = BitStringConstraint(
        ...     root_size=2, is_extensible=True, extension_size=None,
        ...     named_bits={"flag0": 0, "flag1": 1}
        ... )
        >>> c.uper_bit_width is None
        True
        >>> c = BitStringConstraint(
        ...     root_size=2, is_extensible=True, extension_size=3,
        ...     named_bits={"a": 0, "b": 1, "c": 2}
        ... )
        >>> c.is_extensible
        True
    """

    root_size: _PositiveInt
    is_extensible: bool
    extension_size: _PositiveInt | None
    named_bits: Mapping[str, int]

    def __post_init__(self) -> None:
        """Validate constraint values."""
        if self.root_size < 1:
            raise ValueError(f"BitStringConstraint.root_size must be >= 1, got {self.root_size}")
        if self.extension_size is not None:
            if not self.is_extensible:
                raise ValueError("extension_size requires is_extensible=True")
            if self.extension_size <= self.root_size:
                raise ValueError(
                    f"extension_size ({self.extension_size}) must be > "
                    f"root_size ({self.root_size})"
                )
        # Validate named_bits
        if not self.named_bits:
            raise ValueError("BitStringConstraint.named_bits must not be empty")
        max_size = self.extension_size if self.extension_size is not None else self.root_size
        # J2735 BIT STRING types have contiguous named bits for all positions
        if len(self.named_bits) != max_size:
            raise ValueError(
                f"named_bits count ({len(self.named_bits)}) must equal "
                f"max size ({max_size}) - all bit positions must be named"
            )
        positions_seen: set[int] = set()
        for name, pos in self.named_bits.items():
            if pos < 0:
                raise ValueError(f"Bit position for '{name}' must be >= 0, got {pos}")
            if pos >= max_size:
                raise ValueError(
                    f"Bit position for '{name}' ({pos}) must be < max size ({max_size})"
                )
            if pos in positions_seen:
                raise ValueError(f"Duplicate bit position {pos} for '{name}'")
            positions_seen.add(pos)

    # Regex pattern for BIT STRING SIZE constraints:
    # Group 1: root size (required)
    # Group 2: ", ..." extension marker (optional)
    # Group 3: extension size after second comma (optional)
    #
    # Matches:
    #   SIZE(12)           -> root=12, ext=None, ext_size=None
    #   SIZE(9, ...)       -> root=9, ext=..., ext_size=None
    #   SIZE(13, ..., 14)  -> root=13, ext=..., ext_size=14
    PATTERN: ClassVar[Pattern[str]] = re_compile(
        r"BIT\s+STRING\s*\{[^}]*\}\s*\(\s*SIZE\s*\("
        r"\s*(\d+)"  # Group 1: root_size
        r"(\s*,\s*\.\.\.)?"  # Group 2: extension marker (optional)
        r"(?:\s*,\s*(\d+))?"  # Group 3: extension_size (optional, non-capturing outer)
        r"\s*\)\s*\)"
    )

    # Sub-pattern for individual bit definitions: name (position)
    _BIT_VALUE_PATTERN: ClassVar[Pattern[str]] = re_compile(r"(\w+)\s*\(\s*(\d+)\s*\)")

    @property
    def uper_bit_width(self) -> int | None:
        """Return the UPER bit-width for this BIT STRING.

        For non-extensible BIT STRING, returns the fixed root size.
        For extensible BIT STRING, returns None (variable-length).

        Returns:
            The root size in bits, or None if extensible.

        Examples:
            >>> c = BitStringConstraint(
            ...     root_size=2, is_extensible=False, extension_size=None,
            ...     named_bits={"a": 0, "b": 1}
            ... )
            >>> c.uper_bit_width
            2
            >>> c = BitStringConstraint(
            ...     root_size=2, is_extensible=True, extension_size=None,
            ...     named_bits={"a": 0, "b": 1}
            ... )
            >>> c.uper_bit_width is None
            True
        """
        if self.is_extensible:
            return None
        return self.root_size

    @property
    def ext_bits(self) -> int:
        """TODO

        Examples:
            >>> c = BitStringConstraint(
            ...     root_size=2, is_extensible=False, extension_size=None,
            ...     named_bits={"a": 0, "b": 1}
            ... )
            >>> c.ext_bits
            2
            >>> c = BitStringConstraint(
            ...     root_size=2, is_extensible=True, extension_size=3,
            ...     named_bits={"a": 0, "b": 1, "c": 2}
            ... )
            >>> c.ext_bits
            3
        """
        return self.extension_size or self.root_size

    @property
    def read_bits(self) -> int:
        """Return total bits to read for single-read optimization.

        For non-extensible BIT STRING, returns the root size.
        For extensible BIT STRING, returns 1 (ext marker) + 7 (nsnnwn) + ext_bits.

        This value is used by code generators to read all possible bits
        in a single J2735_READ_BITS call.

        Returns:
            Total bits to read from buffer.

        Examples:
            >>> c = BitStringConstraint(
            ...     root_size=2, is_extensible=False, extension_size=None,
            ...     named_bits={"a": 0, "b": 1}
            ... )
            >>> c.read_bits
            2
            >>> c = BitStringConstraint(
            ...     root_size=13, is_extensible=True, extension_size=14,
            ...     named_bits={f"bit{i}": i for i in range(14)}
            ... )
            >>> c.read_bits
            22
        """
        if self.is_extensible:
            return 1 + 7 + self.ext_bits
        return self.root_size

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse BIT STRING constraint from ASN.1 definition.

        Handles three patterns:
            - SIZE(n): Fixed, not extensible
            - SIZE(n, ...): Extensible, open upper bound
            - SIZE(n, ..., m): Extensible, known extension size

        Also extracts named bit definitions from the { name (pos), ... } block.
        Returns None if parsing fails or if named_bits count doesn't match SIZE.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            BitStringConstraint if successfully parsed and valid, None otherwise.

        Examples:
            >>> c = BitStringConstraint.from_asn1("BIT STRING { a (0), b (1) } (SIZE(2))")
            >>> c.root_size
            2
            >>> c.named_bits
            {'a': 0, 'b': 1}
            >>> c = BitStringConstraint.from_asn1("BIT STRING { flag (0) } (SIZE(1, ...))")
            >>> c.is_extensible
            True
            >>> c.root_size
            1
            >>> c.extension_size is None
            True
            >>> c = BitStringConstraint.from_asn1("BIT STRING { x (0), y (1) } (SIZE(1, ..., 2))")
            >>> c.extension_size
            2
            >>> c.named_bits
            {'x': 0, 'y': 1}
            >>> BitStringConstraint.from_asn1("BIT STRING { x (0) }") is None
            True
            >>> BitStringConstraint.from_asn1("BIT STRING { a (0) } (SIZE(5))") is None
            True
        """
        match = cls.PATTERN.search(raw_def)
        if not match:
            return None

        root_size = int(match.group(1))
        is_extensible = match.group(2) is not None
        extension_size = int(match.group(3)) if match.group(3) else None

        # Extract named bits from the { ... } block
        bits_body = _extract_outermost_braces(raw_def)
        if bits_body is None:
            return None

        named_bits: dict[str, int] = {}
        for bit_match in cls._BIT_VALUE_PATTERN.finditer(bits_body):
            name = bit_match.group(1)
            position = int(bit_match.group(2))
            named_bits[name] = position

        if not named_bits:
            return None

        try:
            return cls(
                root_size=root_size,
                is_extensible=is_extensible,
                extension_size=extension_size,
                named_bits=named_bits,
            )
        except ValueError:
            # Validation failed (e.g., count mismatch, invalid positions)
            return None


@dataclass(frozen=True, kw_only=True, slots=True)
class BooleanType(UPERConstraint):
    """Represents an ASN.1 BOOLEAN type.

    BOOLEAN is always encoded as a single bit in UPER.

    Examples:
        >>> BooleanType().uper_bit_width
        1
    """

    # Matches exactly "BOOLEAN" (with optional whitespace)
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"^\s*BOOLEAN\s*$")

    @property
    def uper_bit_width(self) -> int:
        """Return the fixed bit-width for BOOLEAN.

        Returns:
            Always 1 bit.

        Examples:
            >>> BooleanType().uper_bit_width
            1
        """
        return 1

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse BOOLEAN from ASN.1 definition.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            BooleanType if the definition is BOOLEAN, None otherwise.

        Examples:
            >>> BooleanType.from_asn1("BOOLEAN")
            BooleanType()
            >>> BooleanType.from_asn1("INTEGER") is None
            True
        """
        if cls.PATTERN.match(raw_def):
            return cls()
        return None


@dataclass(frozen=True, kw_only=True, slots=True)
class ChoiceType(UPERConstraint):
    """Represents an ASN.1 CHOICE type.

    A CHOICE is a union type where exactly one alternative is selected at runtime.
    In UPER encoding, the choice index is encoded first, followed by the selected
    alternative's value.

    Attributes:
        alternatives: Mapping of alternative names to their type references.
        is_extensible: Whether the CHOICE has an extension marker (...).

    Examples:
        >>> c = ChoiceType(
        ...     alternatives={"approach": "ApproachID", "lane": "LaneID"},
        ...     is_extensible=False,
        ... )
        >>> c.alternative_count
        2
        >>> c.uper_bit_width
        1
    """

    alternatives: Mapping[str, str]
    is_extensible: bool

    # Regex pattern: CHOICE { ... }
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"CHOICE\s*\{([^}]+)\}", DOTALL)

    # Sub-pattern for individual alternatives: name TypeRef
    # Handles optional OPTIONAL keyword and trailing commas
    _ALTERNATIVE_PATTERN: ClassVar[Pattern[str]] = re_compile(
        r"(\w+)\s+([A-Z][\w-]*(?:\s*\([^)]+\))?)"
    )

    @property
    def alternative_count(self) -> int:
        """Count the number of CHOICE alternatives.

        Returns:
            Number of defined alternatives.

        Examples:
            >>> ChoiceType(
            ...     alternatives={"a": "TypeA", "b": "TypeB", "c": "TypeC"},
            ...     is_extensible=False,
            ... ).alternative_count
            3
        """
        return len(self.alternatives)

    @property
    def uper_bit_width(self) -> int | None:
        """Calculate UPER bit-width for the choice index.

        Per ITU-T X.691 Section 23 (Encoding of a choice type):
        - Non-extensible: The index is a constrained whole number from
          0 to (number of alternatives - 1).
        - Extensible: Uses a 1-bit extension flag, plus the root index
          if the choice is in the root, or normally small non-negative
          whole number if in extension.

        For fixed bit-width calculation, we return the root encoding size.
        Variable-length extensions return None.

        Returns:
            Number of bits for the choice index, or None if variable.

        Examples:
            >>> ChoiceType(alternatives={"a": "A", "b": "B"}, is_extensible=False).uper_bit_width
            1
            >>> c = ChoiceType(alternatives={"a": "A", "b": "B", "c": "C"}, is_extensible=False)
            >>> c.uper_bit_width
            2
            >>> ChoiceType(alternatives={"a": "A"}, is_extensible=False).uper_bit_width
            0
            >>> ChoiceType(alternatives={"a": "A"}, is_extensible=True).uper_bit_width is None
            True
        """
        # Extensible CHOICEs have variable-length encoding
        if self.is_extensible:
            return None

        if self.alternative_count <= 1:
            return 0  # Single alternative, no index needed
        return ceil(log2(self.alternative_count))

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse CHOICE type from ASN.1 definition.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            ChoiceType if successfully parsed, None otherwise.

        Examples:
            >>> c = ChoiceType.from_asn1("CHOICE { approach ApproachID, lane LaneID }")
            >>> c.alternatives
            {'approach': 'ApproachID', 'lane': 'LaneID'}
            >>> c.is_extensible
            False
            >>> ChoiceType.from_asn1("CHOICE { a TypeA, b TypeB, ... }").is_extensible
            True
            >>> ChoiceType.from_asn1("NOT A CHOICE") is None
            True
        """
        match = cls.PATTERN.search(raw_def)
        if not match:
            return None

        body = match.group(1)
        is_extensible = _ASN1_EXTENSION_MARKER in body

        # Parse individual alternatives
        alternatives: dict[str, str] = {}
        for alt_match in cls._ALTERNATIVE_PATTERN.finditer(body):
            name = alt_match.group(1)
            type_ref = alt_match.group(2).strip()
            # Skip J2735 regional extension fields (SEQUENCE OF RegionalExtension)
            # These are variable-length and would break fixed bit-width calculation
            if name != _J2735_REGIONAL_FIELD_NAME or not type_ref.startswith(
                _ASN1_SEQUENCE_KEYWORD
            ):
                alternatives[name] = type_ref

        if not alternatives:
            return None

        return cls(alternatives=alternatives, is_extensible=is_extensible)


@dataclass(frozen=True, kw_only=True, slots=True)
class EnumeratedType(UPERConstraint):
    """Represents an ASN.1 ENUMERATED type.

    Attributes:
        values: Mapping of enumeration names to their integer values.
        is_extensible: Whether the enumeration has an extension marker (...).

    Examples:
        >>> e = EnumeratedType(values={"unavailable": 0, "on": 1, "off": 2}, is_extensible=False)
        >>> e.value_count
        3
        >>> e.uper_bit_width
        2
    """

    values: Mapping[str, int]
    is_extensible: bool

    # Regex pattern: ENUMERATED { ... }
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"ENUMERATED\s*\{([^}]+)\}")

    # Sub-pattern for individual enum values: name (value) - explicit
    _VALUE_PATTERN: ClassVar[Pattern[str]] = re_compile(r"(\w+)\s*\(\s*(\d+)\s*\)")

    # Sub-pattern for implicit enum values: identifier preceded by `,` `{` or `...`
    # and followed by , } ... or end-of-string (to avoid matching comment words)
    _IMPLICIT_VALUE_PATTERN: ClassVar[Pattern[str]] = re_compile(
        r"(?:^|[,{]|\.\.\.)\s*([a-zA-Z][a-zA-Z0-9-]*)(?=\s*(?:[,}]|\.\.\.|$))"
    )

    # Pattern matching ALL recognized tokens: explicit values, identifiers, extension, delimiters
    # Used to verify we've parsed everything - any unmatched content is an error
    _RECOGNIZED_TOKEN: ClassVar[Pattern[str]] = re_compile(
        r"\w+\s*\(\s*\d+\s*\)"  # explicit: name(123)
        r"|[a-zA-Z][a-zA-Z0-9-]*"  # implicit: identifier
        r"|\.{3}"  # extension marker
        r"|--[^\n]*"  # ASN.1 comments (-- to end of line)
        r"|[,\s]+"  # delimiters and whitespace
    )

    @property
    def value_count(self) -> int:
        """Count the number of enumeration values.

        Returns:
            Number of defined enumeration values.

        Examples:
            >>> EnumeratedType(values={"a": 0, "b": 1, "c": 2}, is_extensible=False).value_count
            3
        """
        return len(self.values)

    @property
    def uper_bit_width(self) -> int:
        """Calculate UPER bit-width for the enumeration index.

        Per ITU-T X.691 Section 14 (Encoding of an enumerated type):
        - Non-extensible: Uses constrained whole number encoding with
          range equal to the number of enumeration values.
        - Extensible: Uses a 1-bit extension flag followed by the
          root enumeration index if in root, or semi-constrained
          whole number if in extension.

        For bit-width estimation, we use ceil(log2(count)) for non-extensible,
        and ensure at least 1 bit for extensible (the extension flag).

        Returns:
            Number of bits for the enumeration encoding.

        Examples:
            >>> EnumeratedType(values={"a": 0, "b": 1}, is_extensible=False).uper_bit_width
            1
            >>> e = EnumeratedType(values={"a": 0, "b": 1, "c": 2, "d": 3}, is_extensible=False)
            >>> e.uper_bit_width
            2
            >>> EnumeratedType(values={"a": 0}, is_extensible=True).uper_bit_width
            1
        """
        if self.value_count <= 1:
            base_bits = 0
        else:
            base_bits = ceil(log2(self.value_count))

        # Extensible enumerations need a 1-bit extension flag
        if self.is_extensible:
            return max(1, base_bits)  # At least 1 bit for extension
        return base_bits

    @classmethod
    def _extract_explicit_values(
        cls,
        body: str,
    ) -> tuple[dict[str, int], dict[int, tuple[str, int]]]:
        """Extract explicit enum values with their positions from body text.

        Scans for patterns like "name (42)" and returns both a name->value mapping
        and a position->(name, value) mapping for later merge with implicit values.

        Args:
            body: The body text inside ENUMERATED { ... }.

        Returns:
            Tuple of:
                - explicit_values: Maps enum name to its explicit integer value.
                - explicit_positions: Maps character position to (name, value) tuple.

        Examples:
            >>> vals, pos = EnumeratedType._extract_explicit_values("off (0), on (1)")
            >>> vals
            {'off': 0, 'on': 1}
            >>> len(pos)  # Two positions recorded
            2
            >>> vals, _ = EnumeratedType._extract_explicit_values("a (5), b (10)")
            >>> vals
            {'a': 5, 'b': 10}
        """
        explicit_values: dict[str, int] = {}
        explicit_positions: dict[int, tuple[str, int]] = {}
        for val_match in cls._VALUE_PATTERN.finditer(body):
            name = val_match.group(1)
            explicit_values[name] = int(val_match.group(2))
            explicit_positions[val_match.start()] = (name, explicit_values[name])
        return explicit_values, explicit_positions

    @classmethod
    def _extract_implicit_names(
        cls,
        body: str,
        explicit_values: dict[str, int],
    ) -> list[tuple[int, str]]:
        """Extract implicit enum names (without explicit values) from body text.

        Finds identifiers that appear without an explicit "(n)" value assignment.
        Skips names already in explicit_values to avoid double-counting.

        Args:
            body: The body text inside ENUMERATED { ... }.
            explicit_values: Already-extracted explicit values to skip.

        Returns:
            List of (position, name) tuples for implicit values, sorted by position.

        Examples:
            >>> EnumeratedType._extract_implicit_names("low, medium, high", {})
            [(0, 'low'), (3, 'medium'), (11, 'high')]
            >>> EnumeratedType._extract_implicit_names("a (0), b, c", {"a": 0})
            [(5, 'b'), (8, 'c')]
        """
        implicit_names: list[tuple[int, str]] = []
        for imp_match in cls._IMPLICIT_VALUE_PATTERN.finditer(body):
            name = imp_match.group(1)
            if name not in explicit_values:
                implicit_names.append((imp_match.start(), name))
        return implicit_names

    @staticmethod
    def _assign_values(all_items: list[tuple[int, str, int | None]]) -> dict[str, int]:
        """Assign integer values to enum items in document order.

        For items with explicit values, uses that value. For implicit items,
        continues from the last assigned value + 1 (or 0 if none assigned yet).

        This implements ASN.1 enumeration semantics where implicit values
        auto-increment from the previous value.

        Args:
            all_items: List of (position, name, explicit_value_or_None) tuples,
                       must be sorted by position.

        Returns:
            Dict mapping enum names to their final assigned values.

        Examples:
            >>> EnumeratedType._assign_values([(0, "a", 0), (5, "b", None), (10, "c", None)])
            {'a': 0, 'b': 1, 'c': 2}
            >>> EnumeratedType._assign_values([(0, "x", 5), (5, "y", None), (10, "z", 10)])
            {'x': 5, 'y': 6, 'z': 10}
            >>> EnumeratedType._assign_values([(0, "p", None), (5, "q", None)])
            {'p': 0, 'q': 1}
        """
        values: dict[str, int] = {}
        next_implicit = 0
        for _, name, explicit_value in all_items:
            if explicit_value is not None:
                values[name] = explicit_value
                next_implicit = explicit_value + 1
            else:
                values[name] = next_implicit
                next_implicit += 1
        return values

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse ENUMERATED type from ASN.1 definition.

        Supports both explicit values (name (n)) and implicit values (name only).
        For implicit values, assignments continue from the last explicit value + 1,
        or from 0 if no explicit value has been seen.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            EnumeratedType if successfully parsed, None otherwise.

        Examples:
            >>> e = EnumeratedType.from_asn1("ENUMERATED { off (0), on (1), auto (2) }")
            >>> e.values
            {'off': 0, 'on': 1, 'auto': 2}
            >>> EnumeratedType.from_asn1("ENUMERATED { a (0), ... }").is_extensible
            True
            >>> e = EnumeratedType.from_asn1("ENUMERATED { low, medium, high }")
            >>> e.values
            {'low': 0, 'medium': 1, 'high': 2}
            >>> e = EnumeratedType.from_asn1("ENUMERATED { a (0), b, c (5), d }")
            >>> e.values
            {'a': 0, 'b': 1, 'c': 5, 'd': 6}
        """
        match = cls.PATTERN.search(raw_def)
        if not match:
            return None

        body = match.group(1)

        # Verify all content is recognized - fail on any unrecognized tokens
        # This catches malformed input like neg(-1), name(0xFF), garbage, etc.
        if cls._RECOGNIZED_TOKEN.sub("", body).strip():
            # Unrecognized content found - fail loudly rather than silently ignore
            return None

        # Extract explicit and implicit values using helper methods
        explicit_values, explicit_positions = cls._extract_explicit_values(body)
        implicit_names = cls._extract_implicit_names(body, explicit_values)

        # If no values found at all, return None
        if not explicit_values and not implicit_names:
            return None

        # Merge and assign values in order
        # Create a list of all items sorted by position
        all_items: list[tuple[int, str, int | None]] = []  # (pos, name, explicit_value or None)

        for pos, (name, value) in explicit_positions.items():
            all_items.append((pos, name, value))

        for pos, name in implicit_names:
            all_items.append((pos, name, None))

        # Sort by position
        all_items.sort(key=lambda x: x[0])

        # Assign values in document order
        values = cls._assign_values(all_items)

        return cls(values=values, is_extensible=_ASN1_EXTENSION_MARKER in body)


@dataclass(frozen=True, kw_only=True, slots=True)
class IA5StringConstraint(UPERConstraint):
    """Represents an ASN.1 IA5String size constraint.

    IA5String is an ASCII string (International Alphabet No. 5, same as ASCII).
    Each character is encoded using 7 bits in UPER.

    Attributes:
        min_size: Minimum number of characters (must be >= 0).
        max_size: Maximum number of characters (must be >= min_size).

    Raises:
        ValueError: If min_size < 0 or max_size < min_size.

    Examples:
        >>> c = IA5StringConstraint(min_size=1, max_size=63)
        >>> c.is_fixed_size
        False
        >>> c.uper_bit_width is None
        True
        >>> c = IA5StringConstraint(min_size=10, max_size=10)
        >>> c.is_fixed_size
        True
        >>> c.uper_bit_width
        70
    """

    min_size: _NonNegativeInt
    max_size: _NonNegativeInt

    def __post_init__(self) -> None:
        """Validate size constraints."""
        if self.min_size < 0:
            raise ValueError(f"IA5StringConstraint.min_size must be >= 0, got {self.min_size}")
        if self.max_size < self.min_size:
            raise ValueError(
                f"IA5StringConstraint.max_size must be >= min_size, "
                f"got max_size={self.max_size} < min_size={self.min_size}"
            )

    # 7 bits per IA5 character (ASCII subset)
    _BITS_PER_CHAR: ClassVar[int] = 7

    # Regex pattern: IA5String (SIZE(n)) or SIZE(min..max)
    PATTERN: ClassVar[Pattern[str]] = re_compile(
        r"IA5String\s*\(\s*SIZE\s*\(\s*(\d+)(?:\s*\.\.\s*(\d+))?\s*\)\s*\)"
    )

    @property
    def is_fixed_size(self) -> bool:
        """Check if the constraint specifies a fixed size.

        Returns:
            True if min_size equals max_size.

        Examples:
            >>> IA5StringConstraint(min_size=16, max_size=16).is_fixed_size
            True
            >>> IA5StringConstraint(min_size=1, max_size=255).is_fixed_size
            False
        """
        return self.min_size == self.max_size

    @property
    def uper_bit_width(self) -> int | None:
        """Calculate UPER bit-width for fixed-size IA5Strings.

        Per ITU-T X.691, IA5String with known constraint alphabet uses
        7 bits per character. Variable-length strings use length determinant.

        Returns:
            Bit-width if fixed size (7 * length), None if variable.

        Examples:
            >>> IA5StringConstraint(min_size=10, max_size=10).uper_bit_width
            70
            >>> IA5StringConstraint(min_size=1, max_size=63).uper_bit_width is None
            True
        """
        if self.is_fixed_size:
            return self.min_size * self._BITS_PER_CHAR
        return None

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse IA5String constraint from ASN.1 definition.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            IA5StringConstraint if successfully parsed, None otherwise.

        Examples:
            >>> IA5StringConstraint.from_asn1("IA5String (SIZE(1..63))")
            IA5StringConstraint(min_size=1, max_size=63)
            >>> IA5StringConstraint.from_asn1("IA5String (SIZE(16))")
            IA5StringConstraint(min_size=16, max_size=16)
            >>> IA5StringConstraint.from_asn1("IA5String") is None
            True
        """
        match = cls.PATTERN.search(raw_def)
        if match:
            min_size = int(match.group(1))
            max_size = int(match.group(2)) if match.group(2) else min_size
            return cls(min_size=min_size, max_size=max_size)
        return None


@dataclass(frozen=True, kw_only=True, slots=True)
class IntegerConstraint(UPERConstraint):
    """Represents an ASN.1 INTEGER constraint range.

    Attributes:
        min_value: The minimum allowed value (inclusive).
        max_value: The maximum allowed value (inclusive, must be >= min_value).

    Raises:
        ValueError: If max_value < min_value.

    Examples:
        >>> c = IntegerConstraint(min_value=-900000000, max_value=900000001)
        >>> c.range_size
        1800000002
        >>> c.uper_bit_width
        31
    """

    min_value: int
    max_value: int

    def __post_init__(self) -> None:
        """Validate that max_value >= min_value."""
        if self.max_value < self.min_value:
            raise ValueError(
                f"IntegerConstraint.max_value must be >= min_value, "
                f"got max_value={self.max_value} < min_value={self.min_value}"
            )

    # Regex pattern: INTEGER (min..max)
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"INTEGER\s*\(\s*(-?\d+)\s*\.\.\s*(-?\d+)\s*\)")

    @property
    def range_size(self) -> int:
        """Calculate the number of distinct values in the range.

        Returns:
            The count of integers from min to max inclusive.

        Examples:
            >>> IntegerConstraint(min_value=0, max_value=127).range_size
            128
            >>> IntegerConstraint(min_value=-100, max_value=100).range_size
            201
        """
        return self.max_value - self.min_value + 1

    @property
    def uper_bit_width(self) -> int:
        """Calculate UPER bit-width per ITU-T X.691.

        For constrained whole numbers, the encoding uses the minimum
        number of bits to represent (max - min).

        Returns:
            Number of bits required in UPER encoding.

        Examples:
            >>> IntegerConstraint(min_value=0, max_value=127).uper_bit_width
            7
            >>> IntegerConstraint(min_value=0, max_value=255).uper_bit_width
            8
            >>> IntegerConstraint(min_value=0, max_value=65535).uper_bit_width
            16
            >>> IntegerConstraint(min_value=-900000000, max_value=900000001).uper_bit_width
            31
        """
        if self.range_size <= 1:
            return 0  # Single value, no bits needed
        return ceil(log2(self.range_size))

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse INTEGER constraint from ASN.1 definition.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            IntegerConstraint if successfully parsed, None otherwise.

        Examples:
            >>> IntegerConstraint.from_asn1("INTEGER (0..127)")
            IntegerConstraint(min_value=0, max_value=127)
            >>> IntegerConstraint.from_asn1("INTEGER (-900000000..900000001)")
            IntegerConstraint(min_value=-900000000, max_value=900000001)
            >>> IntegerConstraint.from_asn1("INTEGER") is None
            True
        """
        match = cls.PATTERN.search(raw_def)
        if match:
            return cls(
                min_value=int(match.group(1)),
                max_value=int(match.group(2)),
            )
        return None


@dataclass(frozen=True, kw_only=True, slots=True)
class OctetStringConstraint(UPERConstraint):
    """Represents an ASN.1 OCTET STRING size constraint.

    Attributes:
        min_size: Minimum number of octets (must be >= 0).
        max_size: Maximum number of octets (must be >= min_size, or None if unbounded).

    Raises:
        ValueError: If min_size < 0 or max_size < min_size (when max_size is not None).

    Examples:
        >>> c = OctetStringConstraint(min_size=4, max_size=4)
        >>> c.is_fixed_size
        True
        >>> c.uper_bit_width
        32
    """

    min_size: _NonNegativeInt
    max_size: _NonNegativeInt | None

    def __post_init__(self) -> None:
        """Validate size constraints."""
        if self.min_size < 0:
            raise ValueError(f"OctetStringConstraint.min_size must be >= 0, got {self.min_size}")
        if self.max_size is not None and self.max_size < self.min_size:
            raise ValueError(
                f"OctetStringConstraint.max_size must be >= min_size, "
                f"got max_size={self.max_size} < min_size={self.min_size}"
            )

    # Regex pattern: OCTET STRING (SIZE(n)) or SIZE(min..max)
    PATTERN: ClassVar[Pattern[str]] = re_compile(
        r"OCTET\s+STRING\s*\(\s*SIZE\s*\(\s*(\d+)(?:\s*\.\.\s*(\d+))?\s*\)\s*\)"
    )

    @property
    def is_fixed_size(self) -> bool:
        """Check if the constraint specifies a fixed size.

        Returns:
            True if min_size equals max_size.

        Examples:
            >>> OctetStringConstraint(min_size=4, max_size=4).is_fixed_size
            True
            >>> OctetStringConstraint(min_size=1, max_size=8).is_fixed_size
            False
        """
        return self.max_size is not None and self.min_size == self.max_size

    @property
    def uper_bit_width(self) -> int | None:
        """Calculate UPER bit-width for fixed-size OCTET STRINGs.

        Returns:
            Bit-width if fixed size, None if variable.

        Examples:
            >>> OctetStringConstraint(min_size=4, max_size=4).uper_bit_width
            32
            >>> OctetStringConstraint(min_size=1, max_size=8).uper_bit_width is None
            True
        """
        if self.is_fixed_size:
            return self.min_size * _BITS_PER_BYTE
        return None

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse OCTET STRING constraint from ASN.1 definition.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            OctetStringConstraint if successfully parsed, None otherwise.

        Examples:
            >>> OctetStringConstraint.from_asn1("OCTET STRING (SIZE(4))")
            OctetStringConstraint(min_size=4, max_size=4)
            >>> OctetStringConstraint.from_asn1("OCTET STRING (SIZE(1..8))")
            OctetStringConstraint(min_size=1, max_size=8)
            >>> OctetStringConstraint.from_asn1("OCTET STRING") is None
            True
        """
        match = cls.PATTERN.search(raw_def)
        if match:
            min_size = int(match.group(1))
            max_size = int(match.group(2)) if match.group(2) else min_size
            return cls(min_size=min_size, max_size=max_size)
        return None


@dataclass(frozen=True, kw_only=True, slots=True)
class SequenceField:
    """Represents a field within an ASN.1 SEQUENCE.

    After resolution, `type` holds the actual constraint (not a string reference),
    while `type_name` preserves the original ASN.1 type name for display purposes.

    Attributes:
        name: The field identifier (e.g., "msgCnt").
        type_name: The original ASN.1 type name (e.g., "MsgCount").
        type: The resolved constraint for this field's type.
        is_optional: Whether the field is marked OPTIONAL.
        section_comment: Section comment that preceded this field.
        inline_comment: Inline comment on the same line as the field.

    Examples:
        >>> field = SequenceField(
        ...     name="count",
        ...     type_name="MsgCount",
        ...     type=IntegerConstraint(min_value=0, max_value=127),
        ...     is_optional=False,
        ...     section_comment="",
        ...     inline_comment="The message count",
        ... )
        >>> field.name
        'count'
        >>> field.type_name
        'MsgCount'
        >>> field.type.uper_bit_width
        7
    """

    name: str
    type_name: str
    type: UPERConstraint
    is_optional: bool
    section_comment: str
    inline_comment: str

    def evolve(self, **kwargs: object) -> Self:
        """Create a copy with specified fields replaced.

        This is useful during type resolution to replace TypeReference
        with the actual resolved constraint.

        Args:
            **kwargs: Field names and new values to replace.

        Returns:
            New SequenceField with updated fields.

        Examples:
            >>> field = SequenceField(
            ...     name="count",
            ...     type_name="MsgCount",
            ...     type=TypeReference(name="MsgCount"),
            ...     is_optional=False,
            ...     section_comment="",
            ...     inline_comment="",
            ... )
            >>> resolved = field.evolve(type=IntegerConstraint(min_value=0, max_value=127))
            >>> resolved.type.uper_bit_width
            7
            >>> resolved.name  # Other fields unchanged
            'count'
        """
        return replace(self, **kwargs)  # type: ignore[arg-type]

    @classmethod
    def from_asn1(cls, raw_def: str) -> tuple[Self, ...]:
        """Parse SEQUENCE fields from an ASN.1 definition.

        Extracts field names, types, optionality, and comments from a SEQUENCE
        definition string. Fields are created with TypeReference placeholders
        that are resolved later.

        Args:
            raw_def: The raw ASN.1 SEQUENCE definition.

        Returns:
            Tuple of SequenceField objects with TypeReference types.

        Examples:
            >>> fields = SequenceField.from_asn1(
            ...     "SEQUENCE { msgCnt MsgCount, id TemporaryID OPTIONAL }"
            ... )
            >>> len(fields)
            2
            >>> fields[0].name
            'msgCnt'
            >>> fields[0].type_name
            'MsgCount'
            >>> fields[0].type.name  # TypeReference
            'MsgCount'
            >>> fields[1].is_optional
            True

            Section comments are attached to the following field:

            >>> fields = SequenceField.from_asn1('''SEQUENCE {
            ...     flagA BOOLEAN,
            ...     -- Safety indicators
            ...     flagB BOOLEAN OPTIONAL,
            ...     flagC BOOLEAN
            ... }''')
            >>> len(fields)
            3
            >>> fields[0].name, fields[0].section_comment
            ('flagA', '')
            >>> fields[1].name, fields[1].section_comment
            ('flagB', 'Safety indicators')
            >>> fields[2].name, fields[2].section_comment
            ('flagC', '')

            Inline comments are captured separately:

            >>> fields = SequenceField.from_asn1(
            ...     "SEQUENCE { id VehicleID, -- The vehicle identifier\\n name Name }"
            ... )
            >>> fields[0].inline_comment
            'The vehicle identifier'
            >>> fields[1].inline_comment
            ''

            Both comment types can coexist:

            >>> fields = SequenceField.from_asn1('''SEQUENCE {
            ...     -- Group header
            ...     value Count, -- The count value
            ... }''')
            >>> fields[0].section_comment
            'Group header'
            >>> fields[0].inline_comment
            'The count value'
        """
        # TypeReference is defined later in this file, use late binding
        # Find content between outermost braces (handles nested braces like {{Reg-X}})
        body = _extract_outermost_braces(raw_def)
        if body is None:
            return ()

        fields: list[Self] = []
        pending_section_comment: str = ""

        # Process line by line to correctly handle comments
        for line in body.splitlines():
            line = line.strip()
            if not line:
                continue

            # Check if this is a standalone comment line (starts with --)
            if line.startswith(_ASN1_COMMENT_PREFIX):
                pending_section_comment = line[
                    len(_ASN1_COMMENT_PREFIX) :  # noqa: E203  # Black VS flake8
                ].strip()
                continue

            # Extract inline comment from end of line
            line_inline_comment: str = ""
            comment_pos = line.find(_ASN1_COMMENT_PREFIX)
            if comment_pos != -1:
                line_inline_comment = line[
                    comment_pos + len(_ASN1_COMMENT_PREFIX) :  # noqa: E203  # Black VS flake8
                ].strip()
                line = line[:comment_pos].strip()

            # Split line by field separator to get individual fields
            parts = [p.strip() for p in line.split(_ASN1_FIELD_SEPARATOR) if p.strip()]

            # Process each field part
            for index, part in enumerate(parts):
                # Skip extension markers
                if part == _ASN1_EXTENSION_MARKER:
                    continue

                # Parse "fieldName TypeName OPTIONAL" or "fieldName TypeName"
                tokens = part.split()
                if len(tokens) < 2:
                    continue

                is_optional = _ASN1_OPTIONAL_KEYWORD in tokens[2:] if len(tokens) > 2 else False

                # Inline comment only applies to the LAST field on the line
                inline_comment = line_inline_comment if index == len(parts) - 1 else ""

                fields.append(
                    cls(
                        name=tokens[0],
                        type_name=tokens[1],
                        type=TypeReference(name=tokens[1]),
                        is_optional=is_optional,
                        section_comment=pending_section_comment,
                        inline_comment=inline_comment,
                    )
                )

                # Reset section comment after attaching to first field on line
                if index == 0:
                    pending_section_comment = ""

        return tuple(fields)


@dataclass(frozen=True, kw_only=True, slots=True)
class SequenceOfType(UPERConstraint):
    """Represents an ASN.1 SEQUENCE OF type.

    A SEQUENCE OF is a list/array of elements of a single type.
    In UPER encoding, the count is encoded first (using length determinant),
    followed by each element.

    Attributes:
        element_type: The constraint for each element in the sequence.
        min_size: Minimum number of elements (must be >= 0).
        max_size: Maximum number of elements (must be >= min_size).

    Raises:
        ValueError: If min_size < 0 or max_size < min_size.

    Examples:
        >>> sof = SequenceOfType(
        ...     element_type=IntegerConstraint(min_value=0, max_value=255),
        ...     min_size=1,
        ...     max_size=10,
        ... )
        >>> sof.uper_bit_width is None  # Variable length
        True
    """

    element_type: UPERConstraint
    min_size: _NonNegativeInt
    max_size: _NonNegativeInt

    def __post_init__(self) -> None:
        """Validate size constraints."""
        if self.min_size < 0:
            raise ValueError(f"SequenceOfType.min_size must be >= 0, got {self.min_size}")
        if self.max_size < self.min_size:
            raise ValueError(
                f"SequenceOfType.max_size must be >= min_size, "
                f"got max_size={self.max_size} < min_size={self.min_size}"
            )

    # SEQUENCE OF is detected by pattern in parser
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"^$")  # Never matches

    @property
    def uper_bit_width(self) -> int | None:
        """Return None - SEQUENCE OF has variable length.

        Returns:
            Always None (variable length encoding).

        Examples:
            >>> SequenceOfType(
            ...     element_type=IntegerConstraint(min_value=0, max_value=7),
            ...     min_size=1,
            ...     max_size=5,
            ... ).uper_bit_width is None
            True
        """
        return None

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """SequenceOfType is not directly parsed here - requires resolution.

        Args:
            raw_def: The raw ASN.1 type definition string (ignored).

        Returns:
            Always None - SequenceOfType is constructed after resolution.
        """
        return None


@dataclass(frozen=True, kw_only=True, slots=True)
class SequenceType(UPERConstraint):
    """Represents an ASN.1 SEQUENCE type.

    A SEQUENCE is a structured type containing ordered fields, each with
    its own type. In UPER encoding, fields are encoded in order without
    delimiters.

    Attributes:
        fields: Ordered tuple of SequenceField objects.
        is_extensible: Whether the SEQUENCE has an extension marker (...).

    Examples:
        >>> seq = SequenceType(fields=(
        ...     SequenceField(
        ...         name="a",
        ...         type_name="TypeA",
        ...         type=IntegerConstraint(min_value=0, max_value=127),
        ...         is_optional=False,
        ...         section_comment="",
        ...         inline_comment="",
        ...     ),
        ...     SequenceField(
        ...         name="b",
        ...         type_name="TypeB",
        ...         type=IntegerConstraint(min_value=0, max_value=255),
        ...         is_optional=False,
        ...         section_comment="",
        ...         inline_comment="",
        ...     ),
        ... ), is_extensible=False)
        >>> seq.uper_bit_width
        15
    """

    fields: tuple[SequenceField, ...]
    is_extensible: bool

    # SEQUENCE is detected by keyword, not parsed here
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"^$")  # Never matches

    @property
    def uper_bit_width(self) -> int | None:
        """Calculate total UPER bit-width for all fields.

        Returns:
            Sum of all field bit-widths if all are fixed, non-optional,
            and the sequence is non-extensible. None otherwise.

        Examples:
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="x",
            ...         type_name="SmallInt",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=False,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=False).uper_bit_width
            3
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="x",
            ...         type_name="SmallInt",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=True,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=False).uper_bit_width is None
            True
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="x",
            ...         type_name="SmallInt",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=False,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=True).uper_bit_width is None
            True
        """
        if self.is_extensible:
            return None
        if any(f.is_optional for f in self.fields):
            return None
        total = 0
        for field in self.fields:
            width = field.type.uper_bit_width
            if width is None:
                return None
            total += width
        return total

    @property
    def root_uper_bit_width(self) -> int | None:
        """Calculate root component bit-width (preamble + all non-OPTIONAL fields).

        For extensible SEQUENCEs, this returns the size of the root portion only,
        not including any extension data. Used to generate ROOT_SIZE_BITS constants.

        Only includes non-OPTIONAL fields because OPTIONAL fields contribute
        variable width based on presence bits.

        Returns:
            Sum of preamble_bits + non-OPTIONAL field widths if all have fixed
            bit-widths, None otherwise.

        Examples:
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="x",
            ...         type_name="SmallInt",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=False,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=True).root_uper_bit_width
            4
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="x",
            ...         type_name="SmallInt",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=True,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=True).root_uper_bit_width is None
            True
        """
        # OPTIONAL fields make the root size variable
        if any(f.is_optional for f in self.fields):
            return None
        total = self.preamble_bits
        for field in self.fields:
            width = field.type.uper_bit_width
            if width is None:
                return None
            total += width
        return total

    @property
    def optional_count(self) -> int:
        """Count of OPTIONAL fields in this SEQUENCE.

        Returns:
            Number of fields marked as OPTIONAL.

        Examples:
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="a",
            ...         type_name="TypeA",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=False,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ...     SequenceField(
            ...         name="b",
            ...         type_name="TypeB",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=True,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=False).optional_count
            1
        """
        return sum(1 for f in self.fields if f.is_optional)

    @property
    def extension_bit(self) -> int:
        """Extension bit count: 1 if extensible, 0 otherwise.

        Per UPER spec, extensible types have a 1-bit extension flag.

        Returns:
            1 if is_extensible, 0 otherwise.

        Examples:
            >>> SequenceType(fields=(), is_extensible=False).extension_bit
            0
            >>> SequenceType(fields=(), is_extensible=True).extension_bit
            1
        """
        return 1 if self.is_extensible else 0

    @property
    def preamble_bits(self) -> int:
        """UPER preamble bit count for this SEQUENCE.

        For non-extensible SEQUENCE: equals optional_count (1 bit per OPTIONAL).
        For extensible SEQUENCE: extension_bit + optional_count.

        Returns:
            Total preamble bits needed.

        Examples:
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="a",
            ...         type_name="TypeA",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=True,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ...     SequenceField(
            ...         name="b",
            ...         type_name="TypeB",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=True,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=False).preamble_bits
            2
            >>> SequenceType(fields=(
            ...     SequenceField(
            ...         name="a",
            ...         type_name="TypeA",
            ...         type=IntegerConstraint(min_value=0, max_value=7),
            ...         is_optional=True,
            ...         section_comment="",
            ...         inline_comment="",
            ...     ),
            ... ), is_extensible=True).preamble_bits
            2
        """
        return self.extension_bit + self.optional_count

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """Parse a SEQUENCE type from an ASN.1 definition.

        Creates a SequenceType with fields containing TypeReference placeholders
        that will be resolved later.

        Args:
            raw_def: The raw ASN.1 SEQUENCE definition.

        Returns:
            SequenceType with parsed fields, or None if not a valid SEQUENCE.

        Examples:
            >>> seq = SequenceType.from_asn1("SEQUENCE { a TypeA, b TypeB }")
            >>> seq is not None
            True
            >>> len(seq.fields)
            2
            >>> seq.fields[0].name
            'a'
            >>> seq.fields[0].type_name
            'TypeA'
            >>> seq.is_extensible
            False
            >>> SequenceType.from_asn1("SEQUENCE { a TypeA, ... }").is_extensible
            True
            >>> SequenceType.from_asn1("CHOICE { a A }") is None
            True
        """
        # Must start with SEQUENCE keyword (not CHOICE, etc.)
        if not raw_def.strip().startswith(_ASN1_SEQUENCE_KEYWORD):
            return None
        fields = SequenceField.from_asn1(raw_def)
        if not fields:
            return None
        is_extensible = _ASN1_EXTENSION_MARKER in raw_def
        return cls(fields=fields, is_extensible=is_extensible)


@dataclass(frozen=True, kw_only=True, slots=True)
class TypeReference(UPERConstraint):
    """Placeholder for an unresolved type reference.

    During initial parsing, SEQUENCE fields reference other types by name.
    This placeholder holds the name until the resolution phase replaces
    it with the actual constraint.

    Attributes:
        name: The referenced type name (e.g., "MsgCount", "TemporaryID").

    Examples:
        >>> ref = TypeReference(name="MsgCount")
        >>> ref.name
        'MsgCount'
        >>> ref.uper_bit_width is None
        True
    """

    name: str

    # TypeReference is created programmatically, not parsed
    PATTERN: ClassVar[Pattern[str]] = re_compile(r"^$")  # Never matches

    @property
    def uper_bit_width(self) -> int | None:
        """Return None - unresolved references have unknown bit-width.

        Returns:
            Always None until resolved.

        Examples:
            >>> TypeReference(name="SomeType").uper_bit_width is None
            True
        """
        return None

    @classmethod
    def from_asn1(cls, raw_def: str) -> Self | None:
        """TypeReference is not parsed from ASN.1 - always returns None.

        Args:
            raw_def: The raw ASN.1 type definition string (ignored).

        Returns:
            Always None - TypeReferences are created programmatically.
        """
        return None
