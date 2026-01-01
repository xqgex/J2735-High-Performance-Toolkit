/**
 * Copyright 2026 Yogev Neumann
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2026 Yogev Neumann
 */
/**
 * @file
 * @author Yogev Neumann
 * @brief Sanity tests for PathPrediction.
 *
 * The data frame PathPrediction is a simple case with an extension field.
 */

#ifndef J2735_INTERNAL_DF_PATHPREDICTION_TEST_H
#define J2735_INTERNAL_DF_PATHPREDICTION_TEST_H

void test_path_prediction_no_extension(void);
void test_path_prediction_with_extension(void);
void test_path_prediction_signed_negative(void);

void run_testsuite_path_prediction(void);

#endif /* J2735_INTERNAL_DF_PATHPREDICTION_TEST_H */
