# Features and Backlog

What this fork (`tonalOrchestrator/project-generalsx`) adds on top of its base, and what is next.

**Lineage:** TheSuperHackers/GeneralsGameCode (upstream engine fixes) -> fbraz3/GeneralsX (Linux/macOS port: SDL3 +
DXVK + OpenAL) -> this fork. Everything below is *added here*; the port itself is not repeated.

Status key: **done** (built and verified), **wip** (in the tree, partly working), **idea** (not started).

---

## Added in this fork

### AI-vs-AI skirmish runner - done (2026-09-30)
`-aiskirmish <map>` runs a skirmish where every slot is an AI, headless (faster than real time, seed-deterministic) or
watchable. Per-player end-of-game stats as JSON, per-AI decision log (`-aiskirmishLog`).
Docs: `PLAN-023_AI_SKIRMISH_HARNESS.md`. Code: `Core/GameEngine/Source/Common/AISkirmishSimulation.cpp`.

### Windows MSVC build on a remote box - done (2026-10-20)
`scripts/remote-build.sh` snapshots the working tree (including new untracked source files), builds it with VS 2022 on the
Windows machine (ssh alias `vadapuppy`) and returns the log and artifacts. Needs
`-DSAGE_USE_DX8=ON -DRTS_BUILD_OPTION_SAGE_PATCH=OFF` (SAGE_USE_DX8 is undefined by default and silently selects the Linux DXVK
headers). MinGW compiles and links but cannot produce a runnable exe (needs the debug-only `d3dx8d.dll`; the SDK's static
`d3dx8.lib` is MSVC-only), so MSVC is the Windows toolchain. MinGW portability fixes are kept.

### LAN netplay harness (Linux host + Windows joiner) - done (2026-10-20)
`-lanhost <map>` / `-lanjoin <ip>`: a scripted LAN game (create, join, accept, start) with AI opponents, no menus.
`tools/netplay/run-netplay.sh` runs both machines (headless or windowed, `--watch` / `--linux-watch`), collects logs,
per-minute state CRCs, per-player stats and in-engine screenshots, and `compare.py` verdicts the run.
Verified: Linux and Windows stay in sync for headless+headless and windowed+windowed matches.
Docs: `tools/netplay/README.md`. Code: `Core/GameEngine/Source/Common/LanNetplayHarness.cpp`.

### Autopilot (build-only AI for a human player) - done (v1)
Hotkey Ctrl+Shift+A toggles, Ctrl+Shift+L cycles the level (Easy / Normal / Hard = the skirmish AI skill states). Goes through the
network command stream (`MSG_SET_AUTOPILOT`), so every machine flips at the same logic frame; verified in sync across
Linux/Windows with on, off, on-at-Hard. Attaches a skirmish AI to the player with army/attack logic disabled, adopts the
structures the player already owns, and shows "Autopilot: ON (Hard)" on screen.
Verified Linux+Windows in sync with on / off / on-at-Hard (6 game-minutes, two humans on autopilot at once): the autopilot bases
grew from 1 to 7-10 buildings and an income started (supply workers/trucks), and growth paused while it was off.
How it builds: a computer player gets its build order from the skirmish *scripts* (most build-list entries are marked
`AutomaticallyBuild = No`), a human has none, so `AISkirmishPlayer::autopilotQueueNextStructure` plays that role: cheapest missing
entry first, per-level cap on duplicates (Easy 1, Normal 2, Hard 3), power plants only when short of power, and missing
prerequisite buildings (the supply center/stash is not in the list; it can itself need a power plant) are queued recursively.

### Autopilot Wallet Mode (level 4) - done, solo-verified
Ctrl+Shift+L now cycles Easy / Normal / Hard / Wallet. Wallet = "defend myself, survive, hoard money": phase 1 builds the
foundation (supply center, power, and the prerequisites on the way to the faction's advanced tech building: strategy center /
propaganda center / palace), phase 2 only builds economy, power and up to 4 base defenses while $1,500 stays in the bank (defenses
that need a prerequisite building only when the bank is also $3,000 over their cost). Solo: ~$21k earned in 8 game-minutes with
spending stopped at the foundation cost.

