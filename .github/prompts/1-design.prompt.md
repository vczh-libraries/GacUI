# Design

- The problem I would like to solve is in the chat messages sending with this request, under the `#Problem` section. If there are other sections, they are extra instructions for you in higher priority.
- Before solving my problem, read `Copilot_Task.md` and find if there is any `# !!!FINISHED!!!` mark.
  - If there is only a title, you are on a fresh start.
  - If the mark exists, it means the content of the file is out-dated. You must execute `.\copilotPrepare.ps1` to clean up everything from the last run.
    - You must execute `.\copilotPrepare.ps1`, must not be `copilotPrepare.ps1`, as PowerShell refuses to run a script file if there is only a simple file name.
    - Make sure the current directory is set to the folder containing the solution file, which I believe is the default location.
  - If the mark does not exist, it means you are accidentally stopped. Please continue the work, the problem description should be in `Copilot_Task.md`.
- Your goal is to write a design document to `Copilot_Task.md`. DO NOT update any other file including source code.
- Repeat my problem description in `Copilot_Task.md` precisely under a `#PROBLEM DESCRIPTION` section. In case when you accidentally stop later, you will know what you are told to do.
- As an experienced C++ developer for large scale systems, you need to:
  - Analyse the source code and provide a high-level design document.
  - The design document must present your idea, about how to solve the problem in architecture-wide level.
  - The design document must describe the what to change, keep the description in high-level without digging into details about how to update the source code.
  - The design document must explain the reason behind the proposed changes.
  - It is completely OK and even encouraged to have multiple solutions. You must explain each solution in a way mentioned above, and provide a comparison of their pros and cons.
- When mentioned any C++ type name, you must use its full name followed by the file which has its definition.
- Append everything to `Copilot_Task.md`. Make sure only wrap code in code block, do not wrap markdown content in code block.
- Append `# !!!FINISHED!!!` to `Copilot_Task.md` to indicate the work has been finished.
