# Contributing to Libft

Thank you for your interest in improving Libft! This guide captures the expectations for proposing changes, writing code, and keeping the project healthy. Please read it fully before opening an issue or pull request.

## Getting Started

1. **Discuss major ideas first.** Open an issue that outlines the problem you want to solve, the modules it touches, and how you plan to measure success.
2. **Create focused branches.** Keep each branch scoped to a single feature or bug fix so reviews stay quick and the history remains easy to understand.
3. **Follow the existing directory layout.** Place headers, sources, and tests alongside peers in the relevant module folders.

## Code Style and Design

Libft follows a strict style guide so the codebase stays consistent. The highlights below summarize the expectations enforced throughout the repository:

- Use four spaces for indentation and adopt Allman-style braces with the opening brace on its own line.
- Name functions and variables with `snake_case`, preferring descriptive identifiers over single letters.
- Declare classes with private members first, prefix member names with an underscore, and separate access specifiers with a blank line.
- Split class declarations (`.hpp`) from definitions (`.cpp`) unless you are working on templates that require header-only implementations.
- Return from void functions with `return ;` and from non-void functions with `return (value);`, including the required spacing.
- Avoid `for` loops, ternary operators, and `switch` statements—they are intentionally excluded to keep the code uniform.
- Use module prefixes for `.cpp` filenames, and reserve prefixed headers for internal-facing APIs.
- Surface errors through `_error_code` members, implementing `set_error`, `get_error`, and `get_error_str` helpers for each class.

Run `make format` before sending changes so the automated formatting checks pass locally.

## Testing Expectations

- Add or update unit tests whenever you change behavior or introduce new functionality. Favor deterministic tests that do not depend on external state.
- Execute the relevant module builds (for example, `make -C Libft`) and the top-level regression suites in `Test/` when applicable.
- When touching low-level code that manages memory or system resources, run the sanitizer builds via `make asan-tests` and `make ubsan-tests` to surface hidden defects early.
- If a test cannot be added due to missing scaffolding, document the gap in your pull request and add a TODO entry that describes the required follow-up work.

## Commit and Review Process

- Write clear commit messages using the imperative mood (e.g., "Add bounded string helpers").
- Keep diffs small and focused. Large refactors should be split into preparatory commits.
- Ensure `TODO.md` reflects the work completed—check items off when you finish them and add new tasks that emerge during development.
- Include a summary of manual or automated tests in your pull request description so reviewers can verify your steps.

## Pull Request Checklist

Before requesting a review, confirm the following:

- [ ] All new and modified files conform to the style rules described above.
- [ ] Tests build and pass locally, or failures are explained and tracked in `TODO.md`.
- [ ] Documentation is updated for user-visible changes or new contributor workflows.
- [ ] The branch is rebased onto the latest `main` (no merge commits).

We appreciate your contributions and the time you invest in improving Libft. If you have questions about these guidelines, please open an issue so we can clarify expectations for everyone.