### Wallet Mode under attack - done (measured)
When enemy units are near the base (`isLocationSafe` on the base center), Wallet Mode ends its build-up, drops the reserve to $300, allows
up to 12 defenses (4 per type) and trains up to 16 defenders (strongest affordable combat unit, at most a quarter of the bank, at a
barracks / war factory); idle units defend themselves, nothing ever attacks. Measured solo, headless, 14 game-minutes, one enemy AI:
- vs **Medium**: Normal autopilot survives 0/12 (defeated 7.8-9.6 min); Wallet survives 8/12 (the 4 losses at 8.1, 8.6, 11.7, 12.8 min).
- vs **Hard**: nothing survives; Normal lasts ~10.5 min on average, Wallet ~11.8 min (was ~10.7 before the under-attack spending).
  Seen before the fix: a player died with $20k+ banked because its dozers were dead and it had no army.
`autopilot_scenarios.py --enemy E|M|H` runs this comparison (`lost=` column).
Round two (army cap, queue depth and the defense total all scale with the bank; several defenders per factory per cycle): vs Medium still
8/12 survive (the four losses now at 8.6, 10.8, 12.8, 13.5 min); vs Hard still nothing survives (9.7-12.6 min, ~11.4 avg), even with 172
defenders queued across 12 runs. The trace (`autopilot defenders none queued: ...`) shows why more spending does not help: under threat
the bank is only $600-1,100, because the extra defense structures (~$800 each) take the money before the defender trainer runs, and
the player has one production building. Beating a Hard AI needs more than reactive spending (tech, more factories, or a human), so
that is not being chased further.
Cross-machine: a 12-minute Linux + Windows match vs a Medium AI with both humans in Wallet Mode (real combat, 18 defenders queued on
each machine) stayed in sync on all 12 per-minute CRCs with identical final stats.

### Explore mode (autopilot level 5) - done (solo-verified, not yet across machines)
Like Normal, plus explorers that capture neutral buildings (oil derricks and the other tech buildings) for income. It buys the faction's
"Capture Building" upgrade at the barracks (every faction needs it; without it the capture special is "not ready"), trains up to two
capture-capable infantry (Ranger, Red Guard, Rebel), and sends each idle explorer to the nearest safe neutral building owned by the game's
neutral player. A building still in the shroud is first *scouted*: the explorer walks to it, which lifts the shroud, and the next cycle
orders the capture (the game's own capture special, via an AI group, same path as a player's click). A building an explorer fails to take
or reveal is skipped for five minutes. Nothing attacks an enemy. Measured on Death Valley, 12 game-minutes: 8-14 buildings captured
and ~$95-120k earned for 11 of 12 factions (Normal earns ~$25-31k there). On the standard Dark Night map there is nothing to capture,
so level 5 behaves like Normal. Hotkey cycle is now easy / normal / hard / wallet / explore.
Diagnosed with traces: first "not neutral" (the relationship toward an unowned building is not NEUTRAL: test the owner instead), then
"special not ready" (the upgrade), then "in the shroud" (scouting). Built for Challenge mode, which has many neutral buildings; not yet
tried on a Challenge map when this was written; see "Challenge maps from the harness" below.

