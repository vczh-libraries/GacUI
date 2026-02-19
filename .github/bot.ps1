Push-Location $PSScriptRoot\Agent
try {
    yarn install
    yarn compile
    start powershell {yarn portal}
}
finally {
    Pop-Location
}