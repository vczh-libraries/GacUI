Push-Location $PSScriptRoot\Agent
try {
    yarn install
    yarn compile
    start powershell {yarn portal --port 9999}
}
finally {
    Pop-Location
}