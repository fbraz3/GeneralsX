#!/usr/bin/osascript                                                                            

-- Script to execute commands in macOS Terminal.app (SYNCHRONOUS)
-- Usage: runTerminal.sh "command" [profile]
-- The script waits for the command to finish before returning
-- The terminal will be closed automatically after command execution
-- 
-- Phase 54: Added 60-second timeout with SIGKILL to prevent fullscreen lockups
-- The game will be forcefully terminated if it doesn't exit within the timeout

on run argv                                                                                     
  if length of argv is equal to 0                                                               
    set command to ""                                                                           
  else                                                                                          
    set command to item 1 of argv
    -- Phase 54: Wrap command with timeout (60 seconds, SIGKILL on timeout)
    -- This prevents fullscreen lockups from blocking the system
    set command to "date; echo '[runTerminal] Starting with 60s timeout...'; timeout -s 9 60 " & command & "; EXIT_CODE=$?; if [ $EXIT_CODE -eq 137 ]; then echo '[runTerminal] TIMEOUT: Process killed after 60 seconds'; fi; echo '[runTerminal] Exit code: '$EXIT_CODE; exit"
  end if                                                                                        

  if length of argv is greater than 1                                                           
    set profile to item 2 of argv                                                               
    runWithProfileSync(command, profile)                                                            
  else                                                                                          
    runSimpleSync(command)                                                                          
  end if                                                                                        
end run                                                                                         

on runSimpleSync(command)                                                                           
  tell application "Terminal"                                                                   
    activate                                                                                    
    set newTab to do script(command)
    
    -- Wait for the process to finish (polling until tab closes or becomes idle)
    set tabWindow to window 1
    repeat
      delay 1
      try
        -- Check if the tab still exists and is busy
        if not (exists newTab) then exit repeat
        if not busy of newTab then exit repeat
      on error
        -- Tab was closed
        exit repeat
      end try
    end repeat
  end tell                                                                                      
  return "completed"                                                                                 
end runSimpleSync                                                                                   

on runWithProfileSync(command, profile)                                                             
  tell application "Terminal"                                                                   
    activate                                                                                    
    set newTab to do script(command)
    set current settings of newTab to (first settings set whose name is profile)
    
    -- Wait for the process to finish
    repeat
      delay 1
      try
        if not (exists newTab) then exit repeat
        if not busy of newTab then exit repeat
      on error
        exit repeat
      end try
    end repeat
  end tell
  return "completed"                                                                                      
end runWithProfileSync