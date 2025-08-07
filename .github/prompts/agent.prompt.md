# for Copilot with Agent mode in Visual Studio

- You are on Windows running in Visual Studio
- Before saying anything, say "Yes, vczh!". I use it to make sure instruction files are taking effect.
- Before generating any code, if the file is changed, read it. Not all changes come from you, I will edit the file too. Do not generate code based on out-dated version in your memory.
- I would like to add custom commands. For example, when I type `:prepare`, follow instructions in the `## :prepare` section. Same for others.
  - If you can't find the section, warn me and stop immediately.

## :prepare

- Find the full path for the opened solution, and the full path for the folder containing the opened solution.
- Find the solution folder `@Copilot` in the solution explorer. If it does not exist, create it. This is not an actual folder in the file system.
- Prepare the following files:
  - `Copilot_Planning.md`
  - `Copilot_Execution.md`
  - `Copilot_Task.md`
  - `.gitignore`

## :plan

- Firstly, there are `Copilot_Planning.md` and `Copilot_Execution.md` in the solution. If it exists, delete all their content. If any file doesn't exist, create an empty text file on the file path.
- Secondly, there is a `Copilot_Task.md` in the solution. It has the details of the task to execute. Print the content.
- If you don't find any mentioned `*.md` files in the solution, report and stop immediately.
- From now on, DO NOT change any code, ONLY change `Copilot_Planning.md` and `Copilot_Execution.md`.
  - When you add new content to `Copilot_Planning.md` or `Copilot_Execution.md` during the process, everything has to be appended to the file.
  - To generate a correct markdown format, when you wrap code snappet in "```", the must take whole lines.
- Carefully find all necessary files you may need to read.
  - If any file is mentioned in the task, you must read it.
  - If any type name or function name is mentioned in the task, you must first try to find the full name of the name as well as the file where it is defined. Read the file.
    - Write down the full name and the file name in `Copilot_Planning.md`.
- Carefully think about the task, make a overall design.
  - Write down the design in `Copilot_Planning.md`.
- Carefully think about how to implement the design.
  - Do not edit the code directly.
  - Explain what you gonna do, and why you decide to do it in this way.
  - It must be detailed enough, so that the plan can be handed over to another copilot to implement, who does not have access to our conversation.
  - write it down in `Copilot_Planning.md`.
- Carefully find out what could be affected by your change.
  - Do not edit the code directly.
  - Explain what you gonna do, and why you decide to do it in this way.
  - It must be detailed enough, so that the plan can be handed over to another copilot to implement, who does not have access to our conversation.
  - write it down in `Copilot_Planning.md`.
- Add a section `# !!!FINISHED!!!` in `Copilot_Planning.md`
  - From now on you don't need to update `Copilot_Planning.md` anymore.
- Add a section `# !!!EXECUTION-PLAN!!!` in `Copilot_Execution.md`.
  - Carefully review what has been written in `Copilot_Planning.md`.
  - Copy them to `Copilot_Execution.md` only about how to modify the code.
  - It must be detailed enough, so that the plan can be handed over to another copilot to implement, who does not have access to our conversation.

## :execute

- There is a `Copilot_Execution.md` in the solution.
  - Carefully review the file
  - Execute the plan precisely in `Copilot_Execution.md`. 
  - When everything is finished, add a new section `# !!!FINISHED!!!`.

## :task

- Do both `:plan` and `:execute` in order.

## :continue

- It means you accidentally stopped in the middle of `:task`.
- Find out where you stopped, and continue from there.

## :continue-plan

- It means you accidentally stopped in the middle of `:plan`.
- Find out where you stopped, and continue from there.

## :continue-execute

- It means you accidentally stopped in the middle of `:execute`.
- Find out where you stopped, and continue from there.
