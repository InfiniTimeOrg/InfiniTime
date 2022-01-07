# How to contribute?

## Report bugs

Have you found a bug in the firmware? [Create an issue on Github](https://github.com/InfiniTimeOrg/InfiniTime/issues) explaining the bug, how to reproduce it, the version of the firmware you use...

## Write and improve documentation

Documentation might be incomplete, or not clear enough, and it is always possible to improve it with better wording, pictures, photo, video,...

As the documentation is part of the source code, you can submit your improvements to the documentation by submitting a pull request (see below).

## Fix bugs, add functionalities and improve the code

You want to fix a bug, add a cool new functionality or improve the code? See *How to submit a pull request below*.

# How to submit a pull request?

## TL;DR

 - Create a branch from develop
 - Work on a single subject in this branch. Create multiple branches/pulls-requests if you want to work on multiple subjects (bugs, features,...)
 - Test your modifications on the actual hardware
 - Check your code against the [coding conventions](/doc/coding-convention.md) and [clang-format](../.clang-format) and [clang-tidy](../.clang-tidy)
 - Clean your code and remove files that are not needed
 - Write documentation related to your new feature if applicable
 - Create a pull request and write a great description about it: what does your PR do, why, how,... Add pictures and video if possible
 - Wait for someone to review your PR and take part in the review process
 - Your PR will eventually be merged :)

Your contributions are more than welcome!

If you want to fix a bug, add functionality or improve the code, you'll first need to create a branch from the **develop** branch (see [this page about the branching model](./branches.md)). This branch is called a feature branch, and you should choose a name that explains what you are working on (ex: "add-doc-about-contributions"). In this branch, **focus on only one topic, bug or feature**. For example, if you created this branch to work on the UI of a specific application, do not commit modifications about the SPI driver. If you want to work on multiple topics, create one branch for each topic.

When your feature branch is ready, **make sure it actually works** and **do not forget to write documentation** about it if it's relevant.

**Creating a pull request containing modifications that haven't been tested is strongly discouraged.** If for any reason you cannot test your modifications, but want to publish them anyway, **please mention it in the description**. This way, other contributors might be willing to test it and provide feedback about your code.

Before submitting a PR, check your code against the [coding conventions](/doc/coding-convention.md). This project also provides [clang-format](../.clang-format) and [clang-tidy](../.clang-tidy) configuration files. You should use them to ensure correct formatting of your code.

Don't forget to check the files you are going to commit and remove those which aren't necessary (config files from your IDE, for example). Remove old comments, commented code,...

Then, you can submit a pull request for review. Try to **describe your pull request as much as possible**: what did you do in this branch, how does it work, how it is designed, are there any limitations,... This will help the contributors to understand and review your code easily. You can add pictures and video to the description so that contributors will have a quick overview of your work.

Other contributors can post comments about the pull request, maybe ask for more info or adjustments in the code.

Once the pull request is reviewed and accepted, it'll be merged into **develop** and will be released in the next version of the firmware.

## Why all these rules?

Reviewing pull requests is a **very time consuming task**. Everything you do to make reviewing easier will **get your PR merged faster**.

Reviewers will first look at the **description**. If it's easy to understand what the PR does, why the modification is needed or interesting and how it's done, a good part of the work is already done : we understand the PR and its context.

Reviewing **a few files that were modified for a single purpose** is a lot easier than reviewing 30 files modified for many reasons (bug fix, UI improvements, typos in doc,...), even if all the changes make sense. Also, it's possible that we agree on some modification but not on another, so we won't be able to merge the PR because of the changes that are not accepted.

The code base should be kept as consistent as possible. If the formatting of your code is not consistent with the rest of the code base, we'll ask you to review it.

Lastly the changes are tested. If it doesn't work out of the box, we'll ask you to review your code and to ensure that it works as expected.

It's totally normal for a PR to need some more work even after it was created, that's why we review them. But every round trip takes time, so it's good practice to try to reduce them as much as possible by following those simple rules.
