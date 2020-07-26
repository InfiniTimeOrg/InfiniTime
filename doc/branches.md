# Branches
The branching model of this project is based on the workflow named [Git flow](https://nvie.com/posts/a-successful-git-branching-model/).

It is based on 2 main branches:
 - **master** : this branch is always ready to be reployed. It means that at any time, we should be able to build the branch and release a new version of the application.
 - **develop** : this branch contains the latest development that will be integrated in the next release once it's considered as stable.

New features should be implemented in **feature branches** created from **develop**. When the feature is ready, a pull-request is created and it'll be merge into **develop** when it is succesfully reviewed and accepted.

To release a new version of the application, when develop is considered stable, a **release** branch is created from **develop**. This can be considered as a *release candidate* branch. When everything is OK, this release branch is merged into **master** and the release is generated (a tag is applied to git, the release note is finalized, binaries are built,...) from **master**.

Git flow also supports the creation of **hotfix** branches when a bug is discovered in a released version. The **hotfix** branch is created from **master** and will be used only to implement a fix to this bug. Multiple hotfix branches can be created for the same release if more than one bugs are discovered.