### Challenge maps from the harness - done (2026-10-21)
`-file Maps\\<Name>.map -soloAutopilot "5:5" -soloMinutes 12 [-soloTemplate FactionChinaInfantryGeneral]` runs a single-player map headless with the
human on Autopilot and prints the stats (docs: `tools/netplay/README.md`). `-file` itself was dead in release builds (inside the `RTS_DEBUG` block of
the command-line table). A Challenge map additionally needs the state the Challenge menu sets up (campaign, `TheChallengeGameInfo`, human slot),
otherwise the map's `ThePlayer` owns nothing and its lose script fires on frame 0; `setupChallengeForMap` does that.
Explore + garrison squad as China Infantry General, 12 game-minutes, Normal difficulty (the map can end the game earlier):
| map | ended | capture / garrison orders | earned | tech captured |
|---|---|---|---|---|
| Air General | map, 11:07 | 5 / 3 | $47.7k | 6 |
| Chem General | map, 11:05 | 0 / 10 | $13.5k | 1 |
| Demolition General | cap | 4 / 7 | $53.4k | 4 |
| Laser General | cap | 2 / 7 | $38.3k | 5 |
| Super Weapons General | map, 6:37 | 2 / 1 | $11.8k | 2 |
| Tank General | map, 11:37 | 4 / 17 | $15.6k | 3 |
| Nuke General | map, 11:08 | n/a | $34.5k | 4 |
| Stealth | cap | n/a | $35.2k | 3 |
| China Boss | map, 10:48 | n/a | $17.3k | 1 |
(Nuke, Stealth and China Boss first ended in seconds because they are not the first mission of any general's campaign, so the Challenge setup was skipped; it now searches every mission.)
Findings: (1) Challenge maps give their capturable buildings to the *civilian* player (relationship neutral), not the game's neutral player, so Explore
captured nothing on most maps; it now targets those too (Laser $15k -> $24-38k, Demolition 0 -> 4-5 captures). CORRECTION: the first note here said Death Valley stayed identical, but that run used a stale installed binary (`autopilot_scenarios.py` runs the copy in `~/GeneralsX/GeneralsZH`, so copy the build there first). Measured properly, targeting civilian tech buildings cost Death Valley Explore income 11% (1.232M -> 1.107M summed over 12 factions) because the two explorers chased platforms and pads and re-scouted ones they could never reveal. Fixed in part: three scouting tries per building, and income buildings (oil derricks and refineries) are preferred (a non-income building must be a quarter as far away): 1.168M, 5% below the old baseline, accepted for the Challenge gain. (2) Most capturable
buildings belong to the opposing general (22 of 31 on Chem), which Explore never touches, so income stays modest. (3) The squad does garrison
(houses, a convenience store, the Palace); on every map some 10 candidate buildings "refuse": the trace now says why, and on Tank General 8 of 10 are still in the shroud (none enemy-owned or full). The squad now scouts them first (walks to the nearest shrouded one, then enters next cycle): Tank General $15.6k -> $20.5k earned, 13 scouting walks; Wallet vs Medium survival stays 12/12; Linux + Windows Wallet match in sync.
(4) The human, who only builds and defends, is wiped out by the Challenge AI on most maps after 6-11 game-minutes; that is the Wallet-vs-Hard story again.

Wallet (level 4) vs Explore (level 5) on the same nine Challenge maps (China Infantry General, 15-minute cap, seconds survived; 900 = survived):
Air 900 / 665, Chem 580 / 755, China Boss 630 / 645, Demolition 900 / 900, Laser 490 / 900, Nuke 795 / 775, Stealth 900 / 840, Super Weapons 350 / 410,
Tank 605 / 685. Mean survival 683 s (Wallet) vs 730 s (Explore); Explore earns more on 7 of 9 maps. Neither alone holds a Challenge general's base for
long (Fortress, below, does): Super Weapons falls in under 7 minutes in both modes: at the end the player has $3,400, no dozer and no factory that could train one (`AIPlayer::findDozer` already queues a dozer when it has a factory), so it is a destroyed base, not a missing replacement rule. A Challenge-tuned defence (or Explore's income feeding Wallet's defenders) is the open item.

### Module toggles and the Defense Units module - done (2026-10-21), module not yet a win
A level is now just a starting set of modules (`AM_WALLET`, `AM_DEFENSE_UNITS`, `AM_EXPLORE`, `AM_SQUAD`, `AM_HACKERS`, `AM_REPAIR`; `AutopilotProfile::modules()` /
`withModules()`). `MSG_SET_AUTOPILOT` takes an optional second argument, the exact module set to run, so every machine applies a change on the same frame; the
player remembers it (`Player::getAutopilotModules()`, saved in the high bits of the saved level so older saves still load). Ctrl+Shift+D flips Defense Units on
the current level; any other combination goes through the message (harness: `-lanAutopilotSchedule "sec:level:mask"`, `-soloAutopilot` likewise; mask 59 = Wallet +
Defense Units + Squad + Hackers + Repair). The on-screen message lists the modules when a custom set is applied. Verified: default levels give identical numbers
to before; a Linux + Windows match switching the set on, off and back stayed in sync on all 8 checksums.
The Defense Units module (`AutopilotDefense::trainStandingArmy`) keeps a standing force, rotating tank (war factory), tank hunter, rifleman, sized by the bank
(6 units, +1 per $2,000), never dipping into the $1,200 defense fund, and makes the planner build the barracks and war factory it needs. Measured against a Hard AI
(16 game-minutes, 12 factions, Wallet vs Wallet + Defense Units): median 13.0 vs 13.0 minutes, survivors 2 vs 1, kills 17.8 vs 14.8 - **no gain**. Spending down to a
$300 reserve with a bigger target was worse (median 11.0, no survivors): units and defenses draw on the same bank and the defenses are what hold. So the module is
off by default; it needs a different idea (rally and grouping at the defenses, or income to spare) before it earns a place in a level.

### Fortress (level 6 = Wallet + Explore) - done (2026-10-21), the first real answer to a Hard AI
Found by layering with the new module mask (`-lanAutopilotSchedule 5:4:61`): captured income feeds the defense spending. Now a level of its own (hotkey cycle:
easy / normal / hard / wallet / explore / fortress). Measured solo, 12 factions, 16 game-minutes vs one Hard AI on Death Valley: Wallet 5/12 survive (median 15.0 min,
$29.8k earned), **Fortress 10/12 survive (median 16.0 min, $86.8k earned)**. Against Medium on Dark Night (nothing to capture there): 12/12 survive, same as Wallet.
Challenge maps (China Infantry General, 15-minute cap): Fortress reaches the cap on 7 of 9 maps (Air, Chem, Demolition, Laser, Nuke, Stealth, Tank); China Boss
falls at 8:25 and Super Weapons General at 5:50. Wallet alone averaged 11.4 minutes and Explore alone 12.2 on the same maps. Linux + Windows match vs Hard,
10 minutes: in sync. Lesson: the earlier "more spending does not help" finding was about a bank with no income; the income, not the army, was missing.

### Results recovered after the 2026-10-21 shutdown (from the interrupted Challenge-map session)
Two experiments finished just before the computer went to sleep and were never written down; their raw logs survived in `logs/autopilot-scenarios/`:
- **Fortress + Defense Units vs a Hard AI**, Death Valley, 16-minute cap, 12 factions (run 20261021-202256): **10/12 survive**, the same as Fortress alone,
  so the Defense Units module adds nothing on top of Fortress there. The two that fell: plain China (14.0 min) and China Tank General (13.5 min).
- **Wallet on Golden Oasis** (run 20261021-204242, 11 of 12 runs: the shutdown cut the last one): 1/11 survive the cap, median 13.9 min. A baseline for
  comparing Fortress on a second map; the Fortress run for that comparison never happened.
- **Golden Oasis, Wallet vs Fortress, complete 12-faction runs vs a Hard AI, 16-minute cap (2026-10-22, runs 20261022-105618 / 20261022-110045):**
  Wallet 1/12 survive (GLA Stealth General only), median 14.2 min (10.2-16.0), $332k earned in total, 35 tech buildings captured;
  **Fortress 0/12 survive, median 13.6 min (11.3-15.8), $324k earned, 37 captured**. Unlike Death Valley (10/12), Explore adds no income on Golden Oasis
  (the capturable buildings are tents that give no cash), so Fortress is equal to Wallet there, slightly worse within noise. Failed placements are
  high on GLA and China (up to 80, GLA Demolition Fortress): a cheap thing to look at next, since lost placement attempts waste the defense fund.
- **Super Weapons General under `-soloSnapshot 30` (Fortress, China Infantry General, 2026-10-22):** the run ends at exactly 10500 frames (350 s) with the player
  *not* defeated (`endFrame` 0): the map's own mission script ends it once the command center is gone (its HP falls steadily from 174 s, 30% -> 4%, while the
  repair module keeps trying; the last structure is a Gattling Cannon at 330 s). Attackers seen in the snapshots: a Particle Uplink Cannon firing from 30 s
  (`ParticleUplinkCannonTrailRemnant`, a `RadiationFieldMedium` object over the base), 2 Avenger tanks with laser turrets, 3 Comanches, 4 Rangers, 4 Missile
  Defenders, sentry/spy drones, an Aurora bomber at 270 s. So it is a scripted superweapon + air + armor rush at a base of a command center and a dozer, not a
  planner gap. The idle bank is not a missing rule either: Gattling Cannon / Bunker wait on the supply center (itself waiting on the power plant), which
  unlocks them at frame 1500 (50 s) - an experiment that queued the missing prerequisite of the cheapest blocked defense never fired and was reverted.
  Turning on every module (mask 63: Defense Units, squad, hackers, repair) also ends at 350 s with 14 kills. Conclusion: not worth tuning Fortress for this map;
  China Boss, same method (Fortress, 15-minute cap): falls at 8:25 (505 s; again `endFrame` 0, the map ends it when the base is gone). The boss keeps a standing
  army in view from the first minute (about 40 Tank Hunters, 21 Rangers, 21 Hackers, 12 Dragon tanks early, then 20 / 11 / 4 tunnel defenders / 3 Overlords) and
  rocket buggies late; the player's economy stays under $1.7k (bank $550-1,700 from 2:00 on), spending on a barracks, war factory, Propaganda Center, four Speaker
  Towers and one or two Bunkers. Not a planner gap: an AI with a hundred-unit army against a 1v1 Wallet economy. Both maps say the same thing - Fortress is
  built for Hard-AI skirmish pressure and holds 7 of 9 Challenge maps; these two are scripted overkill. Neither is worth more tuning.
