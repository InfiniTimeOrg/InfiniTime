# How to contribute?

Here are things you can do to help.

- [Report bugs or issues](https://github.com/InfiniTimeOrg/InfiniTime/issues/new/choose)
- [Write and improve documentation](#documentation)
- [Fix bugs, add functionalities and improve the code](#how-to-create-a-pull-request)
- Participate in the discussions within issues and PRs. Your feedback is appreciated!
- Review pull requests. Follow the instructions [here](maintainer-guide.md#reviewing-prs)

## Documentation

Documentation might be incomplete,
or not clear enough,
and it is always possible to improve it with better wording, pictures, videos,...
As the documentation is part of the source code,
you can submit changes to the documentation by creating a [pull request](#how-to-create-a-pull-request)

## How to create a pull request?

The workflow is based on [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow).

To create a pull request,
you need a [fork](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/working-with-forks/about-forks) of the repo.
Create a new [branch](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-branches) from develop,
make changes in this branch,
[commit](https://docs.github.com/en/pull-requests/committing-changes-to-your-project/creating-and-editing-commits/about-commits) them,
and create a [pull request](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request).

### PR checklist

When making changes to the firmware:

- Consider if your feature aligns with the [InfiniTime vision](InfiniTimeVision.md)
- Test your changes on a PineTime or InfiniSim
- Make sure the code conforms to the [coding conventions](coding-convention.md)

You can also check the [maintainer's guide](maintainer-guide.md)
for what maintainers will look at in PRs.

### Commit conventions

- Every commit must contain a title and description,
  that sufficiently explains all the changes in the commit
- Commits that fix mistakes from previous commits must be squashed
- Conflicts must be resolved by rebasing,
  instead of merging
