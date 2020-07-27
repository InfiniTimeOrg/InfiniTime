# How to contribute?
## Report bugs
You use your Pinetime and find a bug in the firmware? [Create an issue on Github](https://github.com/JF002/Pinetime/issues) explaining the bug, how to reproduce it, the version of the firmware you use...
## Write and improve documentation
Documentation might be incomplete, or not clear enough, and it is always possible to improve it with better wording, pictures, photo, video,... 

As the documentation is part of the source code, you can submit your improvements to the documentation by submitting a pull request (see below).
## Fix bugs, add functionalities and improve the code
You want to fix a bug, add a cool new functionality or improve the code? See *How to submit a pull request below*.
## Spread the word
Pinetime is a cool open source project that deserves to be know. Talk about it around you, on social networks, on your blog,... and let people know that we are working on an open-source firmware for a smartwatch!

# How to submit a pull request ?
Your contribution is more than welcome! 

If you want to fix a bug, add a functionality or improve the code, you'll first need to create a branch from the **develop** branch (see [this page about the branching model](./branches.md)). This branch is called a feature branch, and you should choose a name that explains what you are working on (ex: "add-doc-about-contributions"). In this branch, try to focus on only one topic, bug or feature. For example, if you created this branch to work on the UI of a specific application, do not commit modifications about the SPI driver. If you want to work on multiple topics, create one branch per topic.

When your feature branch is ready, make sure it actually works and do not forget to write documentation about it if necessary.

Then, you can submit a pull-request for review. Try to describe your pull request as much as possible: what did you do in this branch, how does it work, how is it designed, are there any limitations,... This will help the contributors to understand and review your code easily.

Other contributors can post comments about the pull request, maybe ask for more info or adjustements in the code.

Once the pull request is reviewed an accepted, it'll be merge in **develop** and will be released in the next release version of the firmware.