Open question that session was on: why Fortress still ends early on the **Super Weapons General** (about 5:50) and **China Boss** Challenge maps (see
the Challenge section above and `docs/DEV_BLOG`); it was using `-soloSnapshot` to see what attacks. Resume point: re-run the Golden Oasis comparison, then
look at the Super Weapons General run with the snapshot.

### Superweapons module (AM_SUPERWEAPONS, on in Wallet and Fortress) - done (2026-10-22), a win
Philosophy (Ben): defend, hoard cash by generating revenue, hoard superweapons if not limited; the only offense is firing them. `AutopilotSuperweapons` fires each
ready area power (Particle Uplink Cannon, Nuke / Neutron Missile, Scud Storm, strikes) at the spot where the enemy structures the player has SEEN (shroud respected,
fogged counts) are worth most, never with own/allied structures in range, only above a minimum value, never a general's power whose science is not bought.
The planner side: after 4 defenses stand it SAVES for the cheapest superweapon (plus its prerequisite building): no further defenses, units leave the price in
the bank once 8 exist, with a 300 s give-up and 120 s cooldown; then it adds its own structure entry (most build lists have none), one at a time and only when
the last is finished and powered. Findings that mattered: (1) a superweapon is disabled while the base is short of power: it adds extra power plants
(`queueExtraPowerPlant`) and keeps 25% headroom while one stands (USA Particle Cannons sat disabled for a whole game before); (2) a 16-minute game ends before the
first Scud Storm / Neutron Missile has charged (about 10 minutes) - firings need 25-30 minute games, which only Fortress survives vs Medium; (3) headless runs
crashed in `RadiusDecalTemplate::createRadiusDecal` (null shadow manager), now guarded.
Measured: Fortress vs Hard, Death Valley, 16 min, 12 factions: **module on 12/12 survive, 273 kills, $1.18M earned; module off (mask 61) 10/12, 197 kills,
$1.04M**. 30-minute Fortress vs Medium: GLA 9-12 Scud launches, China Nuke General 3 Neutron Missiles, USA Super Weapon General 12 Particle Cannon shots.
Standard matrix (levels 1-6, 8 min): levels 1, 2, 3, 5 identical to the pre-change baseline; only Wallet (11 rows) and Fortress (10) differ.
Open: USA Laser General finds no buildable superweapon structure (no 'saving' line at all); Wallet alone (no captured income) falls to a Medium AI within 30 min
so it rarely reaches a firing; firing at the same spot twice in a row on China (re-targets before the first blast registers); a general's power bought via
sciences (A-10, artillery) is supported but nothing buys the science yet (a separate decision); Windows build not yet rebuilt/synced with this.

