# Security Policy

## Reporting a Vulnerability

The J2735 High-Performance Toolkit is designed for safety-critical V2X and
embedded systems. We take security vulnerabilities seriously.

Please **DO NOT** report security vulnerabilities via public GitHub issues.

If you believe you have found a security vulnerability in the J2735 Toolkit (e.g., buffer overflow, integer overflow, memory corruption), please report it privately.

### How to Report

Please report security issues by emailing:

Email: **yogev.j2735@gmail.com**

Subject: "J2735 Security Vulnerability"

Include the following information:
- Description of the vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if any)

### What to Expect

- **Acknowledgment**: Within 48 hours
- **Initial Assessment**: Within 7 days
- **Resolution Timeline**: Depends on severity
  - Critical: 7-14 days
  - High: 30 days
  - Medium/Low: Next release cycle

### Disclosure Policy

- We follow coordinated disclosure
- Credit will be given to reporters (unless anonymity is requested)
- We will notify you when the fix is released

## Security Considerations for Users

This library performs zero-copy parsing of untrusted network data. Users should:

1. **Validate buffer lengths** before calling parsing macros
2. **Never trust input sizes** from network sources
3. **Use the provided bit-width constants** to validate field ranges
4. **Run with sanitizers** during development (`make sanitize`)

## Scope

The following are in scope for security reports:
- Buffer overflows / out-of-bounds reads
- Integer overflows in bit arithmetic
- Undefined behavior that could be exploited
- Memory safety issues

The following are out of scope:
- Issues in third-party code (Unity test framework)
