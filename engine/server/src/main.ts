// Entry point: `node dist/src/main.js` with the configuration in the
// environment (src/config.ts; docs/engine/NETPLAY.md, "Deploying on a VPS", and
// docs/engine/DEPLOY_DOKPLOY.md, "Your admin link").
import { adminLink, privateLink } from './access.js';
import { loadConfig } from './config.js';
import { startServer } from './server.js';

try {
  const cfg = loadConfig();
  const app = await startServer(cfg);
  console.log(
    `mvp-server: listening on ${app.url}; site ${cfg.site}; disc ${cfg.disc ? 'configured' : 'not configured'}; ` +
      `turn ${cfg.turn ? 'configured' : 'not configured'}; the site is open to anyone`,
  );
  // The owner reads the links here (Dokploy: the mvp service's Logs tab), at every start.
  const origin = cfg.publicUrl ? new URL(cfg.publicUrl).origin : "https://<this site's address>";
  if (cfg.adminKey) {
    console.log(
      `mvp-server: admin link: ${adminLink(origin, cfg.adminKey)}  ` +
        "(yours only: open it once in each of your browsers for the Server disc page and your save states; don't share it)",
    );
  } else {
    console.log('mvp-server: no admin key (MVP_ADMIN_KEY or MVP_ADMIN_KEY_FILE): no Server disc page and no save states');
  }
  console.log(
    `mvp-server: player link: ${privateLink(origin, cfg.accessKey)}  ` +
      "(lets a browser play with this server's disc; lobby invites carry it)",
  );
  const stop = () => {
    app.close().then(() => process.exit(0));
  };
  process.on('SIGINT', stop);
  process.on('SIGTERM', stop);
} catch (e) {
  console.error(`mvp-server: ${(e as Error).message}`);
  process.exit(1);
}
