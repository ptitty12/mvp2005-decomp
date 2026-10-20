# Deploying with Dokploy

How to put the game site on your VPS using only Dokploy's web pages. You don't
need SSH and you don't type any commands. The repository and the Docker image
hold no game data. You upload your disc image through the site once, after it
is running. It stays in the app's private storage on your server through every
later deploy (section "Where the disc is kept"). Players with your player link
never upload or pick a disc.

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
| `ACCESS_KEY=<16 or more letters and digits>` | You want a new player link (section 6), for example because the old one leaked. Deploy after setting it: every browser then needs the new link to play with your disc. Leave it set; removing it brings the old link back. |
| `ADMIN_KEY=<16 or more letters and digits>` | You want a new admin link (section 6). Deploy after setting it, then open the new admin link in your browsers. Leave it set; removing it brings the old link back. |

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

## 6. Get your admin link

There is no password, and the site is open: anyone who opens your domain gets the
game page and can play with a disc image of their own. Two links, which the server
makes on the first start, give a browser more:

- **Your admin link** (`?admin=...`): yours only. The Server disc page (sections 7
  and 8) and your save states (section 10). It also lets that browser play with your
  disc.
- **The player link** (`?key=...`): it lets a browser play with your disc instead of
  picking its own (section 9).

1. Open the app's **Logs** tab and choose the service **mvp**. Find the line that
   starts with `mvp-server: admin link:`. It looks like
   `https://<this site's address>/?admin=Xy3...`.
2. Put your domain in place of `<this site's address>`, for example
   `https://mvp.yourdomain.com/?admin=Xy3...`, and open that in your browser.

The browser keeps it for 400 days, and every visit starts that count again. The
address bar shows just your domain; nobody types anything. Open the admin link once
in each browser you use. Don't send it to anyone: whoever opens it can change the
disc and the packs, and see your save states.

Both links stay the same through every deploy. The server's disc stays behind the
player link, because anyone with the address could otherwise download the whole
game. Browsers that opened the old private link before this version keep playing
with your disc. To replace a link, see `ACCESS_KEY` and `ADMIN_KEY` in section 3.

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
   The game page never shows a file picker or an upload button to players with
   your player link (or your admin link).

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
the list. Packs are kept in the same volume as the disc. Anyone on the site can choose
them; only you can upload or remove them.

## 9. Play with your brother

If your brother already opened the old private link, nothing changes for him.
Otherwise send him the player link (copy it on the Server disc page): he opens it
once in his browser, and it plays with your disc from then on. An invite link
(below) works too: it carries the same key. Without it he can still play with a disc
image of his own.

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

## 10. Your save states

In your browser (the one that opened the admin link), the game page's **Save
states** panel has a section **On the server**:

- **F2** saves where the game is, in this browser, and copies it to **your slot** on
  the server. **F4** goes back to it; in a browser without a save of its own (your
  laptop, a new browser), F4 takes the one in your slot.
- **Save to the server**, with a name typed next to it, keeps a save under that
  name.
- The list shows your states on the server: **Load** puts one into the running game
  (it needs the same roster packs and gameplay mods); **Start from it** reloads the
  page and sets those up first. A link like `https://<your domain>/?state=<name>`
  does the same.

The Server disc page has the same list, with **Remove**, and uploads a `.mvpstate`
file from your computer under a name (for example the ABS states in
`C:\Users\Patrick Taylor\decompile_mvp_04-states\build\states`). Save states work in
every game you play alone, with or without gameplay mods; not in an online game.
Nobody else can see or open them.

## 11. Your memory card

The game saves profiles, seasons and settings on its memory card in slot A, from
its own menus. Every browser keeps its own card. Yours (the browser that opened
the admin link) is also kept on the server, so your saves follow you to your
laptop or a new browser: the game starts from the server's card, and each time
the game saves, the card goes back to the server.

If a tab that has been open for a while saves after another browser has saved,
the server keeps the newer card and that tab says so; reload it to play on the
server's card. The **Memory card** panel on the game page downloads the card
and takes a card file back. Online games have no memory card.

## Later

- **Update:** click **Deploy** again. It rebuilds from `main`. The links, the disc,
  the packs, your save states and your memory card stay. Both players reload the page afterwards. Right after the first
  deploy of this version the page may say "The server is preparing the game data"
  for a few seconds while the server checks the disc once. Players' browsers keep
  their copy and download again only if you upload a different disc.
- **Online game doesn't connect:** check the firewall ports (section 4), set
  `TURN_EXTERNAL_IP`, and if the domain is proxied, set `TURN_HOST`. Then deploy
  again.
- **A link leaked:** set `ACCESS_KEY` (the player link) or `ADMIN_KEY` (the admin
  link) in section 3 and deploy. The old link stops working in every browser; open
  or send the new one.
- **Removing everything:** deleting the app in Dokploy with its volumes also
  deletes the links' keys, the uploaded disc, the roster packs, your save states and your memory card.

## Where the disc is kept

The disc you uploaded, the packs, your save states, your memory card, the keys of the links and the relay's secret are stored in the
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
