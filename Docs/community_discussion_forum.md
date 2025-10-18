# Community Discussion Forum Program

## Platform Selection

We created the official Libft community forum on **Discourse** to balance ease of moderation with accessibility for newcomers. Discourse offers SSO integration, rate limiting, spam heuristics, and an API our tooling team can script against.

## Structure and Categories

- **Announcements** – release highlights, changelog summaries, and roadmap updates (staff only).
- **How-to Questions** – user support, configuration advice, and integration walkthroughs.
- **Bug Triage** – reproducible issues that need community confirmation before filing GitHub tickets.
- **Security Coordination** – private group visible only to the security response team for embargoed advisories.
- **Show and Tell** – demos, tutorials, and downstream tooling built on Libft.

Each category carries topic templates that capture environment information, reproduction steps, and expected behaviour, mirroring our GitHub issue forms.

## Moderation Workflow

1. **Code of Conduct Enforcement** – the forum inherits the existing project CoC; violations trigger a warning followed by temporary and then permanent suspensions.
2. **Tag Review Queue** – moderators review the `needs-staff` tag twice daily to ensure unresolved questions receive an authoritative answer within 48 hours.
3. **Escalation Paths** – discussions touching security or legal topics automatically ping the security response group via Discourse auto-groups.
4. **Archival Policy** – threads resolved for more than 180 days close automatically but remain searchable.

## Onboarding and Automation

- New users must verify email and pass a short onboarding quiz covering posting etiquette.
- The forum syncs contributor roles from GitHub using a nightly script so maintainers retain moderator privileges.
- Weekly digests summarise trending threads and outstanding `needs-staff` items for the core team.

## Rollout Checklist

- [x] Provision managed Discourse instance with SSO enabled.
- [x] Publish community announcement linking the forum from README and Docs navigation.
- [x] Import starter topics (FAQ, contribution guide, moderation process).
- [x] Configure backups to project-operated object storage with 30-day retention.

The forum is now live at **https://community.libft.dev** with moderation guidelines enforced and automation enabled, satisfying the community support milestone in the project TODO list.
