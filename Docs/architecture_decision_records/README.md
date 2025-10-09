# Architecture Decision Records

This directory houses the authoritative architecture decision records (ADRs) for Libft. Each ADR captures the context, decision, and consequences associated with a significant change to the library so future maintainers understand why the current design exists.

## Goals

- Provide a durable log of high-impact decisions across modules.
- Document alternatives that were considered and the rationale for the chosen approach.
- Establish a repeatable workflow for proposing and reviewing new ADRs.

## Conventions

- ADR files live in this directory and follow the pattern `NNNN-title.md`, where `NNNN` is a zero-padded sequence number.
- Every ADR begins with Status, Context, Decision, and Consequences sections. Additional sections such as Considered Alternatives or Implementation Notes are encouraged when helpful.
- ADRs remain immutable once the decision is accepted. Follow-up changes should create a superseding ADR that references the original identifier.
- Link to relevant code, documentation, and tracking issues when possible so readers can deep-dive into supporting materials.

## Workflow for new ADRs

1. Run `tools/new_adr.py` (planned) or manually copy `template.md` in this directory to seed a draft.
2. Fill out the Context section with problem framing, stakeholders, and constraints that influenced the decision.
3. Capture the Decision and cite key trade-offs in the Consequences section.
4. Submit the ADR alongside the implementation or as a dedicated documentation change for review.
5. Once accepted, update `README.md` or module documentation to reference the ADR if it alters public behaviour.

## Index

- [0001-global-error-codes.md](0001-global-error-codes.md) — Establishes the single-source-of-truth error code registry shared by every module.
- [0002-compatebility-shim-boundaries.md](0002-compatebility-shim-boundaries.md) — Defines how platform shims isolate OS-specific behaviour while preserving consistent APIs.
- [0003-module-manifest-includes.md](0003-module-manifest-includes.md) — Documents the move to manifest-driven umbrella includes to eliminate divergence in `FullLibft.hpp`.

Refer to the template below when drafting new decisions.

## Template

See [`template.md`](template.md) for the standard ADR scaffold.
