#!/bin/sh
# Installs the MVP Baseball 2005 netplay deployment on a fresh Debian 12 or
# Ubuntu 24.04 VPS (E5). Run as root from the unpacked package directory:
#
#   ./deploy/install.sh mvp.example.com
#
# - DOMAIN must already resolve to this VPS (Caddy gets its certificate for it).
# - There is no password: the site is open to anyone. Two links, which the
#   script prints at the end (and the server prints at every start:
#   journalctl -u mvp-server), give a browser more: the player link,
#   https://DOMAIN/?key=<key>, lets it play with the server's disc; the admin
#   link, https://DOMAIN/?admin=<key>, is the owner's (the Server disc page and
#   the save states). The keys are made here once, in /etc/mvp/access-key and
#   /etc/mvp/admin-key; a browser that opened a link keeps it for 400 days. To
#   replace a link (e.g. a leaked one), put a new key of 16 or more letters and
#   digits in its file and restart mvp-server (docs/engine/NETPLAY.md).
# - The disc image is not part of the package: copy it to /srv/mvp/disc/ and
#   set "disc" in /etc/mvp/server.json (the script does this when
#   /srv/mvp/disc/GV4E69.ciso or GV4E69.iso exists).
#
# Idempotent: running it again updates the engine and the server, keeps the
# access key (so the link stays the same) and the TURN secret.
set -eu

DOMAIN=${1:?usage: install.sh DOMAIN}
NODE_VERSION=24.18.0
HERE=$(cd "$(dirname "$0")/.." && pwd)

if [ "$(id -u)" -ne 0 ]; then echo "install.sh: run as root" >&2; exit 1; fi

echo "== packages (Caddy, coturn)"
apt-get update -q
DEBIAN_FRONTEND=noninteractive apt-get install -y -q caddy coturn curl xz-utils ca-certificates

echo "== Node.js $NODE_VERSION (official build, checksum verified)"
if [ "$(/usr/local/bin/node --version 2>/dev/null || true)" != "v$NODE_VERSION" ]; then
  ARCH=$(uname -m); case "$ARCH" in x86_64) ARCH=x64;; aarch64) ARCH=arm64;; esac
  TMP=$(mktemp -d)
  curl -fsSL -o "$TMP/node.tar.xz" "https://nodejs.org/dist/v$NODE_VERSION/node-v$NODE_VERSION-linux-$ARCH.tar.xz"
  curl -fsSL -o "$TMP/SHASUMS256.txt" "https://nodejs.org/dist/v$NODE_VERSION/SHASUMS256.txt"
  (cd "$TMP" && grep " node-v$NODE_VERSION-linux-$ARCH.tar.xz\$" SHASUMS256.txt | sed 's/node-v.*/node.tar.xz/' | sha256sum -c -)
  tar -xJf "$TMP/node.tar.xz" -C /usr/local --strip-components=1
  rm -rf "$TMP"
fi

echo "== user and directories"
id mvp >/dev/null 2>&1 || useradd --system --home /opt/mvp --shell /usr/sbin/nologin mvp
install -d -m 0755 /opt/mvp /srv/mvp /srv/mvp/disc
install -d -m 0750 -g mvp /etc/mvp

echo "== engine site and server"
rm -rf /opt/mvp/site.new /opt/mvp/server.new
cp -r "$HERE/site" /opt/mvp/site.new
cp -r "$HERE/server" /opt/mvp/server.new
rm -rf /opt/mvp/site /opt/mvp/server
mv /opt/mvp/site.new /opt/mvp/site
mv /opt/mvp/server.new /opt/mvp/server
chown -R root:root /opt/mvp/site /opt/mvp/server

echo "== secrets (the TURN secret and the keys of the player and admin links)"
[ -s /etc/mvp/turn-secret ] || head -c 32 /dev/urandom | base64 | tr -d '/+=' > /etc/mvp/turn-secret
[ -s /etc/mvp/access-key ] || head -c 32 /dev/urandom | base64 | tr -d '/+=\n' > /etc/mvp/access-key
[ -s /etc/mvp/admin-key ] || head -c 32 /dev/urandom | base64 | tr -d '/+=\n' > /etc/mvp/admin-key
chown root:mvp /etc/mvp/turn-secret /etc/mvp/access-key /etc/mvp/admin-key
chmod 0640 /etc/mvp/turn-secret /etc/mvp/access-key /etc/mvp/admin-key

echo "== configuration"
DISC=""
for f in /srv/mvp/disc/GV4E69.ciso /srv/mvp/disc/GV4E69.iso; do [ -f "$f" ] && DISC="$f" && break; done
[ -n "$DISC" ] && chown root:mvp "$DISC" && chmod 0640 "$DISC"
sed -e "s#mvp.example.com#$DOMAIN#g" -e "s#/srv/mvp/disc/GV4E69.ciso#$DISC#" "$HERE/deploy/server.json.example" > /etc/mvp/server.json
cp "$HERE/deploy/server.env.example" /etc/mvp/server.env
chown root:mvp /etc/mvp/server.json /etc/mvp/server.env
chmod 0640 /etc/mvp/server.json /etc/mvp/server.env
sed -e "s#<mvp.example.com>#$DOMAIN#g" "$HERE/deploy/Caddyfile.template" > /etc/caddy/Caddyfile
sed -e "s#<mvp.example.com>#$DOMAIN#g" \
    -e "s#<the long random secret, same as MVP_TURN_SECRET>#$(cat /etc/mvp/turn-secret)#" \
    "$HERE/deploy/turnserver.conf.template" > /etc/turnserver.conf
chmod 0640 /etc/turnserver.conf
chown root:turnserver /etc/turnserver.conf 2>/dev/null || true
[ -f /etc/default/coturn ] && sed -i 's/^#\?TURNSERVER_ENABLED=.*/TURNSERVER_ENABLED=1/' /etc/default/coturn

echo "== services"
sed -e "s#/usr/bin/node#/usr/local/bin/node#" "$HERE/deploy/mvp-server.service" > /etc/systemd/system/mvp-server.service
systemctl daemon-reload
systemctl enable --now coturn mvp-server caddy
systemctl restart coturn mvp-server caddy

echo "== firewall (ufw, when present)"
if command -v ufw >/dev/null 2>&1; then
  ufw allow 22/tcp; ufw allow 80/tcp; ufw allow 443/tcp
  ufw allow 3478/udp; ufw allow 3478/tcp; ufw allow 49160:49200/udp
fi

sleep 2
curl -fsS http://127.0.0.1:8080/healthz && echo " <- mvp-server"
echo "== done"
echo "   admin link: https://$DOMAIN/?admin=$(cat /etc/mvp/admin-key)"
echo "   Yours only: open it once in each of your browsers (the Server disc page, your save states)."
echo "   player link: https://$DOMAIN/?key=$(cat /etc/mvp/access-key)"
echo "   It lets a browser play with the server's disc; anyone with it can download the disc."
