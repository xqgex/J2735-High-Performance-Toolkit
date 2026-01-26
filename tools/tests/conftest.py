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
"""Shared test fixtures and utilities for J2735 Python tests.

This module provides:
    - SPEC_FILE_PATH: Path to the J2735 specification file
    - Mock spec builders used across multiple test modules
"""

from pathlib import Path
from unittest import TestCase

from tools.j2735_c_generator_jinja import create_jinja_env, get_template
from tools.j2735_spec_constraints import (
    BitStringConstraint,
    IntegerConstraint,
    SequenceField,
    SequenceType,
)
from tools.j2735_spec_parser import (
    ASN1TypeClass,
    ASN1TypeDefinition,
    J2735Specification,
    parse_spec_file,
)

# =============================================================================
# Path Constants
# =============================================================================


# Path to the J2735 specification file (relative to repository root)
_REPOSITORY_ROOT: Path = Path(__file__).parent.parent.parent
SPEC_FILE_PATH: Path = _REPOSITORY_ROOT / "J2735_202409_pdf_content.txt"


# =============================================================================
# Mock Spec Builders
# =============================================================================


def make_nested_mock_spec() -> J2735Specification:
    """Create a mock specification with nested SEQUENCE types.

    Returns:
        J2735Specification with PositionalAccuracy (32-bit nested SEQUENCE).

    This is useful for testing recursive bit-width computation.
    """
    semi_major_constraint = IntegerConstraint(min_value=0, max_value=255)
    semi_minor_constraint = IntegerConstraint(min_value=0, max_value=255)
    orientation_constraint = IntegerConstraint(min_value=0, max_value=65535)

    semi_major = ASN1TypeDefinition(
        name="SemiMajorAxisAccuracy",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..255)",
        constraint=semi_major_constraint,
        spec_section="",
        description="",
    )
    semi_minor = ASN1TypeDefinition(
        name="SemiMinorAxisAccuracy",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..255)",
        constraint=semi_minor_constraint,
        spec_section="",
        description="",
    )
    orientation = ASN1TypeDefinition(
        name="SemiMajorAxisOrientation",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..65535)",
        constraint=orientation_constraint,
        spec_section="",
        description="",
    )

    positional_accuracy = ASN1TypeDefinition(
        name="PositionalAccuracy",
        type_class=ASN1TypeClass.SEQUENCE,
        raw_definition="SEQUENCE { ... }",
        constraint=SequenceType(
            fields=(
                SequenceField(
                    name="semiMajor",
                    type_name="SemiMajorAxisAccuracy",
                    type=semi_major_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
                SequenceField(
                    name="semiMinor",
                    type_name="SemiMinorAxisAccuracy",
                    type=semi_minor_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
                SequenceField(
                    name="orientation",
                    type_name="SemiMajorAxisOrientation",
                    type=orientation_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
            ),
            is_extensible=False,
        ),
        spec_section="",
        description="",
    )

    registry = {
        "SemiMajorAxisAccuracy": semi_major,
        "SemiMinorAxisAccuracy": semi_minor,
        "SemiMajorAxisOrientation": orientation,
        "PositionalAccuracy": positional_accuracy,
    }

    return J2735Specification(
        version="202409",
        messages=(),
        data_frames=(),
        data_elements=(),
        type_registry=registry,
    )


def make_extensible_mock_spec() -> J2735Specification:
    """Create a mock specification with an extensible SEQUENCE type.

    Returns:
        J2735Specification with PathPrediction-like extensible SEQUENCE.

    This is useful for testing extension handling code generation.
    """
    radius_constraint = IntegerConstraint(min_value=-32767, max_value=32767)
    confidence_constraint = IntegerConstraint(min_value=0, max_value=200)

    radius_of_curvature = ASN1TypeDefinition(
        name="RadiusOfCurvature",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (-32767..32767)",
        constraint=radius_constraint,
        spec_section="",
        description="",
    )
    confidence = ASN1TypeDefinition(
        name="Confidence",
        type_class=ASN1TypeClass.INTEGER,
        raw_definition="INTEGER (0..200)",
        constraint=confidence_constraint,
        spec_section="",
        description="",
    )

    path_prediction = ASN1TypeDefinition(
        name="PathPrediction",
        type_class=ASN1TypeClass.SEQUENCE,
        raw_definition="SEQUENCE { radiusOfCurve RadiusOfCurvature, confidence Confidence, ... }",
        constraint=SequenceType(
            fields=(
                SequenceField(
                    name="radiusOfCurve",
                    type_name="RadiusOfCurvature",
                    type=radius_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
                SequenceField(
                    name="confidence",
                    type_name="Confidence",
                    type=confidence_constraint,
                    is_optional=False,
                    section_comment="",
                    inline_comment="",
                ),
            ),
            is_extensible=True,
        ),
        spec_section="",
        description="",
    )

    registry = {
        "RadiusOfCurvature": radius_of_curvature,
        "Confidence": confidence,
        "PathPrediction": path_prediction,
    }

    return J2735Specification(
        version="202409",
        messages=(),
        data_frames=(),
        data_elements=(),
        type_registry=registry,
    )


# =============================================================================
# Shared typedef Validators
# =============================================================================


def _validate_bitstring_type(typedef: ASN1TypeDefinition | None) -> None:
    """Validate that type_name exists and is a BIT STRING in the spec.

    Args:
        typedef: The ASN.1 type definition to validate.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.
    """
    if typedef is None:
        raise ValueError("Type not found in specification")
    if typedef.type_class != ASN1TypeClass.BIT_STRING:
        raise ValueError(f"Type '{typedef.name}' is not a BIT STRING")
    if not isinstance(typedef.constraint, BitStringConstraint):
        raise ValueError(f"Type '{typedef.name}' has unexpected constraint type")


def generate_bitstring_code(template_path: str, spec: J2735Specification, type_name: str) -> str:
    """Generate BIT STRING C code for a given template.

    Args:
        template_path: Path to the Jinja2 template for BIT STRING code generation.
        spec: The parsed J2735 specification.
        type_name: Name of the BIT STRING type (e.g., "VehicleEventFlags").

    Returns:
        C code from the rendered template.

    Raises:
        ValueError: If type_name is not found or not a BIT STRING.
        jinja2.TemplateNotFound: If template_path does not exist.
    """
    typedef = spec.lookup_type(type_name)
    _validate_bitstring_type(typedef)
    env = create_jinja_env()
    return get_template(env, template_path).render(typedef=typedef)


def get_sequence_typedef(type_name: str, spec: J2735Specification) -> ASN1TypeDefinition:
    """Lookup and validate a SEQUENCE type from the specification.

    Args:
        type_name: Name of the type to look up.
        spec: The parsed J2735 specification.

    Returns:
        The type definition (guaranteed to have SequenceType constraint).

    Raises:
        ValueError: If type not found, not a SEQUENCE, or missing constraint.
    """
    typedef = spec.lookup_type(type_name)
    if typedef is None:
        raise ValueError(f"Type '{type_name}' not found in specification")
    if typedef.type_class != ASN1TypeClass.SEQUENCE:
        raise ValueError(f"Type '{type_name}' is not a SEQUENCE")
    if not isinstance(typedef.constraint, SequenceType):
        raise ValueError(f"Type '{type_name}' has no SequenceType constraint")
    return typedef


# =============================================================================
# Shared Test Base Classes
# =============================================================================


class SpecLoadingTestBase(TestCase):
    """Base class that loads the J2735 specification once per test class.

    Use this for any test that needs access to the parsed J2735 spec.
    Subclasses can access the spec via `self.spec`.
    """

    spec: J2735Specification

    @classmethod
    def setUpClass(cls) -> None:
        """Load spec once for all tests."""
        cls.spec = parse_spec_file(SPEC_FILE_PATH)
