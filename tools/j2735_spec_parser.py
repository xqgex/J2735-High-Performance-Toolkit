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
J2735 ASN.1 Specification Parser.

A parser that converts SAE J2735 specification text files
into structured Python data types suitable for C code generation.

This module provides:
    - Immutable data structures for ASN.1 type definitions
    - UPER bit-width calculation per ITU-T X.691
"""

from collections.abc import Mapping
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path
from re import DOTALL, MULTILINE, Pattern
from re import compile as re_compile
from typing import Final, Self

from .j2735_asn1_constants import (
    ASN1_COMMENT_PREFIX,
    ASN1_TYPE_DEF_DOTALL_PATTERN,
    ASN1_TYPE_DEF_PATTERN,
)
from .j2735_spec_constraints import (
    BitStringConstraint,
    BooleanType,
    ChoiceType,
    EnumeratedType,
    IA5StringConstraint,
    IntegerConstraint,
    OctetStringConstraint,
    SequenceOfType,
    SequenceType,
    TypeReference,
    UPERConstraint,
)

# =============================================================================
# Constants - ASN.1 Language Elements
# =============================================================================

# Parser constants
_COMMENT_BIT_KEYWORD: Final[str] = "bit"
_COMMENT_SIZE_KEYWORD: Final[str] = "size"
_DEFAULT_VERSION: Final[str] = "unknown"

# =============================================================================
# Constants - Regex Patterns (Private)
# =============================================================================

# Section marker pattern (e.g., "<MARK_BEGINNING_SECTION_5>")
_SECTION_MARKER_PATTERN: Final[Pattern[str]] = re_compile(r"^<MARK_BEGINNING_SECTION_(\d+)>$")

# SEQUENCE OF pattern
_SEQUENCE_OF_PATTERN: Final[Pattern[str]] = re_compile(
    r"SEQUENCE\s*\(\s*SIZE\s*\(\s*(\d+)\s*\.\.\s*(\d+)\s*\)\s*\)\s*OF\s+(\w+)"
)

# Section header patterns
_MESSAGE_HEADER_PATTERN: Final[Pattern[str]] = re_compile(
    r"^(\d+\.\d+)\s+Message:\s+MSG_(\w+)\s*(?:\((\w+)\))?",
    MULTILINE,
)
_DATAFRAME_HEADER_PATTERN: Final[Pattern[str]] = re_compile(
    r"^(\d+\.\d+)\s+Data\s+Frame:\s+DF_(\w+)",
    MULTILINE,
)
_DATAELEMENT_HEADER_PATTERN: Final[Pattern[str]] = re_compile(
    r"^(\d+\.\d+)\s+Data\s+Element:\s+DE_(\w+)",
    MULTILINE,
)

# Block extraction patterns
_USE_BLOCK_PATTERN: Final[Pattern[str]] = re_compile(r"Use:\s*(.+?)(?=ASN\.1|$)", DOTALL)
_ASN1_REPR_BLOCK_PATTERN: Final[Pattern[str]] = re_compile(
    r"ASN\.1\s+Representation:\s*\n(.+?)(?=Remarks:|<MARK|$)", DOTALL
)
_REMARKS_BLOCK_PATTERN: Final[Pattern[str]] = re_compile(r"Remarks:\s*(.+?)(?=<MARK|$)", DOTALL)

# Version extraction pattern (6 digits like "202409")
_VERSION_PATTERN: Final[Pattern[str]] = re_compile(r"(\d{6})")


# =============================================================================
# Enumerations
# =============================================================================


class J2735EntryKind(Enum):
    """Classification of J2735 specification entry types.

    J2735 defines three categories of ASN.1 types, each in their own section:
        - MESSAGES: Top-level message types (MSG_*) - Section 5
        - DATA_FRAMES: Composite data structures (DF_*) - Section 6
        - DATA_ELEMENTS: Primitive/simple types (DE_*) - Section 7

    Values are the actual section numbers from the J2735 specification document.
    """

    MESSAGES = 5  # Section 5: Messages
    DATA_FRAMES = 6  # Section 6: Data Frames
    DATA_ELEMENTS = 7  # Section 7: Data Elements


class ASN1TypeClass(Enum):
    """ASN.1 built-in type classifications for UPER encoding.

    Each type class has different UPER encoding rules that affect
    bit-width calculation.
    """

    BIT_STRING = "BIT STRING"  # Fixed or variable bit sequence
    BOOLEAN = "BOOLEAN"  # Single bit
    CHOICE = "CHOICE"  # Union/variant type
    ENUMERATED = "ENUMERATED"  # Index into enumeration
    IA5_STRING = "IA5String"  # ASCII string (7-bit chars)
    INTEGER = "INTEGER"  # Constrained integer
    OCTET_STRING = "OCTET STRING"  # Fixed or variable byte sequence
    SEQUENCE = "SEQUENCE"  # Structured type (record)
    SEQUENCE_OF = auto()  # List/array type
    TYPE_REFERENCE = auto()  # Reference to another defined type

    def __repr__(self) -> str:
        return f"<{self.__class__.__name__}.{self.name}>"

    @classmethod
    def from_definition(cls, raw_def: str) -> "ASN1TypeClass":
        """Classify an ASN.1 type from its definition string.

        Args:
            raw_def: The raw ASN.1 type definition string.

        Returns:
            The appropriate ASN1TypeClass enumeration value.

        Examples:
            >>> ASN1TypeClass.from_definition("BOOLEAN")
            <ASN1TypeClass.BOOLEAN>
            >>> ASN1TypeClass.from_definition("INTEGER (0..127)")
            <ASN1TypeClass.INTEGER>
            >>> ASN1TypeClass.from_definition("SEQUENCE { x Y, z W }")
            <ASN1TypeClass.SEQUENCE>
            >>> ASN1TypeClass.from_definition("SomeOtherType")
            <ASN1TypeClass.TYPE_REFERENCE>
        """
        stripped = raw_def.strip()

        if _SEQUENCE_OF_PATTERN.match(stripped):
            return cls.SEQUENCE_OF

        for member in cls:
            if isinstance(member.value, str) and stripped.startswith(member.value):
                return member

        # If none of the above, assume it's a reference to another defined type
        return cls.TYPE_REFERENCE


# =============================================================================
# Immutable Data Structures
# =============================================================================


@dataclass(frozen=True, kw_only=True, slots=True)
class ASN1TypeDefinition:
    """Complete ASN.1 type definition with parsed constraints.

    All type information is stored in the unified `constraint` field,
    which holds the appropriate constraint class for each type:
    - INTEGER → IntegerConstraint
    - BIT STRING → BitStringConstraint
    - BOOLEAN → BooleanType
    - SEQUENCE → SequenceType (with resolved SequenceField.type)
    - etc.

    Attributes:
        name: The type identifier (e.g., "Latitude", "MsgCount").
        type_class: The classification of the ASN.1 type.
        raw_definition: The original ASN.1 text.
        constraint: The parsed constraint (type varies by type_class).
        spec_section: Source section number (e.g., "7.99").
        description: The "Use:" text from the spec.

    Examples:
        >>> typedef = ASN1TypeDefinition(
        ...     name="MsgCount",
        ...     type_class=ASN1TypeClass.INTEGER,
        ...     raw_definition="INTEGER (0..127)",
        ...     constraint=IntegerConstraint(min_value=0, max_value=127),
        ...     spec_section="",
        ...     description="",
        ... )
        >>> typedef.uper_bit_width
        7
    """

    name: str
    type_class: ASN1TypeClass
    raw_definition: str  # TODO: Evaluate for generated C Doxygen (already accessible to templates)
    constraint: UPERConstraint | None
    spec_section: str  # TODO: Evaluate for generated C Doxygen (already accessible to templates)
    description: str  # TODO: Evaluate for generated C Doxygen (already accessible to templates)

    @property
    def uper_bit_width(self) -> int | None:
        """Calculate UPER bit-width if determinable.

        Returns:
            Bit-width for primitive types and resolved SEQUENCEs,
            None for variable/unresolved types.

        Examples:
            >>> ASN1TypeDefinition(
            ...     name="Test",
            ...     type_class=ASN1TypeClass.BOOLEAN,
            ...     raw_definition="BOOLEAN",
            ...     constraint=BooleanType(),
            ...     spec_section="",
            ...     description="",
            ... ).uper_bit_width
            1
        """
        if self.constraint is None:
            return None

        return self.constraint.uper_bit_width

    @classmethod
    def from_asn1(cls, name: str, raw_def: str, **kwargs: str) -> Self:
        """Parse a complete ASN.1 type definition.

        Factory method to create an ASN1TypeDefinition from raw ASN.1 text.
        For SEQUENCE types, fields initially contain TypeReference placeholders
        that are resolved later.

        Args:
            name: The type name.
            raw_def: The raw ASN.1 definition string.
            **kwargs: Additional metadata fields (spec_section, description).

        Returns:
            A fully parsed ASN1TypeDefinition.

        Examples:
            >>> typedef = ASN1TypeDefinition.from_asn1("TireTemp", "INTEGER (-8736..55519)")
            >>> typedef.type_class
            <ASN1TypeClass.INTEGER>
            >>> typedef.uper_bit_width
            16
            >>> typedef = ASN1TypeDefinition.from_asn1("Flag", "BOOLEAN")
            >>> typedef.uper_bit_width
            1
        """
        type_class = ASN1TypeClass.from_definition(raw_def)

        constraint: UPERConstraint | None = None

        if type_class == ASN1TypeClass.BIT_STRING:
            constraint = BitStringConstraint.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.BOOLEAN:
            constraint = BooleanType()
        elif type_class == ASN1TypeClass.CHOICE:
            constraint = ChoiceType.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.ENUMERATED:
            constraint = EnumeratedType.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.IA5_STRING:
            constraint = IA5StringConstraint.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.INTEGER:
            constraint = IntegerConstraint.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.OCTET_STRING:
            constraint = OctetStringConstraint.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.SEQUENCE:
            # Parse fields with TypeReference placeholders (resolved later)
            constraint = SequenceType.from_asn1(raw_def)
        elif type_class == ASN1TypeClass.SEQUENCE_OF:
            seq_match = _SEQUENCE_OF_PATTERN.match(raw_def.strip())
            if seq_match:
                min_size = int(seq_match.group(1))
                max_size = int(seq_match.group(2))
                element_type_name = seq_match.group(3)
                constraint = SequenceOfType(
                    element_type=TypeReference(name=element_type_name),
                    min_size=min_size,
                    max_size=max_size,
                )
        elif type_class == ASN1TypeClass.TYPE_REFERENCE:
            # Reference to another type - placeholder until resolution
            constraint = TypeReference(name=raw_def.strip())

        return cls(
            name=name,
            type_class=type_class,
            raw_definition=raw_def,
            constraint=constraint,
            spec_section=kwargs.get("spec_section", ""),
            description=kwargs.get("description", ""),
        )


@dataclass(frozen=True, kw_only=True, slots=True)
class SpecEntry:
    """A single entry in the J2735 specification.

    Represents a Message, Data Frame, or Data Element with its
    complete definition.

    Attributes:
        section_number: The numeric section (e.g., "5.2", "7.99").
        entry_type: Classification (MESSAGE, DATA_FRAME, DATA_ELEMENT).
        name: The entry name without prefix (e.g., "BasicSafetyMessage").
        abbreviation: Short form if present (e.g., "BSM").
        use_description: The "Use:" paragraph from the spec.
        asn1_definition: The parsed ASN.1 type definition.
        remarks: The "Remarks:" paragraph if present.
        line_number: Source file line number for error reporting (None if unknown).
    """

    section_number: str  # TODO: Evaluate for generated C Doxygen (not yet passed to templates)
    entry_type: (
        J2735EntryKind  # TODO: Evaluate for generated C Doxygen (not yet passed to templates)
    )
    name: str
    abbreviation: str  # TODO: Evaluate for generated C Doxygen (not yet passed to templates)
    use_description: str  # TODO: Evaluate for generated C Doxygen (not yet passed to templates)
    asn1_definition: ASN1TypeDefinition | None
    remarks: str  # TODO: Evaluate for generated C Doxygen (not yet passed to templates)
    line_number: int | None  # TODO: Evaluate for generated C Doxygen (not yet passed to templates)

    # -----------------------------------------------------------------
    # Block Parsing Helpers
    # -----------------------------------------------------------------

    @staticmethod
    def _extract_block_sections(block: str) -> tuple[str, str, str]:
        """Extract Use, ASN.1 text, and Remarks from a spec block.

        Each specification block (Message, Data Frame, Data Element) contains
        the same three optional sections. This helper applies the shared
        regex extraction once, returning empty strings for missing sections.

        Args:
            block: The raw text block for one specification entry.

        Returns:
            A tuple of ``(use_description, asn1_text, remarks)``.

        >>> SpecEntry._extract_block_sections(
        ...     "Use: A simple counter.\\n"
        ...     "ASN.1 Representation:\\n"
        ...     "MsgCount ::= INTEGER (0..127)\\n"
        ...     "Remarks: Wraps at 127."
        ... )
        ('A simple counter.', 'MsgCount ::= INTEGER (0..127)', 'Wraps at 127.')
        >>> SpecEntry._extract_block_sections("No sections here.")
        ('', '', '')
        """
        use_match = _USE_BLOCK_PATTERN.search(block)
        use_description = use_match.group(1).strip() if use_match else ""

        asn1_match = _ASN1_REPR_BLOCK_PATTERN.search(block)
        asn1_text = asn1_match.group(1).strip() if asn1_match else ""

        remarks_match = _REMARKS_BLOCK_PATTERN.search(block)
        remarks = remarks_match.group(1).strip() if remarks_match else ""

        return use_description, asn1_text, remarks

    @staticmethod
    def _parse_asn1_multiline(
        asn1_text: str, section_number: str, use_description: str
    ) -> ASN1TypeDefinition | None:
        """Parse ASN.1 text using multiline strategy (for Data Elements).

        Data Element definitions are single-line with optional continuation
        lines. This strategy matches the first ``name ::= definition`` line,
        then walks subsequent lines, appending non-comment content and
        comments that contain encoding info (bit/size keywords).

        Args:
            asn1_text: The raw ASN.1 representation text.
            section_number: The spec section (e.g., "7.99").
            use_description: The Use: description for this entry.

        Returns:
            The parsed type definition, or None if parsing fails.
        """
        if not asn1_text:
            return None

        type_def_match = ASN1_TYPE_DEF_PATTERN.search(asn1_text)
        if not type_def_match:
            return None

        type_body = type_def_match.group(2)
        continuation_text = asn1_text[type_def_match.end() :]  # noqa: E203  # Black VS flake8

        for line in continuation_text.splitlines():
            stripped = line.strip()
            if stripped and not stripped.startswith(ASN1_COMMENT_PREFIX):
                type_body += " " + stripped
            elif stripped.startswith(ASN1_COMMENT_PREFIX):
                if (
                    _COMMENT_BIT_KEYWORD in stripped.lower()
                    or _COMMENT_SIZE_KEYWORD in stripped.lower()
                ):
                    type_body += " " + stripped

        return ASN1TypeDefinition.from_asn1(
            type_def_match.group(1),
            type_body,
            spec_section=section_number,
            description=use_description,
        )

    @staticmethod
    def _parse_asn1_dotall(
        asn1_text: str, section_number: str, use_description: str
    ) -> ASN1TypeDefinition | None:
        """Parse ASN.1 text using DOTALL strategy (for Data Frames/Messages).

        Frame and Message definitions span multiple lines with nested braces.
        This strategy captures the full definition including newlines in a
        single regex match.

        Args:
            asn1_text: The raw ASN.1 representation text.
            section_number: The spec section (e.g., "6.10").
            use_description: The Use: description for this entry.

        Returns:
            The parsed type definition, or None if parsing fails.
        """
        if not asn1_text:
            return None

        type_def_match = ASN1_TYPE_DEF_DOTALL_PATTERN.search(asn1_text)
        if not type_def_match:
            return None

        return ASN1TypeDefinition.from_asn1(
            type_def_match.group(1),
            type_def_match.group(2).strip(),
            spec_section=section_number,
            description=use_description,
        )

    # -----------------------------------------------------------------
    # Classmethod Constructors
    # -----------------------------------------------------------------

    @classmethod
    def from_data_element_block(cls, block: str, line_offset: int) -> Self | None:
        """Parse a single Data Element block from section 7.

        Args:
            block: The text block for one Data Element.
            line_offset: The starting line number in the source file.

        Returns:
            A SpecEntry for the Data Element, or None if parsing fails.
        """
        header_match = _DATAELEMENT_HEADER_PATTERN.search(block)
        if not header_match:
            return None

        section_number = header_match.group(1)
        use_description, asn1_text, remarks = cls._extract_block_sections(block)

        return cls(
            section_number=section_number,
            entry_type=J2735EntryKind.DATA_ELEMENTS,
            name=header_match.group(2),
            abbreviation="",
            use_description=use_description,
            asn1_definition=cls._parse_asn1_multiline(asn1_text, section_number, use_description),
            remarks=remarks,
            line_number=line_offset,
        )

    @classmethod
    def from_data_frame_block(cls, block: str, line_offset: int) -> Self | None:
        """Parse a single Data Frame block from section 6.

        Args:
            block: The text block for one Data Frame.
            line_offset: The starting line number in the source file.

        Returns:
            A SpecEntry for the Data Frame, or None if parsing fails.
        """
        header_match = _DATAFRAME_HEADER_PATTERN.search(block)
        if not header_match:
            return None

        section_number = header_match.group(1)
        use_description, asn1_text, remarks = cls._extract_block_sections(block)

        return cls(
            section_number=section_number,
            entry_type=J2735EntryKind.DATA_FRAMES,
            name=header_match.group(2),
            abbreviation="",
            use_description=use_description,
            asn1_definition=cls._parse_asn1_dotall(asn1_text, section_number, use_description),
            remarks=remarks,
            line_number=line_offset,
        )

    @classmethod
    def from_message_block(cls, block: str, line_offset: int) -> Self | None:
        """Parse a single Message block from section 5.

        Args:
            block: The text block for one Message.
            line_offset: The starting line number in the source file.

        Returns:
            A SpecEntry for the Message, or None if parsing fails.
        """
        header_match = _MESSAGE_HEADER_PATTERN.search(block)
        if not header_match:
            return None

        section_number = header_match.group(1)
        use_description, asn1_text, remarks = cls._extract_block_sections(block)

        return cls(
            section_number=section_number,
            entry_type=J2735EntryKind.MESSAGES,
            name=header_match.group(2),
            abbreviation=header_match.group(3) or "",
            use_description=use_description,
            asn1_definition=cls._parse_asn1_dotall(asn1_text, section_number, use_description),
            remarks=remarks,
            line_number=line_offset,
        )


# =============================================================================
# Type Resolution
# =============================================================================


def _resolve_constraint(
    constraint: UPERConstraint | None,
    registry: Mapping[str, ASN1TypeDefinition],
    resolving: frozenset[str],
) -> UPERConstraint | None:
    """Recursively resolve TypeReference placeholders to actual constraints.

    Args:
        constraint: The constraint to resolve.
        registry: Mapping of type names to their (unresolved) definitions.
        resolving: Set of type names currently being resolved (cycle detection).

    Returns:
        The resolved constraint, or None if unresolvable.
    """
    if constraint is None:
        return None

    if isinstance(constraint, TypeReference):
        type_ref_return = None
        # Look up the referenced type
        if constraint.name in resolving:
            # TODO: Circular references silently return None. Consider:
            #   - Logging an error for debugging
            #   - Raising an exception that propagates to the caller
            #   - Making cycles impossible by construction (prove with types)
            pass  # Circular reference
        else:
            typedef = registry.get(constraint.name)
            type_ref_return = (
                constraint
                if typedef is None
                else _resolve_constraint(
                    typedef.constraint,
                    registry,
                    resolving | {constraint.name},
                )
            )
        return type_ref_return

    if isinstance(constraint, SequenceType):
        # Resolve each field's type using evolve()
        resolved_fields = tuple(
            field.evolve(type=resolved_type)
            for field in constraint.fields
            if (resolved_type := _resolve_constraint(field.type, registry, resolving) or field.type)
        )
        return SequenceType(fields=resolved_fields, is_extensible=constraint.is_extensible)

    if isinstance(constraint, SequenceOfType):
        # Resolve the element type
        resolved_element = _resolve_constraint(
            constraint.element_type,
            registry,
            resolving,
        )
        if resolved_element is None:
            resolved_element = constraint.element_type
        return SequenceOfType(
            element_type=resolved_element,
            min_size=constraint.min_size,
            max_size=constraint.max_size,
        )

    # Other constraints don't need resolution
    return constraint


def _resolve_type_registry(
    unresolved: dict[str, ASN1TypeDefinition],
) -> dict[str, ASN1TypeDefinition]:
    """Resolve all TypeReference placeholders in the type registry.

    Args:
        unresolved: Dictionary of type definitions with TypeReference placeholders.

    Returns:
        Dictionary with all resolvable references replaced by actual constraints.
    """
    resolved: dict[str, ASN1TypeDefinition] = {}

    for name, typedef in unresolved.items():
        resolved_constraint = _resolve_constraint(
            typedef.constraint,
            unresolved,
            frozenset({name}),
        )
        resolved[name] = ASN1TypeDefinition(
            name=typedef.name,
            type_class=typedef.type_class,
            raw_definition=typedef.raw_definition,
            constraint=resolved_constraint,
            spec_section=typedef.spec_section,
            description=typedef.description,
        )

    return resolved


@dataclass(frozen=True, kw_only=True, slots=True)
class J2735Specification:
    """Complete parsed J2735 specification.

    This is the top-level immutable container for all parsed data.

    Attributes:
        version: Specification version (e.g., "202409").
        messages: All MSG_* definitions.
        data_frames: All DF_* definitions.
        data_elements: All DE_* definitions.
        type_registry: Mapping of type names to their definitions.
    """

    version: str
    messages: tuple[SpecEntry, ...]
    data_frames: tuple[SpecEntry, ...]
    data_elements: tuple[SpecEntry, ...]
    type_registry: Mapping[str, ASN1TypeDefinition]

    def collect_fixed_width_types(
        self,
    ) -> tuple[list[ASN1TypeDefinition], int]:
        """Collect all types with deterministic UPER bit-widths.

        Iterates ``type_registry`` in alphabetical order and partitions
        types into those with a known fixed bit-width and those without.

        Returns:
            A tuple of ``(fixed_types, variable_count)`` where *fixed_types*
            is a sorted list of type definitions whose ``uper_bit_width`` is
            not ``None``, and *variable_count* is the number of skipped
            variable-width types.  Returns ``([], 0)`` when the registry
            is empty.

        Examples:
            >>> from tools.tests.conftest import SPEC_FILE_PATH
            >>> from tools.j2735_spec_parser import parse_spec_file
            >>> spec = parse_spec_file(SPEC_FILE_PATH)
            >>> fixed, variable = spec.collect_fixed_width_types()
            >>> all(t.uper_bit_width is not None for t in fixed)
            True
            >>> variable >= 0
            True
        """
        fixed_types: list[ASN1TypeDefinition] = []
        variable_count = 0

        for _, typedef in sorted(self.type_registry.items()):
            if typedef.uper_bit_width is not None:
                fixed_types.append(typedef)
            else:
                variable_count += 1

        return fixed_types, variable_count

    def lookup_type(self, name: str) -> ASN1TypeDefinition | None:
        """Look up a type definition by name.

        Args:
            name: The ASN.1 type name.

        Returns:
            The type definition if found, None otherwise.
        """
        return self.type_registry.get(name)

    @classmethod
    def from_content(
        cls,
        content: str,
        version: str = _DEFAULT_VERSION,
    ) -> Self:
        """Parse J2735 specification from content string.

        This is the core parsing method that processes the specification text.
        Use this for testing or when content is already available in memory.

        Args:
            content: The full specification text content.
            version: Optional version string (e.g., "202409").

        Returns:
            A J2735Specification containing all parsed definitions.

        Raises:
            ValueError: If the content format is invalid.
        """
        # Split into sections
        sections = _extract_sections(content)

        # Parse Messages (Section 5)
        messages: list[SpecEntry] = []
        if J2735EntryKind.MESSAGES.value in sections:
            blocks = _split_into_blocks(
                sections[J2735EntryKind.MESSAGES.value], _MESSAGE_HEADER_PATTERN
            )
            for line_num, block in blocks:
                entry = SpecEntry.from_message_block(block, line_num)
                if entry:
                    messages.append(entry)

        # Parse Data Frames (Section 6)
        data_frames: list[SpecEntry] = []
        if J2735EntryKind.DATA_FRAMES.value in sections:
            blocks = _split_into_blocks(
                sections[J2735EntryKind.DATA_FRAMES.value], _DATAFRAME_HEADER_PATTERN
            )
            for line_num, block in blocks:
                entry = SpecEntry.from_data_frame_block(block, line_num)
                if entry:
                    data_frames.append(entry)

        # Parse Data Elements (Section 7)
        data_elements: list[SpecEntry] = []
        if J2735EntryKind.DATA_ELEMENTS.value in sections:
            blocks = _split_into_blocks(
                sections[J2735EntryKind.DATA_ELEMENTS.value], _DATAELEMENT_HEADER_PATTERN
            )
            for line_num, block in blocks:
                entry = SpecEntry.from_data_element_block(block, line_num)
                if entry:
                    data_elements.append(entry)

        # Build type registry (unresolved - contains TypeReference placeholders)
        unresolved_registry: dict[str, ASN1TypeDefinition] = {}
        for entry in messages + data_frames + data_elements:
            if entry.asn1_definition:
                unresolved_registry[entry.asn1_definition.name] = entry.asn1_definition

        # Resolve all TypeReference placeholders
        type_registry = _resolve_type_registry(unresolved_registry)

        return cls(
            version=version,
            messages=tuple(messages),
            data_frames=tuple(data_frames),
            data_elements=tuple(data_elements),
            type_registry=type_registry,
        )


# =============================================================================
# Parser Functions
# =============================================================================


def _extract_sections(
    content: str,
) -> dict[int, list[tuple[int, str]]]:
    """Split specification content into sections by marker.

    Args:
        content: The full specification text.

    Returns:
        Dictionary mapping section number to list of (line_number, text) tuples.

    Examples:
        >>> content = "<MARK_BEGINNING_SECTION_5>\\n5. MESSAGES\\nfoo\\n"
        >>> content += "<MARK_BEGINNING_SECTION_6>\\n6. FRAMES\\nbar\\n"
        >>> sections = _extract_sections(content)
        >>> 5 in sections
        True
        >>> 6 in sections
        True
    """
    lines = content.splitlines()
    sections: dict[int, list[tuple[int, str]]] = {}
    current_section: int | None = None
    current_lines: list[tuple[int, str]] = []

    for line_num, line in enumerate(lines, start=1):
        marker_match = _SECTION_MARKER_PATTERN.match(line)
        if marker_match:
            # Save previous section if exists
            if current_section is not None and current_lines:
                if current_section not in sections:
                    sections[current_section] = []
                sections[current_section].extend(current_lines)
            # Start new section
            current_section = int(marker_match.group(1))
            current_lines = []
        elif current_section is not None:
            current_lines.append((line_num, line))

    # Don't forget the last section
    if current_section is not None and current_lines:
        if current_section not in sections:
            sections[current_section] = []
        sections[current_section].extend(current_lines)

    return sections


def _split_into_blocks(
    section_lines: list[tuple[int, str]],
    header_pattern: Pattern[str],
) -> list[tuple[int, str]]:
    """Split section content into individual entry blocks.

    Args:
        section_lines: List of (line_number, text) tuples.
        header_pattern: Regex pattern that matches entry headers.

    Returns:
        List of (start_line, block_text) tuples.
    """
    blocks: list[tuple[int, str]] = []
    current_block_lines: list[str] = []
    current_start_line = 0

    for line_num, line in section_lines:
        if header_pattern.match(line):
            # Save previous block
            if current_block_lines:
                blocks.append(
                    (
                        current_start_line,
                        "\n".join(current_block_lines),
                    )
                )
            current_block_lines = [line]
            current_start_line = line_num
        else:
            current_block_lines.append(line)

    # Don't forget the last block
    if current_block_lines:
        blocks.append(
            (
                current_start_line,
                "\n".join(current_block_lines),
            )
        )

    return blocks


def parse_spec_file(
    file_path: str | Path,
) -> J2735Specification:
    """Parse a J2735 specification text file.

    This is the main entry point for file-based parsing. It reads the
    specification file and delegates to `J2735Specification.from_content`.

    Args:
        file_path: Path to the J2735 specification text file.

    Returns:
        A J2735Specification containing all parsed definitions.

    Raises:
        FileNotFoundError: If the specification file doesn't exist.
        ValueError: If the file format is invalid.
    """
    path = Path(file_path)
    if not path.exists():
        raise FileNotFoundError(f"Specification file not found: {path}")

    content = path.read_text(encoding="utf-8")

    # Extract version from filename
    version_match = _VERSION_PATTERN.search(path.stem)
    version = version_match.group(1) if version_match else _DEFAULT_VERSION

    return J2735Specification.from_content(content, version)
