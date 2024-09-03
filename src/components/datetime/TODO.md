# Refactoring needed

## Context

The [PR #2041 - Continuous time updates](https://github.com/InfiniTimeOrg/InfiniTime/pull/2041) highlighted some
limitations in the design of DateTimeController: the granularity of the time returned by `DateTime::CurrentDateTime()`
is limited by the frequency at which SystemTask calls `DateTime::UpdateTime()`, which is currently set to 100ms.

@mark9064 provided more details
in [this comment](https://github.com/InfiniTimeOrg/InfiniTime/pull/2041#issuecomment-2048528967).

The [PR #2041 - Continuous time updates](https://github.com/InfiniTimeOrg/InfiniTime/pull/2041) provided some changes
to `DateTime` controller that improves the granularity of the time returned by `DateTime::CurrentDateTime()`.

However, the review showed that `DateTime` cannot be `const` anymore, even when it's only used to get the current time,
since `DateTime::CurrentDateTime()` changes the internal state of the instance.

We tried to identify alternative implementation that would have maintained the "const correctness" but we eventually
figured that this would lead to a re-design of `DateTime` which was out of scope of the initial PR (Continuous time
updates and always on display).

So we decided to merge this PR #2041 and agree to fix/improve `DateTime` later on.

## What needs to be done?

Improve/redesign `DateTime` so that it

* provides a very granular (ideally down to the millisecond) date and time via `CurrentDateTime()`.
* can be declared/passed as `const` when it's only used to **get** the time.
* limits the use of mutex as much as possible (an ideal implementation would not use any mutex, but this might not be
  possible).
* improves the design of `DateTime::Seconds()`, `DateTime::Minutes()`, `DateTime::Hours()`, etc as
  explained [in this comment](https://github.com/InfiniTimeOrg/InfiniTime/pull/2054#pullrequestreview-2037033105).

Once this redesign is implemented, all instances/references to `DateTime` should be reviewed and updated to use `const`
where appropriate.

Please check the following PR to get more context about this redesign:

* [#2041 - Continuous time updates by @mark9064](https://github.com/InfiniTimeOrg/InfiniTime/pull/2041)
* [#2054 - Continuous time update - Alternative implementation to #2041 by @JF002](https://github.com/InfiniTimeOrg/InfiniTime/pull/2054)