@echo off

setlocal enabledelayedexpansion

set n=0
set fail=0

for %%f in (examples\*.pwc) do (
  echo %%f
  build\Debug\powerc %%f
  set /a n+=1
  if !errorlevel! neq 0 (
    set /a fail+=1
  )
  echo ----------------------
)

echo %n% file(s) tested, %fail% failed
endlocal