### Player-supplied music (Music/<playlist>/, shuffle + crossfade) - done (2026-10-22), stages 1-4 of `FACTION_MUSIC_DESIGN.md`
`MusicPlaylistPlayer` (Core, client-only) replaces the retail `Game_*` tracks with the local player's playlist folder; see the design doc's "How to use it". Open: `playlists.ini` /
`.m3u` options, the debug now-playing line, options-menu toggle, ducking over `Game_Silent`; checking it by ear and on Windows / MiniAudio; the pause-menu clock stop; a Suno
export step. Side fixes: OpenAL kept no music volume when a music stream's volume was adjusted (now does); `FFmpegFile::getDurationMs` falls back to file size / bitrate because
the custom I/O cannot seek.

### Defending against a Hard AI (Wallet mode) - improved, not solved
Goal from Ben: a 1v1 setup that can actually defend against a Hard opponent. Measured solo, headless, 16 game-minute cap, all 12 factions vs one
Hard AI (median survival; none survived 16 minutes until the extra defenses):
- before today's defence work: 11.4 min (9.8-12.1), ~8 kills / 60 losses per game.
- sticky threat flag (90 s) + bigger caps: 11.3 min - **no change**: the faction build lists hold only 1-2 defenses, so the caps never applied.
- **extra defenses beyond the build list** (`AutopilotPlanner::queueExtraDefense`: rotate through the defenses the dozer can build, ring around
  the base, spend a surplus even before an attack): 12.2 min, one run survived all 16; then a lower gate ($5k) and higher caps: **12.9 min**
  (10.9-14.9), ~14 kills per game. The USA still builds only 6-10 (one defense type, cause not yet found); GLA and China 20-30+.
