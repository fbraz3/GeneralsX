# Test script to capture game logs

$logPath = "c:\Users\Felipe\Projects\GeneralsX\logs\game_run.log"
$gamePath = "$env:USERPROFILE\GeneralsX\GeneralsMD\GeneralsXZH.exe"

Write-Host "Running game with output capture..."
Write-Host "Log file: $logPath"

&$gamePath -win -noshellmap 2>&1 | Tee-Object -FilePath $logPath

Write-Host "Game exited. Log saved to: $logPath"
Write-Host "First 50 lines of output:"
Get-Content $logPath -Head 50
