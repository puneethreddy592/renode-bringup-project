# Build Notes & Issues Encountered

A running log of real problems hit while building this project, and how they were fixed —
kept because most of these are non-obvious and will resurface on a fresh machine or a
newer Ubuntu release.

## 1. `dotnet-sdk-8.0` not available via apt / Microsoft's repo
Ubuntu 26.04 is new enough that Microsoft's package repo (`packages.microsoft.com`) has no
published packages for it yet, and Ubuntu's own repo only ships `dotnet-sdk-10.0`.
**Fix:** installed `dotnet-sdk-10.0` directly — Renode only requires dotnet >= 6.0, so a
newer SDK works fine despite version-pinned instructions elsewhere online.

## 2. Renamed/missing packages on Ubuntu 26.04
Older setup guides reference package names that no longer exist:
- `policykit-1` → replaced by `polkitd` + `pkexec`
- `libgtk2.0-0` → `libgtk2.0-0t64`
- `gtk-sharp2` → not needed at all for modern dotnet-based Renode builds

## 3. `renode-latest.deb` 404s from GitHub releases directly
`https://github.com/renode/renode/releases/latest/download/renode-latest.deb` 404s because
GitHub release assets are named with the version number (e.g. `renode_1.17.0_amd64.deb`),
not `renode-latest.deb`. That generic filename only exists on Antmicro's own build server.
**Fix:** download from `https://builds.renode.io/renode-latest.deb` instead.

## 4. Renode's `.deb` depends on `dotnet-runtime-8.0`, which doesn't exist on 26.04
Only `dotnet-runtime-10.0` is available, but Renode runs fine on it — the version pin in
the package metadata just hasn't caught up.
**Fix:** `sudo dpkg -i --ignore-depends=dotnet-runtime-8.0 renode-latest.deb`

## 5. ⚠️ Renode gets silently removed by later, unrelated `apt` commands
This bit us twice. Running `sudo apt install <anything>` or `sudo apt --fix-broken install`
after Renode is installed can cause apt to silently uninstall Renode again, because it
re-evaluates the same unsatisfiable `dotnet-runtime-8.0` dependency and "resolves" the
conflict by removing the package — with no obvious warning.
**Symptom:** `renode` command suddenly not found; `dpkg -l | grep renode` shows status `rc`
(removed) instead of `ii` (installed).
**Fix:** re-run step 4's `dpkg -i --ignore-depends=...` command. **Always re-check
`dpkg -l | grep renode` after any unrelated `apt` operation.**
**Better long-term fix (not yet applied in this repo):** create a dummy/equivs package that
satisfies the `dotnet-runtime-8.0` dependency permanently, so apt stops trying to "fix" it.

## 6. `west sdk install` / other west extension commands "unknown command" outside the workspace
`west`'s extension commands (like `sdk`) are only registered when run from inside a Zephyr
workspace (where `west-commands.yml` gets picked up). Running `west sdk install` from `~`
instead of `~/zephyrproject/zephyr` fails with `unknown command "sdk"`.
**Fix:** always `cd` into the Zephyr workspace first.

## 7. Not every Zephyr board supports `west build -t run` (Renode)
Real hardware board targets (e.g. `stm32f4_disco`) fail with "Emulation/Simulation not
supported with this board" — only boards with `SUPPORTED_EMU_PLATFORMS renode` set in their
`board.cmake` support this.
**Fix:** `grep -rl "SUPPORTED_EMU_PLATFORMS renode" boards/` to find valid targets. We used
`stm32h7_renode_reference_board` (an Antmicro-maintained reference platform built specifically
for Renode, with an editable `.repl`).

## 8. `RENODE-NOTFOUND` even after Renode is installed
CMake's `find_program(renode)` result gets cached in `CMakeCache.txt` at configure time. If
Renode wasn't on PATH during the *first* configure (e.g. it had been silently removed per
issue #5), later reinstalling it doesn't retroactively fix the stale cache.
**Fix:** `rm -rf build` and reconfigure from scratch after fixing the underlying PATH/install
issue.

## 9. `Python.PythonPeripheral`'s `filename:` field needs an absolute path
A relative path like `"peripherals/event_counter.py"` in a `.repl` file is *not* resolved
relative to your shell's current working directory — it fails with "Could not find source
file for the script." **Fix:** use an absolute path.

## 10. Guessing Renode monitor command names wastes time — check help/usage first
Several command names had to be corrected by trial and error:
- `sysbus GetAllRegisteredPeripherals` doesn't exist → correct command is just `peripherals`
- `nvic IsIrqPending <n>` doesn't exist → instead, query your own peripheral's GPIO pin
  directly by name, e.g. `eventCounterIrq IRQ` (prints `GPIO: set`/`GPIO: unset`)
- `west sdk install -v` fails ("unexpected arguments") → check `west sdk install --help`
  first; global flags like `-v` belong *before* the subcommand: `west -v sdk install`

## 11. `zephyr_library()` silently fails in **application-mode** CMake
Calling `zephyr_library()` / `add_subdirectory()` for an out-of-tree driver from a plain
application `CMakeLists.txt` prints a CMake warning ("will not be treated as a Zephyr
library") and the resulting object never gets linked into the final binary — causing
`undefined reference` errors at link time for every symbol in the driver, even though the
build otherwise "succeeds."
**Fix (simple, used here):** add the driver's `.c` file directly via
`target_sources(app PRIVATE drivers/event_counter/event_counter.c)` instead of a separate
library. (The "correct" long-term fix is to structure the driver as a proper Zephyr module
with a `zephyr/module.yml` manifest — not done here to keep things simple.)

## 12. Manual Renode monitor sessions don't survive a watchdog/CPU reset cleanly
Loading a platform and ELF by hand at the monitor prompt (`mach create` →
`LoadPlatformDescription` → `LoadELF` → `start`) never registers a **reset macro**. When the
watchdog fires and resets the CPU, Renode has no defined behavior for reloading the ELF or
resetting the vector table/stack pointer — it just sets `PC = 0x0, SP = 0x0`, which then
crashes into garbage memory-mapped writes.
**Fix:** write a `.resc` script (following the pattern from the board's own official
`stm32h7_renode_reference_board.resc`) with a `macro reset` block that reloads the ELF,
calls `cpu EnableZephyrMode "CONFIG_TICKLESS_KERNEL"`, and sets
`cpu VectorTableOffset` from the ELF's own `_vector_table` symbol. This gives a clean,
repeatable reboot every time the watchdog fires — see `platform/custom_board_irq.resc`.