- **units leave a $1,200 defense fund** (the defender trainer spent the bank down to $300 every two seconds, so the planner could never afford the
  ~$1,000 defenses; under attack both the USA and GLA had a bank of only $250-600): **13.4 min** (11.3-16.0), one run survived the full 16, ~16 kills /
  44 losses per game, the USA now builds 12-20 extra defenses (was 6-10). Lesson: units and structures draw on one bank, so the one that matters has to
  be protected, and a decline trace (`extra defense declined: ... too dear for a bank of N`) found it in one run.
- **dozer repair** (`AutopilotRepair`: an idle dozer is sent to the most damaged building below 70% health, wallet and explore): median 13.1 min
  (11.9-16.0), ~18 kills / 45 losses - **no measurable change in the median** (noise is about +-0.3), a slightly better worst run. Kept because the
  healing is free and the Medium / matrix / Explore numbers did not move; it is not what decides a Hard game.
- Against a **Medium** AI: Wallet survivors **12/12** (plain Autopilot 0/12; Wallet was 8/12 this morning).
Findings that mattered: units die piecemeal (60 losses / 8 kills), the bank sat unspent ($14k at death) and the only defense was the one the build
list names. Not yet tried: repair of damaged structures, keeping defenders grouped at the defenses, an anti-vehicle mix tuned to the Hard AI's waves.

### Garrison squad and Hackers (Wallet and Explore modes) - done (solo-verified)
Both requested by Ben. **Garrison squad** (wallet and explore): keeps a standing pair, alternating a "tank person" (Tank Hunter, Missile
Defender, GLA RPG trooper) and a "gun person" (Ranger, Red Guard, Rebel, Minigunner), trained at a barracks while the bank is comfortable and
sent into the nearest building that can hold troops within reach of the base (neutral buildings count; the game decides who may enter);
the explorers are exempt (the first two capture-capable units). **Hackers** (wallet): the planner builds the Internet Center and a barracks,
then China trains up to 6 Hackers and sends them into the Internet Center, where the game data says they hack faster. Measured solo:
- vs a Medium AI, 14 game-minutes: Wallet survivors **8/12 -> 10/12**.
- China Wallet income per game-minute rises from ~$3k (minute 4) to ~$5.8-6.7k by minutes 8-10 with the hackers in; over only 8 minutes
  Wallet earns slightly less ($22.9k vs $24.8k on average) because the barracks, hackers and squad cost money up front.
- Garrison orders happen on maps with enterable buildings within ~1,500 units of the base (Golden Oasis tents, Bitter Winter towers, Death
  Valley watchtowers, Hostile Dawn bunkers); Leipzig Lowlands has none.
Open: the USA squad made no garrison orders on Death Valley in Wallet mode (not yet understood); squad units are not sent to captured
buildings specifically; supply drops (USA) are already covered by the supply logic.

### Autopilot economy and scenario matrix - done
- Supply centers are placed next to the nearest warehouse with cash (fan search 6-20 cells around it, base side first), the
  engine refuses a center too close to supplies and trucks ignore one with no supplies in reach. Trucks are sent only to
  build-list entries flagged as supply buildings, so autopilot flags the ones it adds (2/3/4 gatherers for Easy/Normal/Hard).
- Hard and Wallet add a second supply center at another unserved warehouse once there is money to spare.
- `tools/netplay/autopilot_scenarios.py` runs every (faction, level) combination (12 factions x 4 levels) headless on one
  machine with `-lanFast` and checks alive / supply center / income / base growth / placement failures; the 48-run matrix takes
  about two minutes. `-lanHostTemplate <Faction>` picks the human's faction, `-lanFast` skips the network frame pacing (a 4
  game-minute run takes 5 seconds, results identical to real time).
- Self-diagnosing logs (harness runs): `NETPLAY autopilot place ... placed|waiting for money|FAILED`, a stall explanation with
  dozer / money / legality codes, and a reason whenever the supply expansion declines.

