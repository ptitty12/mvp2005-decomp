@echo off
rem Your own copy of the MVP Baseball 2005 site, on this computer (Windows).
rem macOS and Linux: run-local.sh. What it is and what you need: docs\GETTING_STARTED.md.
rem
rem   run-local           build (the first time takes a while), start, print the links, open the page
rem                       (double-clicking the file does the same)
rem   run-local links     print the links again
rem   run-local logs      follow the server's log (Ctrl+C to stop following)
rem   run-local stop      stop it; the keys and anything you uploaded are kept
rem   run-local reset     stop it and delete what it kept: the keys, the uploaded disc, packs, save states
rem
rem "set MVP_LOCAL_PORT=8081" first uses another port (the default is 8080).
rem --no-open as the second word skips opening the browser.
setlocal EnableExtensions
cd /d "%~dp0"

if "%MVP_LOCAL_PORT%"=="" set "MVP_LOCAL_PORT=8080"
set "COMPOSE=docker compose -f deploy/docker-compose.standalone.yml -p mvp-standalone"
set "SITE=http://localhost:%MVP_LOCAL_PORT%"
set "RC=0"

set "ACTION=%~1"
if "%ACTION%"=="" set "ACTION=start"
if /i "%ACTION%"=="--no-open" set "ACTION=start"
set "NOOPEN="
if /i "%~1"=="--no-open" set "NOOPEN=1"
if /i "%~2"=="--no-open" set "NOOPEN=1"

call :check_docker || goto :failed

if /i "%ACTION%"=="start" goto :start
if /i "%ACTION%"=="links" goto :links_only
if /i "%ACTION%"=="logs" goto :logs
if /i "%ACTION%"=="stop" goto :stop
if /i "%ACTION%"=="reset" goto :reset
echo run-local: unknown command '%ACTION%' (start, links, logs, stop or reset)
goto :failed

:start
echo run-local: building and starting (the first build downloads about 1 GB of build tools and compiles the engine:
echo            usually 5 to 15 minutes; later starts take seconds)
%COMPOSE% up -d --build --wait --wait-timeout 180 || goto :failed
call :links || goto :failed
if not defined NOOPEN start "" "%SITE%/"
goto :done

:links_only
call :links || goto :failed
goto :done

:logs
%COMPOSE% logs -f mvp
goto :done

:stop
%COMPOSE% down || goto :failed
goto :done

:reset
set "ANSWER="
set /p "ANSWER=Delete the keys, the uploaded disc, roster packs and save states kept by this copy? [y/N] "
if /i "%ANSWER%"=="y" goto :reset_yes
if /i "%ANSWER%"=="yes" goto :reset_yes
echo run-local: nothing deleted
goto :done
:reset_yes
%COMPOSE% down -v || goto :failed
goto :done

:check_docker
where docker >nul 2>nul
if errorlevel 1 (
  echo run-local: Docker is not installed. Install Docker Desktop: https://docs.docker.com/desktop/setup/install/windows-install/
  exit /b 1
)
docker info >nul 2>nul
if errorlevel 1 (
  echo run-local: Docker Desktop is installed but not running. Start it, wait until it says it is running, and try again.
  exit /b 1
)
docker compose version >nul 2>nul
if errorlevel 1 (
  echo run-local: this needs Docker Compose v2 ^(the "docker compose" command^), which comes with Docker Desktop.
  exit /b 1
)
exit /b 0

:links
set "ADMIN="
set "ACCESS="
for /f "usebackq delims=" %%k in (`%COMPOSE% exec -T mvp cat /data/admin-key 2^>nul`) do set "ADMIN=%%k"
for /f "usebackq delims=" %%k in (`%COMPOSE% exec -T mvp cat /data/access-key 2^>nul`) do set "ACCESS=%%k"
if not defined ADMIN (
  echo run-local: it is not running. Start it with run-local
  exit /b 1
)
echo.
echo MVP Baseball 2005 is running on this computer.
echo.
echo   Play:        %SITE%/
echo                Pick your disc image (.iso, .ciso or .gcm) on the page.
echo   Admin link:  %SITE%/?admin=%ADMIN%
echo                Yours alone. Open it once in your browser for the Server disc page:
echo                upload the disc there once and the game starts without picking it,
echo                and roster packs and save states are kept here.
echo   Player link: %SITE%/?key=%ACCESS%
echo                Lets a browser use the uploaded disc (a second window, to try online play).
echo.
echo Stop it with "run-local stop". Your uploads are kept until "run-local reset".
exit /b 0

:failed
set "RC=1"

:done
rem Double-clicked (Explorer runs it with cmd /c): keep the window open so the links can be read.
set "CCL=%CMDCMDLINE:"=%"
if /i not "%CCL: /c =%"=="%CCL%" pause
exit /b %RC%
