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
"""Tests for CHOICE type code generation via generate_data_frame()."""

from tools.j2735_c_generator_data_frame import generate_data_frame
from tools.tests.conftest import SpecLoadingTestBase


class TestGenerateDataframeChoice(SpecLoadingTestBase):
    """Tests for generate_data_frame() with CHOICE types."""

    def test_generates_header_guard(self) -> None:
        """Generated code includes proper header guard."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("#ifndef J2735_INTERNAL_DF_APPROACHORLANE_H", code)
        self.assertIn("#define J2735_INTERNAL_DF_APPROACHORLANE_H", code)
        self.assertIn("#endif /* J2735_INTERNAL_DF_APPROACHORLANE_H */", code)

    def test_generates_includes(self) -> None:
        """Generated code includes required headers."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn('#include "J2735_internal_common.h"', code)
        self.assertIn('#include "J2735_internal_constants.h"', code)

    def test_generates_index_bits_constant(self) -> None:
        """Generated code includes CHOICE index bits constant."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE", code)
        self.assertIn("1U", code)  # 2 alternatives -> 1 bit

    def test_generates_alternative_constants(self) -> None:
        """Generated code includes alternative index constants."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_CHOICE_APPROACH_OR_LANE_APPROACH 0U", code)
        self.assertIn("J2735_CHOICE_APPROACH_OR_LANE_LANE 1U", code)

    def test_generates_max_wire_bits(self) -> None:
        """Generated code includes MAX_WIRE_BITS constant."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_INTERNAL_MAX_WIRE_BITS_APPROACH_OR_LANE", code)
        self.assertIn("J2735_BW_LANE_ID", code)  # Max alternative

    def test_generates_raw_read_macro(self) -> None:
        """Generated code includes RAW_READ macro."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_APPROACH_OR_LANE_RAW_READ(buf)", code)
        self.assertIn("J2735_READ_BITS", code)

    def test_generates_which_macro(self) -> None:
        """Generated code includes WHICH macro."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_APPROACH_OR_LANE_WHICH(raw9)", code)

    def test_generates_get_approach_macro(self) -> None:
        """Generated code includes GET_APPROACH macro."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_APPROACH_OR_LANE_GET_APPROACH(raw9)", code)
        self.assertIn("J2735_BW_APPROACH_ID", code)

    def test_generates_get_lane_macro(self) -> None:
        """Generated code includes GET_LANE macro."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_APPROACH_OR_LANE_GET_LANE(raw9)", code)
        self.assertIn("J2735_BW_LANE_ID", code)

    def test_generates_size_macro(self) -> None:
        """Generated code includes SIZE macro."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("J2735_APPROACH_OR_LANE_SIZE(raw9)", code)

    def test_generates_license_header(self) -> None:
        """Generated code includes Apache 2.0 license header."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("Copyright 2026 Yogev Neumann", code)
        self.assertIn("Apache License, Version 2.0", code)
        self.assertIn("SPDX-License-Identifier: Apache-2.0", code)

    def test_generates_wire_format_docs(self) -> None:
        """Generated code includes wire format documentation."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("Wire Format", code)
        self.assertIn("approach selected", code)
        self.assertIn("lane selected", code)

    def test_generates_usage_example(self) -> None:
        """Generated code includes usage example in documentation."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn("Usage Pattern", code)
        self.assertIn("switch", code)

    def test_misra_compliant_no_shift_by_zero(self) -> None:
        """GET_LANE macro uses mask-only pattern (no shift by 0)."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        # Lane is the max alternative, so shift = 9 - 1 - 8 = 0
        # Should NOT have a shift in the GET_LANE macro
        get_lane_line = [
            line
            for line in code.split("\n")
            if "J2735_APPROACH_OR_LANE_GET_LANE" in line and "#define" in line
        ]
        self.assertTrue(len(get_lane_line) > 0)
        # The mask-only pattern should have a comment indicating no shift needed
        self.assertIn("no shift needed", code.lower())

    def test_generates_static_assert_choice_index_bits(self) -> None:
        """Generated code validates CHOICE index bits with _Static_assert."""
        code = generate_data_frame("ApproachOrLane", self.spec)
        self.assertIn(
            "_Static_assert(J2735_INTERNAL_CHOICE_INDEX_BITS_APPROACH_OR_LANE"
            " == J2735_BW_APPROACH_OR_LANE",
            code,
        )

    def test_unknown_type_raises(self) -> None:
        """generate_data_frame raises ValueError for unknown type."""
        with self.assertRaises(ValueError) as cm:
            generate_data_frame("NonExistentType", self.spec)
        self.assertIn("not found", str(cm.exception))

    def test_non_choice_or_sequence_type_raises(self) -> None:
        """generate_data_frame raises ValueError for non-composite type."""
        # ApproachID is an INTEGER, not a SEQUENCE or CHOICE
        with self.assertRaises(ValueError) as cm:
            generate_data_frame("ApproachID", self.spec)
        self.assertIn("not a supported Data Frame type", str(cm.exception))
