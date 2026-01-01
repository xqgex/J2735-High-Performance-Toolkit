---
applyTo: "**"
---

# License Header Standards

Every source file MUST have the full Apache 2.0 license header.

## Templates by File Type

### C/H Files

Use Doxygen comments.

```c
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
```

### Python / YAML / Makefile / Shell / Gitignore

Use hash comments.

```sh
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
<THE REST OF THE FILE>>
```

### Jinja2 Templates

Templates should also have the license as a comment at the top of each file wrapped with `{#-` and `-#}`.

## Exceptions

- **Third-party files**: Do NOT modify (e.g., `tests/unity*.c`, `tests/unity*.h` are MIT licensed)
- **GitHub Issue Templates**: `.github/ISSUE_TEMPLATE/*.yml` must start with `name:` per GitHub spec - no license header
- **LICENSE, NOTICE, README.md, CITATION.cff**: No license header needed
