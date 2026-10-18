# The MVP Baseball 2005 engine site and its server in one image, for Dokploy
# (deploy/docker-compose.yml; the owner's guide: docs/engine/DEPLOY_DOKPLOY.md).
#
# Build context: the repository root. The root .dockerignore admits only
# engine/, config/GV4E69/symbols.txt (repository metadata) and the context
# check. The image holds no game data: the owner uploads the disc image through
# the site afterwards, into the /data volume.
#
#   context  checks every file in the build context: no disc images, DOLs, ELFs (else the build fails)
#   web      the engine (WebAssembly) and the web shell, with the pinned Emscripten (docs/engine/TOOLCHAIN.md)
#   server   engine/server compiled with TypeScript; runtime dependencies only
#   runtime  Node 24 on Debian slim, as the unprivileged user "node"; one HTTP port; /data is the volume
#
# Reproducible: every base image is pinned by digest, every download by SHA-256,
# npm installs from the lockfiles, and the build runs in fixed paths. The image's
# /app/MANIFEST.txt lists the SHA-256 of every file it serves or runs.

ARG EMSDK_IMAGE=emscripten/emsdk:6.0.12@sha256:77638e6c215a65f25a704c0c831d5c2463a8c0468a58c6cb82489dbe621dfa05
ARG NODE_IMAGE=node:24.22.0-trixie-slim@sha256:7f486cabe892bff8fef04c0f7da8dba84a560be3e4e2740b7a86afc57ebec315

# ---- context: nothing game-derived may be here ----
FROM ${NODE_IMAGE} AS context
COPY . /ctx
RUN node /ctx/deploy/check-context.mjs /ctx && echo ok > /context-ok

# ---- web: the engine and the shell (build/engine/emscripten/web/site) ----
FROM ${EMSDK_IMAGE} AS web
COPY --from=context /context-ok /tmp/context-ok
ARG TARGETARCH
# Ninja (the emscripten preset's generator): github.com/ninja-build/ninja release assets.
ARG NINJA_VERSION=1.13.1
ARG NINJA_SHA256_AMD64=0830252db77884957a1a4b87b05a1e2d9b5f658b8367f82999a941884cbe0238
ARG NINJA_SHA256_ARM64=740f1b9f9d8ae68438240a6a2f3f7a27fc8b1946d2024a6a6b25857ee877987b
# emdawnwebgpu: the same Dawn release as the native renderer (docs/engine/TOOLCHAIN.md).
ARG DAWN_RELEASE=v20261013.193955
ARG EMDAWNWEBGPU_SHA256=c65cdd820c75354ef2ed7196f3dd43428c2b56f49f49a2c513263b593563ee3a
RUN set -eu; \
    case "${TARGETARCH:-amd64}" in \
      amd64) asset=ninja-linux.zip; sum="$NINJA_SHA256_AMD64" ;; \
      arm64) asset=ninja-linux-aarch64.zip; sum="$NINJA_SHA256_ARM64" ;; \
      *) echo "unsupported architecture: $TARGETARCH" >&2; exit 1 ;; \
    esac; \
    curl -fsSL --retry 3 -o /tmp/ninja.zip "https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/${asset}"; \
    echo "${sum}  /tmp/ninja.zip" | sha256sum -c -; \
    unzip -q /tmp/ninja.zip -d /usr/local/bin; \
    curl -fsSL --retry 3 -o /tmp/emdawnwebgpu.zip \
      "https://github.com/google/dawn/releases/download/${DAWN_RELEASE}/emdawnwebgpu_pkg-${DAWN_RELEASE}.zip"; \
    echo "${EMDAWNWEBGPU_SHA256}  /tmp/emdawnwebgpu.zip" | sha256sum -c -; \
    mkdir -p /opt/emdawnwebgpu; \
    unzip -q /tmp/emdawnwebgpu.zip -d /opt/emdawnwebgpu; \
    test -f /opt/emdawnwebgpu/emdawnwebgpu_pkg/emdawnwebgpu.port.py; \
    rm /tmp/ninja.zip /tmp/emdawnwebgpu.zip
