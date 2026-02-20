# Spec Driven Development

The root of this project `REPO-ROOT/.github/Agent` is not the workspace root,
you need to cd/pushd to this folder before calling yarn and nodejs.
The CopilotPortal package serves a website and a RESTful API that can be tested with playwright chromium.
All folders and files mentioned in the instructions are in the `REPO-ROOT/.github/Agent` folder.

When this file is tagged, find out new changes in the spec:
- Delete all files in `prompts/snapshot`.
- Copy all files in `prompts/spec` to `prompts/snapshot`.
- You can see what has been changed in the spec by git diff.
  - Ignore CRLF and LF differences if there is any.
- Changes includes not only spec files changing but also adding or removing.
- You are recommended to read `README.md` to understand the whole picture of the project as well as specification organizations.
- You are recommended to read all spec files to understand dependencies and relationships between each component.
- Implement all changes.

**IMPORTANT**: If you find any mistake in the spec causing you not being able to strictly follow, fix the spec.
**IMPORTANT**: Always use `api/stop` to stop `yarn portal`, trying to ENTER will hang the terminal forever and you can't proceed.
**IMPORTANT**: Avoid global variables at all cost, until you find no other way.
**IMPORTANT**: This project is full of parallel tasks, try to keep all functions re-entrable.
**IMPORTANT**: DO NOT add dynamic properties to any object. If the interface need to change, change the interface and the spec together.

## Changing the Spec

You are allowed to change the spec. And you must change the spec when you decide to:
- Add a new feature but the spec doesn't describe it.
- Change the implementation when the spec describes the old one.
- Delete a feature but the spec describes it.

When you think the spec needs to update,
You will have to change files in both `prompts/snapshot` and `prompts/spec`, keep them sync.

### Fixing Bugs

You might find some lines begins with `**BUG**:` in the spec.
It means you need to fix the bug about the surrounding context.
Delete the bug from the spec after you fix it, keep `prompts/snapshot` and `prompts/spec` sync.

### Performing Tasks

You might find some lines begins with `**TASK**:` in the spec, or `**TASK-BEGIN**` to its nearest `**TASK-END**` blocks.
It means you need to complete the task described in the surrounding context.
Delete the task from the spec after you complete it, keep `prompts/snapshot` and `prompts/spec` sync.

### Maintaining "Referenced By"

There are bullet lists under `**Referenced by**:` for most of the sessions, their format is:
```
- SpecFile.md: `# This Section`, `### That Section`...
```
Each file occupys one line.
It means the behavior of those sections depend on the current section, when the current section is changed, those sections probably need to check.
Maintain the list when you find new or outdated dependencies.

### Interface Changes

It is very often that you need to change the interface because of the spec changes, but they are not mentioned.
You need to update the spec with your interface changes too.

**IMPORTANT**:
- The list coule be incomplete, it is just a hint.
- You should maintain this list automatically.

## Testing with a Free Model

There are some models whose multiplier is 0.
Try your best to pick them, therefore it doesn't cost for performing any testing.
List all models by `api/copilot/models`.

## RESTful API Testing

When RESTful API implementation is changed,
you are recommended to start the server and test the API directly first.
Walk through all APIs.
Pay attention to the working directory argument in the API, you can use the `REPO-ROOT` of this project.
Make some random conversation without making any side effect to the project (specified by working directory argument).

## Playwright Testing

When the website is changed, you must always try all webpage features described in the spec.

## Maintaining Unit Test

It starts a copilot session so it could be hard to predict what the response will be.
But try your best to design and maintain a set of unit test files in the `test` folder of any working package.
Cover as much as you can against RESTful API and website features.

There will be **TEST-NOTE** or **TEST-NOTE-BEGIN** to **TEST-NOTE-END** blocks in the spec.
They applies to the limited context surrounding them.
They offer ideas of how to perform testing for a certain feature that is complex and not easy to do, or to skip something that is really not able to be tested.
Having a test note doesn't mean it is enough to only cover what a test note says. You must always do complete testing. When testing against a feature with a test note, follow the note to organize your test cases.

When any feature is changed, update the unit test as well.

## Post Implementation

Remember to update `README.md` to describe:
- What this project is.
- What does it use.
- How to maintain and run the project.
- Brief description of the project structure.

Always send the `api/stop` to kill the server no matter it is running or not.

### Git Push

This section only applies when you are running locally (aka not running in github.com maintaining a pull request).

When you think you have implemented all changes and all tests including playwright and unit test pass,
git commit the change with title "Updated .github/Agent" and git push.
Git push may fail when the remote branch has new commits:
- Do the merge when it could be automatically done. And then git push again.
- If there is any merge conflict, tell me and stop.
