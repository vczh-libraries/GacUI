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

Run this command after finishing the instruction:
& <absolute-path-to-WorkflowDriver.ps1> -Keyword KEYWORDS
  - Use keyword "KEYWORD" if <condition-of-the-keyword>
...
```

The last part can be omitted if there is no keyword in the current state.
#>

param(
    [string]$Keyword = ""
)

# Parse the workflow file
function Parse-Workflow {
    $workflowFile = Join-Path $PSScriptRoot "Workflow.md"
    $content = Get-Content $workflowFile -Raw
    $lines = $content -split "`n"
    
    $workflow = @{
        States = @{}
        GeneralInstructions = ""
    }
    
    $currentState = $null
    $inGeneralInstructions = $false
    $promptLines = @()
    
    foreach ($line in $lines) {
        $line = $line.TrimEnd("`r")
        
        # Check for General Instructions
        if ($line -match '^# General Instructions\s*$') {
            # Save current state if any
            if ($currentState) {
                $currentState.Prompt = ($promptLines -join "`n").Trim()
                $workflow.States[$currentState.Name] = $currentState
            }
            $inGeneralInstructions = $true
            $promptLines = @()
            continue
        }
        
        if ($inGeneralInstructions) {
            $promptLines += $line
        }
        elseif ($line -match '^# \[State\] (.+)$') {
            # Save previous state if any
            if ($currentState) {
                $currentState.Prompt = ($promptLines -join "`n").Trim()
                $workflow.States[$currentState.Name] = $currentState
            }
            
            # Start new state
            $stateName = $matches[1].Trim()
            $currentState = @{
                Name = $stateName
                Keywords = @()
                Prompt = ""
            }
            $promptLines = @()
        }
        elseif ($currentState -and $line -match '^\[([^\]]+)\]\[([^\]]+)\](.*)$') {
            # Keyword line
            $keyword = @{
                Keyword = $matches[1].Trim()
                TargetState = $matches[2].Trim()
                Condition = $matches[3].Trim()
            }
            $currentState.Keywords += $keyword
        }
        else {
            # Prompt line
            $promptLines += $line
        }
    }
    
    # Save last state or general instructions
    if ($inGeneralInstructions) {
        $workflow.GeneralInstructions = ($promptLines -join "`n").Trim()
    }
    elseif ($currentState) {
        $currentState.Prompt = ($promptLines -join "`n").Trim()
        $workflow.States[$currentState.Name] = $currentState
    }
    
    return $workflow
}

# Display the current state
function Display-State {
    param(
        [hashtable]$Workflow,
        [string]$StateName
    )
    
    $state = $Workflow.States[$StateName]
    if (-not $state) {
        Write-Host "Error: State '$StateName' not found"
        return
    }
    
    # Always print "# Instructions" first
    Write-Host "# Instructions"
    Write-Host ""
    
    # Display the prompt
    Write-Host $state.Prompt
    Write-Host ""
    
    # Display general instructions unless in Finished state
    if ($StateName -ne "Finished") {
        Write-Host "# General Instructions"
        Write-Host ""
        Write-Host $Workflow.GeneralInstructions
        Write-Host ""
    }
    
    # Display keywords if any
    if ($state.Keywords.Count -gt 0) {
        $scriptPath = $PSCommandPath
        
        # Collect all keywords
        $allKeywords = $state.Keywords | ForEach-Object { $_.Keyword }
        $keywordString = $allKeywords -join '|'
        
        Write-Host "Run this command after finishing the instruction:"
        Write-Host "& $scriptPath -Keyword $keywordString"
        foreach ($kw in $state.Keywords) {
            Write-Host "  - Use keyword `"$($kw.Keyword)`" if $($kw.Condition)"
        }
        Write-Host ""
    }
}

# Main execution
$workflow = Parse-Workflow
$stateFile = Join-Path $PSScriptRoot "WorkflowState.log"

if ($Keyword -eq "") {
    # No keyword provided, start from Entry
    $currentState = "Entry"
    Display-State -Workflow $workflow -StateName $currentState
    Set-Content -Path $stateFile -Value $currentState
}
else {
    # Keyword provided, read current state and transition
    if (-not (Test-Path $stateFile)) {
        Write-Host "Error: Workflow state file not found. Please start the workflow first."
        exit 1
    }
    
    $currentState = Get-Content $stateFile -Raw
    $currentState = $currentState.Trim()
    
    $state = $workflow.States[$currentState]
    if (-not $state) {
        Write-Host "Error: Current state '$currentState' not found"
        exit 1
    }
    
    # Find the keyword
    $targetState = $null
    foreach ($kw in $state.Keywords) {
        if ($kw.Keyword -eq $Keyword) {
            $targetState = $kw.TargetState
            break
        }
    }
    
    if (-not $targetState) {
        Write-Host "Error: Keyword '$Keyword' not found in state '$currentState'"
        exit 1
    }
    
    # Transition to target state
    $currentState = $targetState
    Display-State -Workflow $workflow -StateName $currentState
    
    # Handle Finished state
    if ($currentState -eq "Finished") {
        Remove-Item $stateFile -ErrorAction SilentlyContinue
    }
    else {
        Set-Content -Path $stateFile -Value $currentState
    }
}