WORKDIR /src
# TypeScript for the shell (and Playwright, a dev tool that is never run or served here), from the lockfile.
COPY engine/web/package.json engine/web/package-lock.json engine/web/
RUN cd engine/web && npm ci --ignore-scripts --no-audit --no-fund
COPY engine/ engine/
COPY config/GV4E69/symbols.txt config/GV4E69/symbols.txt
RUN cd engine \
 && cmake --preset emscripten -DMVP_EMDAWNWEBGPU_DIR=/opt/emdawnwebgpu/emdawnwebgpu_pkg \
 && cmake --build --preset emscripten --target mvp-web -j "$(nproc)"

# ---- server: engine/server/dist and its one runtime dependency ----
FROM ${NODE_IMAGE} AS server
COPY --from=context /context-ok /tmp/context-ok
WORKDIR /src/engine/server
COPY engine/server/package.json engine/server/package-lock.json ./
RUN npm ci --ignore-scripts --no-audit --no-fund
COPY engine/server/tsconfig.json ./
COPY engine/server/src src
COPY engine/server/test test
RUN node node_modules/typescript/bin/tsc -p tsconfig.json \
 && npm prune --omit=dev --ignore-scripts --no-audit --no-fund

# ---- runtime ----
FROM ${NODE_IMAGE}
ENV NODE_ENV=production \
    MVP_HOST=0.0.0.0 \
    MVP_PORT=8080 \
    MVP_SITE=/app/site \
    MVP_ACCESS_KEY_FILE=/data/access-key \
    MVP_ACCESS_KEY_CREATE=1 \
    MVP_DISC_DIR=/data/disc \
    MVP_PACKS_DIR=/data/packs \
    MVP_TURN_MODE=off \
    MVP_TURN_SECRET_FILE=/data/turn/secret \
    MVP_TURN_SECRET_CREATE=1 \
    MVP_SECURE_COOKIES=true
COPY --from=web /src/build/engine/emscripten/web/site /app/site
COPY --from=server /src/engine/server/package.json /app/server/package.json
COPY --from=server /src/engine/server/dist/src /app/server/dist/src
COPY --from=server /src/engine/server/node_modules /app/server/node_modules
# The site is the engine's own file types only (as engine/server/tools/package.mjs); the manifest; the volume's
# layout: /data (the access key, 0600), /data/disc (the uploaded disc, owner only), /data/packs (the uploaded
# roster packs, owner only), /data/turn (coturn's secret, readable by coturn's own user in its container).
RUN set -eu; \
    cd /app; \
    bad=$(find site -type f ! -regex '.*\.\(html\|css\|js\|wasm\|txt\|json\)$'); \
    if [ -n "$bad" ]; then echo "unexpected files in the site: $bad" >&2; exit 1; fi; \
    test -f site/index.html; test -f site/mvp-web.wasm; test -f site/main.js; test -f site/build-id.json; \
    find site server -type f | LC_ALL=C sort | xargs sha256sum > MANIFEST.txt; \
    mkdir -p /data/disc /data/packs /data/turn; \
    chown node:node /data /data/disc /data/packs /data/turn; \
    chmod 0755 /data /data/turn; \
    chmod 0700 /data/disc /data/packs
USER node
WORKDIR /app/server
VOLUME ["/data"]
EXPOSE 8080
HEALTHCHECK --interval=30s --timeout=5s --start-period=30s --retries=3 \
  CMD ["node", "-e", "fetch('http://127.0.0.1:' + (process.env.MVP_PORT || 8080) + '/healthz').then((r) => process.exit(r.ok ? 0 : 1), () => process.exit(1))"]
CMD ["node", "dist/src/main.js"]
