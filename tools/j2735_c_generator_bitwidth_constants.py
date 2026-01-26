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
J2735 Bit-Width Constants C Code Generator.

Generates #define J2735_BW_* constants for all Data Elements with fixed
UPER bit-widths. These constants enable O(1) bit-stream navigation.

Example usage:
    from tools.j2735_c_generator_bitwidth import generate_bitwidth_constants
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")
    code = generate_bitwidth_constants(spec)
    print(code)
"""

from .j2735_c_generator_jinja import create_jinja_env, get_template
from .j2735_spec_parser import ASN1TypeDefinition, J2735Specification

_BITWIDTH_TEMPLATE_NAME = "bitwidth_constants.j2"


def generate_bitwidth_constants(spec: J2735Specification) -> str:
    """Generate C #define constants for all types with fixed bit-widths.

    This generator collects all Data Elements from the specification that
    have determinable UPER bit-widths and generates corresponding
    J2735_BW_* constants.

    Args:
        spec: The parsed J2735 specification.

    Returns:
        C code with #define constants and comments.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_bitwidth_constants(spec)
        >>> "J2735_BW_MSG_COUNT" in code
        True
        >>> "7U" in code
        True
    """
    # Collect types with fixed bit-widths
    fixed_types: list[ASN1TypeDefinition] = []
    variable_count = 0

    for _, typedef in sorted(spec.type_registry.items()):
        if typedef.uper_bit_width is not None:
            fixed_types.append(typedef)
        else:
            variable_count += 1

    env = create_jinja_env()
    template = get_template(env, _BITWIDTH_TEMPLATE_NAME)

    return template.render(
        types=fixed_types,
        spec_version=spec.version,
        variable_count=variable_count,
    )
