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
J2735 Byte Size Constants C Code Generator.

Generates #define J2735_SIZE_* constants for all Data Elements with fixed
UPER bit-widths. SIZE = ceil(bit_width / 8). These constants are used for
buffer allocation in zero-copy container structs.

Example usage:
    from tools.j2735_c_generator_size import generate_size_constants
    from tools.j2735_spec_parser import parse_spec_file

    spec = parse_spec_file("J2735_202409_pdf_content.txt")
    code = generate_size_constants(spec)
    print(code)
"""

from __future__ import annotations

from typing import TYPE_CHECKING

from .j2735_c_generator_jinja import create_jinja_env, get_template
from .j2735_spec_parser import ASN1TypeDefinition

if TYPE_CHECKING:
    from .j2735_spec_parser import J2735Specification

_TEMPLATE_NAME = "size_constants.j2"


def generate_size_constants(spec: J2735Specification) -> str:
    """Generate C #define constants for byte sizes of all types with fixed bit-widths.

    SIZE = ceil(bit_width / 8) = (bit_width + 7) // 8

    Args:
        spec: The parsed J2735 specification.

    Returns:
        C code with #define constants and comments.

    Examples:
        >>> from tools.tests.conftest import SPEC_FILE_PATH
        >>> from tools.j2735_spec_parser import parse_spec_file
        >>> spec = parse_spec_file(SPEC_FILE_PATH)
        >>> code = generate_size_constants(spec)
        >>> "J2735_SIZE_BSM_CORE_DATA" in code
        True
        >>> "37U" in code  # 290 bits -> 37 bytes
        True
    """
    # Collect types with fixed bit-widths (same as bitwidth generator)
    fixed_types: list[ASN1TypeDefinition] = []

    for _, typedef in sorted(spec.type_registry.items()):
        if typedef.uper_bit_width is not None:
            fixed_types.append(typedef)

    env = create_jinja_env()
    template = get_template(env, _TEMPLATE_NAME)

    return template.render(
        types=fixed_types,
        spec_version=spec.version,
    )
