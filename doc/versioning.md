# Versioning
The versioning of this project is based on [Semantic versionning](https://semver.org/) :

 - The **patch** is incremented when we fix a bug on a **released** version (most of the time using a **hotfix** branch).
 - The **minor** is incremented when we release a new version with new features. It corresponds to a merge of **develop** into **master**.
 - The **major** should be incremented when a breaking change is made to the application. We still have to define what is a breaking change in the context of this project. For now, I suggest that it stays **0** until we have a fully functionning firmware suited for the final user.