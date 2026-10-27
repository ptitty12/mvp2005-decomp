#!/usr/bin/env bash
# Your own copy of the MVP Baseball 2005 site, on this computer (macOS, Linux).
# Windows: run-local.cmd. What it is and what you need: docs/GETTING_STARTED.md.
#
#   ./run-local.sh           build (the first time takes a while), start, print the links, open the page
#   ./run-local.sh links     print the links again
#   ./run-local.sh logs      follow the server's log (Ctrl+C to stop following)
#   ./run-local.sh stop      stop it; the keys and anything you uploaded are kept
#   ./run-local.sh reset     stop it and delete what it kept: the keys, the uploaded disc, packs, save states
#
# MVP_LOCAL_PORT=8081 ./run-local.sh uses another port (the default is 8080).
# --no-open after any command skips opening the browser.
set -euo pipefail
cd "$(dirname "$0")"

export MVP_LOCAL_PORT="${MVP_LOCAL_PORT:-8080}"
compose=(docker compose -f deploy/docker-compose.standalone.yml -p mvp-standalone)
site="http://localhost:${MVP_LOCAL_PORT}"

fail() {
  echo "run-local: $*" >&2
  exit 1
}

check_docker() {
  command -v docker >/dev/null 2>&1 ||
    fail "Docker is not installed. Install Docker Desktop (macOS) or Docker Engine (Linux): https://docs.docker.com/get-docker/"
  docker info >/dev/null 2>&1 ||
    fail "Docker is installed but not running (or this user may not use it). Start Docker Desktop, or the docker service, and try again."
  docker compose version >/dev/null 2>&1 ||
    fail "this needs Docker Compose v2 (the 'docker compose' command): https://docs.docker.com/compose/install/"
}

key() {
  "${compose[@]}" exec -T mvp cat "/data/$1" 2>/dev/null | tr -d '\r\n'
}

links() {
  local admin access
  admin=$(key admin-key) || true
  access=$(key access-key) || true
  [ -n "$admin" ] || fail "it is not running. Start it with ./run-local.sh"
  cat <<EOF

MVP Baseball 2005 is running on this computer.

  Play:        ${site}/
               Pick your disc image (.iso, .ciso or .gcm) on the page.
  Admin link:  ${site}/?admin=${admin}
               Yours alone. Open it once in your browser for the Server disc page:
               upload the disc there once and the game starts without picking it,
               and roster packs and save states are kept here.
  Player link: ${site}/?key=${access}
               Lets a browser use the uploaded disc (a second window, to try online play).

Stop it with ./run-local.sh stop. Your uploads are kept until ./run-local.sh reset.
EOF
}

open_page() {
  if command -v open >/dev/null 2>&1 && [ "$(uname -s)" = Darwin ]; then
    open "$site/" || true
  elif command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$site/" >/dev/null 2>&1 || true
  fi
}

no_open=0
args=()
for a in "$@"; do
  if [ "$a" = --no-open ]; then no_open=1; else args+=("$a"); fi
done

case "${args[0]:-start}" in
  start)
    check_docker
    echo "run-local: building and starting (the first build downloads about 1 GB of build tools and compiles the engine:"
    echo "           usually 5 to 15 minutes; later starts take seconds)"
    "${compose[@]}" up -d --build --wait --wait-timeout 180
    links
    [ "$no_open" = 1 ] || open_page
    ;;
  links)
    check_docker
    links
    ;;
  logs)
    check_docker
    "${compose[@]}" logs -f mvp
    ;;
  stop)
    check_docker
    "${compose[@]}" down
    ;;
  reset)
    check_docker
    read -r -p "Delete the keys, the uploaded disc, roster packs and save states kept by this copy? [y/N] " answer
    case "$answer" in
      y | Y | yes) "${compose[@]}" down -v ;;
      *) echo "run-local: nothing deleted" ;;
    esac
    ;;
  *)
    fail "unknown command '${args[0]}' (start, links, logs, stop or reset)"
    ;;
esac
