# Branches

The project has 1 main branch, aptly called **main**.
This branch contains the latest development that will be tagged for the next release once it's considered stable.

New features should be implemented in **feature branches** created from **main**.
When the feature is ready, a pull request is created and it'll be merged into **main** when it is successfully reviewed and accepted.

To release a new version of the application, when main is considered stable, a tag is created on the version bump commit in **main** and the release is generated (a tag is applied to git, the release note is finalized, binaries are built,...).

We also supports the creation of **hotfix** branches when a bug is discovered in a released version. The **hotfix** branch is created from the latest tag and will be used only to implement a fix to this bug.
Multiple hotfix branches can be created for the same release if multiple bugs are discovered.
