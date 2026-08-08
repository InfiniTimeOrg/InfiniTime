<!--
SPDX-FileCopyrightText: 2025 Gusted, jeanbaptisteC, gedankenstuecke, mark9064

SPDX-License-Identifier: CC-BY-4.0
-->

# AI usage in the InfiniTime project

This document describes if and how _artificial intelligence_ can be used in the InfiniTime project by contributors, including agreements for how to use it in a transparent and responsible way. This agreement is applicable to all projects hosted under the [InfiniTime organisation](https://github.com/InfiniTimeOrg).

All contributors are expected to be aware of this agreement and to apply it in their contributions. Contributions that do not honor these agreements will not be accepted and admins will close corresponding pull requests and issues.

## Definitions & Motivations

> [!NOTE]
>This does not necessarily reflect the official or commonly used definitions.

Software and services that heavily rely on large language model technology to generate their outcomes are referred to as Artificial Intelligence (AI). Examples of products that fit this definition: GitHub Copilot, ChatGPT, Claude Sonnet, DeepSeek, Llama and Gemini.

There's a distinction between general and narrow AI, all the aforementioned examples fall under general AI as they were not trained to execute a specific well-defined task. Narrow AI is trained to be used for specific well-defined tasks where the problem space is known in advance.

Vibe coding is the practice where AI creates a code change (feature, bug fix, tests, refactor) with a human that describes what needs to be implemented.

AI agents are AIs that are configured to perform interactions or make changes with little to no human supervision.

"AI agents" and "vibe coding" typically result in the "authors" of code not having expertise in the changes they aim to contribute, thus not able to answer questions about which trade-offs were made and why. If authors were only minimally involved in how code was generated, they can't be accountable for AI's output, which is harmful to the long-term sustainability of open source projects like InfiniTime.

## Agreement

1. If content was made with the help of AI, you must convey that this is the case. This includes content that you authored but was motivated by a suggestion of AI.
2. If at any point you used AI's work in your contribution you should make an effort to verify that you can submit this under the license of the repository.
3. The accountability of using AI in a contribution lies with the person that makes that contribution.
4. All communication, that includes: **commit messages**, **pull request messages**, **documentation**, **code comments** and **issues** (and comments on issues/pull requests), that is intended to be read by people to understand your thoughts and work must not have been generated with AI. We exclude machine translation and tooling that helps with grammar and spelling check.
5. Using general AI for review is forbidden. Changes have to be approved by a human reviewer.
6. It is not allowed to use AI in an autonomous-looking way to contribute in InfiniTime (i.e. solely using 'agent mode' or 'vibe coding'). To ensure the long-term maintainability and sustainability of the project, we require authors to have read **and** understand the code they submit themselves.

## License

This agreement is a near verbatim copy of [the _AI Agreement_ that the _CoMaps_ project has passed in March 2026](https://codeberg.org/comaps/Governance/pulls/136) which is [licensed under a Creative Commons CC BY 4.0 license](https://creativecommons.org/licenses/by/4.0/legalcode).

That agreement was in turn adapted from the _Forgejo_ project. The InfiniTime project extends its thanks to many upstream contributors to this policy, and also releases this version under the same CC BY 4.0 license with the hope that any further adopters of this policy would consider doing the same.
