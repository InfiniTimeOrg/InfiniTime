# Reviewing PRs

Approving a PR means that the reviewer has deemed the PR ready to be merged.

There are two steps to reviewing PRs.

- Review the feature:

  - Consider if the feature aligns with the [InfiniTime vision](InfiniTimeVision.md)
  - Discuss alternative ideas or implementations

- Review the code:

  - Check the quality of the code and make sure it conforms to the [coding conventions](coding-convention.md)
  - Consider the maintainability of the code
  - Test the code with at least InfiniSim or a PineTime

# Merging PRs

Two approvals from core developers is usually required to merge a PR.
Exceptions include urgent fixes
and small maintenance PRs by core developers,
that don't affect the apparent behaviour of the watch in any way.

All but the simulator check must be passed before merging a PR.

PRs are either rebase or squash merged,
depending on whether the commits satisfy the following requirements:

### Commits

- Commits that fix mistakes from previous commits must be squashed before merging a PR.
- The title and description of the commit must sufficiently explain the changes in the commit.

If these requirements are not met,
the PR must be squash merged,
and the merger must write a satisfactory description.

# Stale PRs

Work-in-Progress PRs shall be marked as draft.

Draft PRs with no activity by the author for 3 months may be closed as stale.

PRs with changes requested, but no activity by the author for 3 months may be closed as stale.
