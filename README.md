# Renode Custom Peripheral Bring-up (Zephyr RTOS)

Simulated SoC bring-up project: modeling a custom memory-mapped peripheral in Renode,
writing its device tree binding, and implementing a Zephyr RTOS driver from scratch —
covering boot, interrupt handling, and fault recovery, without physical hardware.

## Environment
- Renode v1.17.0
- Zephyr v4.4.0 (west-managed workspace)
- Target platform: `stm32h7_renode_reference_board` (Antmicro reference board, Cortex-M7)

## Status
- [x] Renode installed and verified (v1.17.0)
- [x] Zephyr `hello_world` sample built and booted inside Renode
- [x] Custom peripheral modeled in Renode (.repl) — register-verified in isolation
- [x] Zephyr driver + device tree binding written — verified end-to-end boot log
- [ ] Interrupt handling verified end-to-end
- [ ] Watchdog / low-power integration

## Setup (for reproducing)
See `docs/setup.md`
