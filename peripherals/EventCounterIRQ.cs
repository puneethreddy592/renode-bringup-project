using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class EventCounterIRQ : IDoubleWordPeripheral, IKnownSize
    {
        public EventCounterIRQ(Machine machine)
        {
            IRQ = new GPIO();
            Reset();
        }

        public uint ReadDoubleWord(long offset)
        {
            switch (offset)
            {
                case 0x0:
                    return ctrl;
                case 0x4:
                    return status;
                case 0x8:
                    return eventCount;
                default:
                    this.Log(LogLevel.Warning, "Unhandled read from offset 0x{0:X}", offset);
                    return 0;
            }
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            switch (offset)
            {
                case 0x0:
                    ctrl = value & 0x1u;
                    break;

                case 0x4:
                    // write-1-to-clear
                    if ((value & 0x1u) != 0)
                    {
                        status &= ~0x1u;
                        IRQ.Unset();
                        this.Log(LogLevel.Info, "Status cleared, IRQ deasserted");
                    }
                    break;

                case 0xC:
                    if (ctrl != 0)
                    {
                        eventCount++;
                        status |= 0x1u;
                        IRQ.Set();
                        this.Log(LogLevel.Info, "Event triggered, count now {0}, IRQ asserted", eventCount);
                    }
                    else
                    {
                        this.Log(LogLevel.Warning, "Trigger written while disabled (CTRL.enable=0)");
                    }
                    break;

                default:
                    this.Log(LogLevel.Warning, "Unhandled write to offset 0x{0:X}, value 0x{1:X}", offset, value);
                    break;
            }
        }

        public void Reset()
        {
            ctrl = 0;
            status = 0;
            eventCount = 0;
            IRQ.Unset();
        }

        public long Size => 0x1000;

        public GPIO IRQ { get; private set; }

        private uint ctrl;
        private uint status;
        private uint eventCount;
    }
}
