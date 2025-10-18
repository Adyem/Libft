# Developer Ergonomics

This guide documents the tooling added to streamline day-to-day development tasks.

## Pre-commit Hooks

The repository ships with a [pre-commit](https://pre-commit.com/) configuration that
runs basic hygiene checks and executes `make format` before each commit.

1. Install the tool once:
   ```sh
   pip install pre-commit
   ```
2. Install the git hooks by running the helper script:
   ```sh
   ./tools/install_pre_commit_hooks.sh
   ```
3. Commit as usual. The hooks will run automatically and block the commit if they
   detect formatting issues or merge conflicts.

You can trigger the hooks manually across every file with:
```sh
pre-commit run --all-files
```

## Git Blame Ignore List

Large formatting rewrites and dependency vendoring commits can make `git blame`
less useful. The `.git-blame-ignore-revs` file now tracks these commits so they can be
ignored when running:
```sh
git blame --ignore-revs-file=.git-blame-ignore-revs <path>
```

Add any new mechanical commits to the file after pushing the change so future
history inspection remains readable.

## Code generation scaffolds

`tools/generate_scaffold.py` creates module and test skeletons with the required
error-handling helpers and Allman-style formatting. See
`Docs/code_generation_scaffolds.md` for usage examples and command options.
