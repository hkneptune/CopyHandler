@echo off

call "%WORKSPACE%\scripts\jenkins\_get-version.bat" || exit /b 1

if "%CHDetailVersion%" == "0" (
	SET "OutDir=%WORKSPACE%\Output\%CHTextVersion%"
) else (
	SET "OutDir=%WORKSPACE%\Output\internal\%CHTextVersion%"
)

echo "Detected output directory: %OutDir%"

mkdir "%OutDir%" || exit /b 0
