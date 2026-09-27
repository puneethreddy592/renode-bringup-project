# Simple memory-mapped "event counter" peripheral for Renode.
# Registers:
#   0x00 CTRL        - bit0: enable
#   0x04 STATUS      - bit0: event flag (write 1 to clear)
#   0x08 EVENT_COUNT - read-only, increments on each TRIGGER write
#   0x0C TRIGGER     - write-only, any write triggers an event

if request.IsInit:
    ctrl = 0
    status = 0
    event_count = 0

elif request.IsRead:
    if request.Offset == 0x00:
        request.Value = ctrl
    elif request.Offset == 0x04:
        request.Value = status
    elif request.Offset == 0x08:
        request.Value = event_count
    else:
        self.WarningLog("Unhandled read from offset 0x%x" % request.Offset)
        request.Value = 0

elif request.IsWrite:
    if request.Offset == 0x00:
        ctrl = request.Value & 0x1
    elif request.Offset == 0x04:
        # write-1-to-clear
        if request.Value & 0x1:
            status = status & ~0x1
    elif request.Offset == 0x0C:
        if ctrl & 0x1:
            event_count = event_count + 1
            status = status | 0x1
            self.NoisyLog("Event triggered, count now %d" % event_count)
        else:
            self.WarningLog("Trigger written while peripheral disabled (CTRL.enable=0)")
    else:
        self.WarningLog("Unhandled write to offset 0x%x, value 0x%x" % (request.Offset, request.Value))
