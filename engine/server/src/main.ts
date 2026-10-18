// Entry point: `node dist/src/main.js` with the configuration in the
// environment (src/config.ts; docs/engine/NETPLAY.md, "Deploying on a VPS" and
// "The private link").
import { privateLink } from './access.js';
import { loadConfig } from './config.js';
import { startServer } from './server.js';

try {
  const cfg = loadConfig();
  const app = await startServer(cfg);
  console.log(
    `mvp-server: listening on ${app.url}; site ${cfg.site}; disc ${cfg.disc ? 'configured' : 'not configured'}; ` +
      `turn ${cfg.turn ? 'configured' : 'not configured'}; access by private link`,
  );
  // The owner reads the link here (Dokploy: the mvp service's Logs tab), at every start.
  const origin = cfg.publicUrl ? new URL(cfg.publicUrl).origin : "https://<this site's address>";
  console.log(
    `mvp-server: private link: ${privateLink(origin, cfg.accessKey)}  ` +
      '(open it once in each browser; anyone with this link can play and download the disc)',
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
