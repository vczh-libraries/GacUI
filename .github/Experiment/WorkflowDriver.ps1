<#
Read $PSScriptRoot\Workflow.md and parse it

The file follows the format:
```
# [State] STATE-NAME
[KEYWORD][TARGET-STATE]condition

Prompt of the current state

# General Instructions

Prompt of general instructions
```

There will be multiple states in the file.
In each state there will be multiple keyword descriptions.
The entry state is named `Entry`.
After reaching `# General Instructions`, there will be no more states. The rest of the content are general instructions.

I would use the following TypeScript types to represent the workflow
```
interface Workflow {
    states: State[];
    generalInstructions: string;
}

interface State {
    name: string;
    keywords: Keyword[];
    prompt: string;
}

interface Keyword {
    keyword: string;
    targetState: string;
    condition: string;
}
```

Before examining arguments, you need to parse the file and organized the content into the above structure.
You can assume the file is well-formed.
You can make your own decision about how to represent the structure in powershell script for your own convenience.

The script can be executed using following syntax:

`WorkflowDriver.ps1`
The current state will be "Entry"
Display the current state
Write the current state name to $PSScriptRoot\WorkflowState.log

`WorkflowDriver.ps1 -Keyword KEYWORD
Read the state name from $PSScriptRoot\WorkflowState.log
Follow the keyword to transit to the next state, which becomes the current state
Display the current state
If the current state is "Finished":
  Delete $PSScriptRoot\WorkflowState.log
Otherwise:
  Write the current state name to $PSScriptRoot\WorkflowState.log

Follow the format to display the current state:
```
Prompt of the state

Call <absolute-path-to-WorkflowDriver.ps1> -Keyword KEYWORD
Use keyword "KEYWORD" if <condition-of-the-keyword>
...
```

The last part can be omitted if there is no keyword in the current state.
#>