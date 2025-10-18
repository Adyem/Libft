# Changelog Program

This directory hosts the recurring changelog updates requested in the project TODO list. Each entry
captures the highlights for a release window and links to upstream discussions or follow-up issues.

## Publication cadence
* **Monthly snapshots.** Short notes that summarize fixes and minor features. These live in
  `Docs/changelog/YYYY-MM.md` and are published on the first Monday of the following month.
* **Quarterly highlights.** Longer form posts that collect architectural changes, roadmap updates,
  and callouts for contributor help. These live in `Docs/changelog/YYYY-QN.md`.

## Distribution workflow
1. Draft the entry in this directory and submit it with the corresponding feature work.
2. Announce the update in the community channels (forum, chat) and pin it for at least two weeks.
3. Mirror the markdown to the project website using the static site generator in `tools/site/`.
4. Archive past entries by linking them from `Docs/changelog/index.md` so new adopters can discover
   the project's history quickly.

## Template
```
# <Month/Quarter> <Year> Update

## Highlights
- Feature bullet 1 with module references.
- Feature bullet 2 that links to follow-up tasks.

## Quality and operations
- Summaries of noteworthy test runs, CI migrations, or tooling upgrades.

## Community
- Upcoming office hours, new maintainers, or requests for feedback.
```

Every entry should note the git tag (when applicable) and cross-link to relevant documentation.