### Cheat mode - done, verified solo (not yet by hand in play)
`-cheats` (needed on every machine) enables Ctrl+Shift+M (+$10,000) and Ctrl+Shift+G (+1 general's point) through the
network command stream. A machine without `-cheats` skips the command, so the match desyncs visibly instead of anyone
cheating unnoticed. `-lanCheatSchedule "sec:money:points,..."` triggers it from a test run: verified that `-cheats` applies the
money and general's points (frame request + 4) and that without the flag the request is ignored.

---

## Findings worth keeping

- **Headless netplay needs `MSG_FRAME_TICK`.** The network layer only leaves PRE-GAME and closes logic frames when it sees
  the client's per-update frame tick. A headless loop that does not send it runs an independent simulation and silently
  drops every command it would send. The harness now sends it.
- **Headless vs windowed desync** (observed before the tick fix; the earlier headless runs were not true lockstep, so
  re-verify): windowed+windowed on Linux and Windows is in sync.
- `AISkirmishPlayer::adjustBuildList` destroys the player's start command center (an AI gets it rebuilt from its list).
  Autopilot adoption must not call it with the default flag.
- **Placement for a human player is stricter than for the AI** in three places (all relaxed for a human with autopilot on):
  `BuildAssistant::buildObjectNow` (units in the way), `DozerAIUpdate::construct` / `WorkerAIUpdate::construct`
  (`SHROUD_REVEALED`, path and overlap checks). The shroud check made every placement on unexplored ground fail silently; the
  first symptom was the supply stash being retried for four minutes.
- The AI only sends supply trucks to build-list entries flagged `isSupplyBuilding` with a desired gatherer count.
- `LBC_TOO_CLOSE_TO_SUPPLIES`: a supply center must not stand close to a supply source.
- Why autopilot built nothing at first: (1) that command-center destruction, (2) human players have no skirmish scripts and
  almost every build-list entry is script-triggered, (3) prerequisite buildings (supply center/stash) are not in the list.
- `macos_2p_vanilla_map.rep` fails at frame 0 with the same CRC on Linux and Windows (a replay/map issue, not platform).

---

## Backlog

### Autopilot
- [x] Prerequisite retries: caused by the human-only placement checks above, fixed.
- [x] Wallet defenses: USA builds a Patriot battery, China speaker towers, GLA tunnel networks (prerequisite defenses once the bank
      is $3,000 over their cost). [x] Defending units and more defenses under attack (see Wallet Mode under attack). [~] Defend against Hard: median 11.4 -> 13.4 game-minutes (see "Defending against a Hard AI"); a Hard AI still wins in the end.
- [x] Second supply center on GLA factions (fan search around the warehouse).
- [ ] Replace lost trucks/workers explicitly and scale it with the level (today the AI's gatherer logic does it).
- [x] Manually built structures count towards the caps (the planner counts what the player owns). A dozer with a build task pending
      is already skipped by `findDozer`, so a dozer the human ordered to build is left alone.
- [x] Cheat mode across two machines with `-cheats` on both: gifts land on the same frame with identical money/points, in sync.
- [ ] One machine without `-cheats` should desync visibly (untested), and the hotkeys by hand.
- [ ] Re-adopt manual buildings on toggle-on (done for entries with the same template; extra structures the player built that are
      not in the list are ignored).
- [ ] Supply trucks / dozers via the assist; decide on upgrades and general's skills (currently left to the player).
- [ ] Levels now differ in duplicate cap, gatherers and second supply center; make them change build pace and tech choice too.
- [ ] `getPlayerDifficulty()` returns the attached AI's difficulty: check nothing gameplay-visible changes for the human.
- [ ] Control-bar toggle button and an options entry instead of hotkeys only.
- [~] Saved games: a save made with Autopilot on used to be unloadable (the load found "AI present" for a human player). `Player::xfer`
      now saves the autopilot level for that case and re-creates the autopilot AI on load; old saves are unchanged. NOT yet tried by
      actually saving and loading a skirmish (the harness has no save/load path): test by hand, then tick this.
- [x] Rapid on/off/level toggling (15 changes per run, all 12 factions) leaves the player alive and building: no crash.

- [x] Explore for China Infantry General: he has no Red Guard (his barracks builds Minigunner, Tank Hunter, Hacker, Black Lotus), so the explorer
      is the Minigunner; fixed, 8 captured and ~$104k earned on Death Valley (was 0 and ~$29k).
- [x] Activity feed: `AutopilotLog::feed` prints "Sending a Mini-Gunner to capture the Oil Derrick" style lines to the console (`AUTOPILOT: ...`) and the in-game message line, for the local player only (capture, scout, garrison, next structure, extra defense).
- [ ] Explore: explorers do not run from enemies; the Challenge-map runs (headless) are done, a hand-played one is not.
- [x] Squad scouts shrouded garrison buildings before ordering entry.

### Netplay / determinism
- [x] Cross-machine check of the placement/economy/cheat changes (Linux + Windows, Autopilot wallet/normal/hard + cheat gifts): in sync.
- [x] Headless netplay now runs true lockstep (frame tick): every Linux + Windows match since 2026-10-20 evening (autopilot modes, cheats,
      hand orders) is in sync. [ ] Re-run the old pre-fix baselines for the record now that the harness sends `MSG_FRAME_TICK` (true lockstep).
- [x] Commands from a human player across machines: `-lanBuildSchedule` sends a real select + construct command; each machine's
      human ordered a barracks by hand (Autopilot off at 100 s, on again at 190 s) in a Linux + Windows match: in sync on all 6
      per-minute CRCs, identical final stats. Note: with Autopilot on, a human's build click is placed with the AI's relaxed rules.
- [~] Windows package for a remote friend: BUILT, NOT SHARED (Ben: "distribution later"). `~/generalsx-friend-package/GeneralsX-Windows-474f8fb017.zip` (3.4 MB):
      GeneralsXZH.exe, mss32.dll / binkw32.dll (this project's stubs), Play-GeneralsX.bat, README.txt, LICENSE.txt, and Microsoft's msvcp140.dll,
      vcruntime140.dll, msvcp140_atomic_wait.dll (the exe imports them; a PC without Visual Studio would not start it otherwise; app-local copies are
      allowed). No game data (checked: no .big/.ini/.w3d/.wav/.bik inside). It is the 2026-10-21 12:56 build (`474f8fb01`), BEFORE Defense Units,
      Fortress and the activity feed: rebuild from `495819555` or newer before sending. Never started on a clean PC; the internet-play steps (Tailscale +
      Direct Connect) are untested. Sharing it (Drive or otherwise) needs Ben's explicit OK.
- [x] Retail compatibility, answer: `RETAIL_COMPATIBLE_CRC` is `(0)` by default (`Core/GameEngine/Include/Common/GameDefines.h`), so
      neither the Linux nor the Windows build is expected to be CRC-compatible with retail Zero Hour 1.04 and a match against the
      original game would desync or be refused. Linux <-> Windows builds of THIS fork agree with each other (every match above).
      The autopilot code only changes anything for a player who has switched Autopilot on, so it would not break CRC compatibility
      for ordinary games if that switch were turned on later (it is a compile definition guarded by `#ifndef`).
      [ ] Try a build with `RETAIL_COMPATIBLE_CRC=1` against a real retail client if retail cross-play is ever wanted.
- [ ] Netplay over the internet (Tailscale) between two homes.

### Tooling
- [x] `run-netplay.sh --fast` (skip the frame pacing on both machines): the game phase of a 4 game-minute Linux + Windows match took 68 s instead
      of 247 s (3.6x), both runs IN SYNC. Two runs with the same seed end with slightly different numbers because the network picks the
      frame a scheduled command (autopilot switch, cheat, hand order) lands on from live timing (frame 304 vs 306): consistent across the two
      machines, not a determinism bug. [ ] ~75 s of fixed setup and teardown per match remain (lobby, map load, ssh, log fetch): trim.
- Windows build is not the bottleneck: it is incremental (~1 min for a sim change) on the 16-thread laptop.
- [x] Linux screenshots are written with a literal backslash in the file name (fixed in `W3DScreenshot.cpp`: `Screenshots/` on POSIX; the driver still matches the legacy name).
- [x] Demo video: `tools/netplay/make_demo.sh` runs a visible solo game (Autopilot on, off, a hand-ordered Barracks via the real
      select + construct commands, on again) and stitches in-engine screenshots into a captioned mp4. [ ] A real screen recording with
      mouse clicks on the Windows laptop (ffmpeg gdigrab + SendInput) is still optional and would take over its desktop.
- [x] `tools/netplay/compare_summaries.py <before> <after>`: diff two scenario runs (exit 0 = identical).
- [ ] Remove the Linux software-Vulkan Xvfb path (`tools/netplay/linux-launch.sh`) or make it work.

### Housekeeping
- [x] Three older fixes (INI parse errors surfaced, recursive file listing fix, locomotor re-specification) committed separately.
- [ ] Push policy: `origin` is a public fork. Decide what goes public (no game data, no credentials, no personal paths).
