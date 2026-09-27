# Renode Custom Peripheral Bring-up (Zephyr RTOS)

Simulated SoC bring-up project: modeling a custom memory-mapped peripheral in Renode,
writing its device tree binding, and implementing a Zephyr RTOS driver from scratch —
covering boot, interrupt handling, and fault recovery, without physical hardware.

## Status
- [x] Renode installed and verified (v1.17.0)
- [ ] Base platform boots Zephyr RTOS
- [ ] Custom peripheral modeled in Renode
- [ ] Zephyr driver + device tree binding written
- [ ] Interrupt handling verified end-to-end
- [ ] Watchdog / low-power integration
