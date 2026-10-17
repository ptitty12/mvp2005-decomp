# Deploying with Dokploy

How to put the game site on your VPS using only Dokploy's web pages. You don't
need SSH and you don't type any commands. The repository and the Docker image
hold no game data. You upload your disc image through the site once, after it
is running. It stays in the app's private storage on your server through every
later deploy (section "Where the disc is kept"). Players never upload or pick a
disc.

Technical details for agents are in `deploy/docker-compose.yml`, `deploy/Dockerfile`
and `docs/engine/NETPLAY.md`.

## Before you start

- **A domain or subdomain**, for example `mvp.yourdomain.com`. In your DNS
  provider's web panel, give it an **A record** pointing to the VPS's public IP
  address. If you use Cloudflare, set that record to **DNS only** (grey cloud).
- **The VPS** needs at least 2 GB of memory and about 6 GB of free disk. The first
  deploy compiles the engine twice (the regular one and the faster threaded one),
  which takes roughly 20 to 40 minutes on a small VPS.

## 1. Create the app

1. In Dokploy, open (or create) a project, then **Create Service → Compose**.
   Name it `mvp2005`, keep the type **Docker Compose**, and create it.
2. Open the **General** tab and choose the **Git** provider:
   - Repository URL: `https://github.com/ptitty12/mvp2005-decomp.git`
   - Branch: `main`
   - Compose Path: `./deploy/docker-compose.yml`

   Click **Save**.

## 2. Domain

Open the **Domains** tab and click **Add Domain**:

| Field | Value |
|---|---|
| Service Name | `mvp` |
| Host | your domain, for example `mvp.yourdomain.com` |
| Path | `/` |
| Container Port | `8080` |
| HTTPS | on |
| Certificate | Let's Encrypt |

Click **Create**. Only `mvp` gets a domain. The other service, `coturn`, uses its
own ports (section 4).

## 3. Settings (optional)

Open the **Environment** tab. Everything works with nothing set. Recommended:

```
TURN_EXTERNAL_IP=<your VPS's public IPv4 address>
```

Other settings, only if you need them:

| Setting | When |
|---|---|
| `TURN_HOST=<VPS IP address>` | Your domain goes through a proxy, such as Cloudflare's orange cloud. |
| `TURN_MODE=stun` or `off` | You want no relay (`stun`), or no STUN and no relay (`off`). The default is `relay`. |
| `SETUP_HOURS=<hours>` | How long the create-password page stays open after the first start. The default is 48. |
| `RESET_PASSWORD=<any new word>` | You forgot the password, or someone else created it. Set a word you haven't used before, then deploy. The password is removed once and the create-password page comes back. |

Click **Save**.

## 4. Firewall

If your VPS provider has a firewall in its web panel, allow these **inbound**
ports so online games can connect through the relay:

- UDP 3478
- TCP 3478
- UDP 49160-49200

Ports 80 and 443 are already open for Dokploy.

## 5. Deploy

On the **General** tab, click **Deploy**. Watch the **Deployments** tab: the first
deploy is done when the log ends without errors and both services (`mvp` and
`coturn`) are running. Later deploys, from the same button, are much faster.

## 6. Create the password (right away)

Open `https://<your domain>/` as soon as the deploy finishes. The first visit
shows **create the password**:

1. Type a password of at least 12 characters twice. A few unrelated words work well.
2. Click **Create the password**. You are now logged in.

If the first visit shows a **login** form instead, someone else created the
password first. Set `RESET_PASSWORD` (section 3), deploy again, and repeat this
step. Until a password exists, nothing else on the site works, and no disc can
be uploaded.

## 7. Upload the disc

1. Open `https://<your domain>/disc`. The game page also links to it: **Server
   disc (site operator)**, in small print at the very bottom.
2. Click **Choose file** and pick
   `C:\Users\Patrick Taylor\decompile_mvp_04\orig\GV4E69\MVP Baseball 2005 (USA).ciso`.
3. Click **Upload** and keep the page open. The page shows the progress and the
   time left (about 1.4 GB).

   The server checks the file early and refuses anything that isn't MVP Baseball
   2005, GV4E69 revision 0.

   If the upload stops (lost connection, closed page), choose the same file and
   click **Upload** again. It continues where it stopped.
4. When it says **Done**, the server has the disc, and you never upload it again.
   The game page never shows a file picker or an upload button to players.

## 8. Add roster packs (optional)

Roster packs change the rosters, players and team names (for example the MLB 2026
pack, `docs/MLB2026.md`). Put them on the server and every player picks them from a
list on the game page instead of bringing the file.

1. Open `https://<your domain>/disc` again. Under **Roster packs**, click **Choose
   file**, pick the pack's `.zip` (the MLB 2026 pack is
   `C:\Users\Patrick Taylor\decompile_mvp_04-mlb2026\build\mlb2026\pack\mlb-2026.zip`),
   and click **Upload**. The server checks that it is a roster pack for this game.
2. On the game page, the **Roster packs** panel now has **Packs on this server**.
   Choose the pack there before pressing Play. The page remembers it for next time.
3. For an online game both of you choose the same packs in the same order; the game
   refuses to connect otherwise and says why.

Uploading a new version of a pack replaces the old one; **Remove** takes a pack off
the list. Packs are kept in the same volume as the disc, behind the same password.

## 9. Play with your brother

Send your brother the site address and, separately, the password.

The first time a browser opens the game page, it downloads the game data from
your server once, about 1.4 GB, with a progress bar ("Downloading the game data
from the server, once: 34% ..."). The browser keeps it, so the game reads the
disc from the player's own computer and never waits on the network while you
play. Later visits start at once. If the download stops (a closed tab, a lost
connection), reloading the page continues where it stopped.

When the game is ready, either of you can:

- press **Play** to play alone; or
- **create a game** and send the invite link. The other player opens the link
  and joins.

The browser needs about 1.5 GB of free disk space for this. In a private or
incognito window it cannot keep the game data; the page then says so and offers
to play while reading the disc from the server, which pauses while the game
loads.

## Later

- **Update:** click **Deploy** again. It rebuilds from `main`. The password and
  the disc stay. Both players reload the page afterwards. Right after the first
  deploy of this version the page may say "The server is preparing the game data"
  for a few seconds while the server checks the disc once. Players' browsers keep
  their copy and download again only if you upload a different disc.
- **Online game doesn't connect:** check the firewall ports (section 4), set
  `TURN_EXTERNAL_IP`, and if the domain is proxied, set `TURN_HOST`. Then deploy
  again.
- **Removing everything:** deleting the app in Dokploy with its volumes also
  deletes the stored password, the uploaded disc and the roster packs.

## Where the disc is kept

The disc you uploaded, the password and the relay's secret are stored in the
app's volume, `mvp-data` (Docker names it after the app: the app's name, then
`_mvp-data`). It is not part of the code Dokploy downloads from GitHub, and
Dokploy deploys the app under the same name every time, so every deploy uses the
same volume:

- **Kept:** Deploy, Redeploy, Rebuild, Stop and Start, automatic deploys,
  restarting the VPS, and changes on the Environment or Domains tabs.
- **Lost:** deleting the app with its volumes, and deleting the app and creating
  a new one (a new app gets a new, empty volume: upload the disc again).
- **Leave Randomize Compose off** (the app's Advanced tab). It gives the volume a
  new name, so the app would start with an empty volume.
