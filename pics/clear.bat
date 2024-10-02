@echo off
setlocal enabledelayedexpansion

for %%f in (*.*) do (
    echo %%f | findstr /i /v "template" >nul
    set "has_template=!errorlevel!"
    echo %%f | findstr /i /v ".bat" >nul
    set "has_bat=!errorlevel!"
    if !has_template! neq 0 if !has_bat! neq 0 (
        del "%%f"
    )
)

endlocal
