---
applyTo: "**/*.md"
---

# Diátaxis Documentation Rules

## Know What You're Writing

| Type | Purpose | Rule |
|------|---------|------|
| **README** | Quick start | Commands to run, single entry point, link to LICENSE |
| **Docstring** | API reference | Facts only, mirrors code structure |
| **Tutorial** | Learning | Hand-holding, user does things |
| **Explanation** | Understanding | Why, context, opinions allowed |

Don't mix types. READMEs aren't tutorials. Docstrings aren't explanations.

## Don't Duplicate Code

**Litmus test**: If the code changes, must this doc change too?
- Yes → Don't write it (it will rot)
- No → Write it (stable truth)

**Never document**: Line counts, internal module lists, architecture diagrams, field names, byte sizes.

**Do document**: How to run, where to start, links.
