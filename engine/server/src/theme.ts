// The look of the server's own pages (the Server disc page, the private-server page): the game page's
// theme (engine/web/static/style.css), which follows MVP Baseball 2005's own front end: gunmetal
// bands, silver-framed panels with a silver heading bar and a slanted blue tab, gray menu bars, and
// the maroon bar with gold lettering for the main action. Fonts are the machine's own (the pages
// load nothing). Each page adds its own rules and pins the whole style by hash in its CSP.

export const THEME_CSS = `:root{color-scheme:dark;--fg:#ecece6;--muted:#a7a9ab;--gold:#dec654;--title-red:#8c0000;
--silver:#cfd2d2;--silver-lo:#7d8080;--gun-hi:#6e6d75;--gun:#32343b;--band:#070a14;
--blue-hi:#4b75d8;--blue:#24489f;--blue-lo:#11307c;
--face:Bahnschrift,"Franklin Gothic Medium","Arial Narrow",Arial,sans-serif;
--script:"Brush Script MT","Segoe Script","Lucida Handwriting",cursive;
--outline:-1px -1px 0 #000,1px -1px 0 #000,-1px 1px 0 #000,1px 1px 0 #000,0 2px 2px rgba(0,0,0,.5);
--silver-bar:linear-gradient(180deg,#fbfbfb 0%,#dedede 45%,#b2b2b2 56%,#cacaca 100%);
--pick-bar:linear-gradient(180deg,#74393a 0%,#602a2b 45%,#521f20 100%)}
*{box-sizing:border-box}[hidden]{display:none!important}
html{background:#0c0d11}
body{margin:0;min-height:100vh;color:var(--fg);font:15px/1.45 var(--face);
background:radial-gradient(110% 60% at 50% -12%,rgba(132,136,148,.24),transparent 62%),
repeating-linear-gradient(0deg,rgba(255,255,255,.022) 0 1px,transparent 1px 3px),linear-gradient(180deg,#17181d 0%,#0b0c0f 100%)}
a{color:var(--gold)}
.topbar{height:58px;background:linear-gradient(180deg,#0d1222 0%,var(--band) 58%,#000 100%);
border-bottom:1px solid var(--gun-hi);box-shadow:0 1px 0 #000,0 4px 0 var(--gun),0 5px 0 #000}
.title{max-width:712px;margin:0 auto;padding:14px 16px 0;font-weight:normal;line-height:1}
.title-script{position:relative;z-index:0;display:inline-block;padding:0 46px 0 18px;font:italic 700 40px/1 var(--script);
color:var(--title-red);white-space:nowrap;text-shadow:-2px -2px 0 #fff,2px -2px 0 #fff,-2px 2px 0 #fff,2px 2px 0 #fff,
0 -2px 0 #fff,0 2px 0 #fff,-2px 0 0 #fff,2px 0 0 #fff,3px 4px 3px rgba(0,0,0,.7)}
.title-script::before{content:'';position:absolute;z-index:-1;left:0;right:0;bottom:2px;height:52%;
background:linear-gradient(180deg,#fff 0%,#e3e5e5 42%,#a9acac 70%,#c9cbcb 100%);border-bottom:2px solid #000;
clip-path:polygon(0 0,calc(100% - 26px) 0,100% 100%,0 100%)}
main{max-width:712px;margin:30px auto 40px;padding:0 16px;display:grid;gap:18px}
.card{position:relative;padding:0 14px 14px;display:grid;gap:10px;
background:linear-gradient(180deg,rgba(38,40,46,.97) 0%,rgba(22,23,27,.97) 100%);
border:3px solid;border-color:var(--silver) var(--silver-lo) var(--silver-lo) var(--silver);outline:1px solid #000;
box-shadow:inset 0 0 0 1px #000,0 8px 20px rgba(0,0,0,.5)}
.card>h1,.card>h2{position:relative;margin:0 -14px 2px;padding:4px 14px 3px 36px;background:var(--silver-bar);
border-bottom:1px solid #000;color:#0b0b0b;font:700 17px/1.25 var(--face);font-stretch:condensed;
text-transform:uppercase;letter-spacing:.04em}
.card>h1::before,.card>h2::before{content:'';position:absolute;left:0;top:0;bottom:0;width:22px;
background:linear-gradient(180deg,var(--blue-hi) 0%,var(--blue) 50%,var(--blue-lo) 100%);clip-path:polygon(0 0,100% 0,60% 100%,0 100%)}
p{margin:0}.note,small{color:var(--muted);font-size:14px}.err{color:#ff9c8f}.ok{color:#a6e3a0}
button{font:700 15px/1.2 var(--face);font-stretch:condensed;letter-spacing:.02em;padding:5px 12px 4px;border:1px solid;
border-color:#fff #6f6f6f #4a4a4a #e6e6e6;border-radius:0;background:var(--silver-bar);color:#101010;cursor:pointer;
box-shadow:0 0 0 1px #000}
button:hover:not(:disabled){background:linear-gradient(180deg,#fff 0%,#ececec 45%,#c4c4c4 56%,#dadada 100%)}
button:active:not(:disabled){background:linear-gradient(180deg,#b6b6b6 0%,#cfcfcf 50%,#e8e8e8 100%)}
button:focus-visible{outline:2px solid var(--gold);outline-offset:2px}
button:disabled{opacity:.45;cursor:default}
button.primary{background:var(--pick-bar);border-color:#481212 #2b0d0d #2b0d0d #481212;color:var(--gold);
font-variant:small-caps;letter-spacing:.05em;text-shadow:var(--outline)}
button.primary:hover:not(:disabled){background:linear-gradient(180deg,#8a4546 0%,#70302f 45%,#5d2324 100%);color:#f3dc6c}
input{font:14px var(--face);padding:4px 6px;border:1px solid;border-color:#000 #8f9292 #8f9292 #000;border-radius:0;
background:#0d0e11;color:var(--fg);max-width:100%}
input:focus-visible{outline:2px solid var(--gold);outline-offset:0}
input[type=file]{padding:0;border:0;background:none;color:var(--muted);font-size:13px}
input[type=file]::file-selector-button{margin-right:8px;font:700 14px/1.2 var(--face);font-stretch:condensed;padding:3px 10px 2px;
border:1px solid #000;border-radius:0;background:var(--silver-bar);color:#101010;cursor:pointer}
progress{display:block;width:100%;height:12px;appearance:none;border:1px solid;border-color:#000 #8f9292 #8f9292 #000;background:#0d0e11}
progress::-webkit-progress-bar{background:#0d0e11}
progress::-webkit-progress-value{background:linear-gradient(180deg,#f6e48a 0%,var(--gold) 45%,#a88b25 100%)}
progress::-moz-progress-bar{background:linear-gradient(180deg,#f6e48a 0%,var(--gold) 45%,#a88b25 100%)}
code{font:13px ui-monospace,Consolas,monospace;word-break:break-all}
@media (max-width:560px){.title-script{font-size:30px;padding-right:34px}main{padding:0 10px}}`;

/** The top band with the title on its swoosh, as on the game page. */
export const TOPBAR_HTML = '<header class="topbar"><div class="title"><span class="title-script">MVP Baseball 2005</span></div></header>';
