# Environment Setup

This documents how to reproduce the dev environment for this project from scratch on Ubuntu.

## 1. Install Renode

```bash
sudo apt update
sudo apt install -y policykit-1 libgtk2.0-0 screen libc6-dev libicu-dev gcc python3 python3-pip libgdiplus
# note: on newer Ubuntu releases, use: polkitd pkexec libgtk2.0-0t64 in place of the above where needed

cd ~/Downloads
wget https://builds.renode.io/renode-latest.deb
sudo dpkg -i renode-latest.deb
sudo apt --fix-broken install -y
# if it fails on a dotnet-runtime-8.0 dependency mismatch (e.g. only dotnet-runtime-10.0
# is available in your distro's repos), force it through since Renode works fine on
# newer runtimes despite the pinned dependency name:
sudo dpkg -i --ignore-depends=dotnet-runtime-8.0 renode-latest.deb

renode --version   # should print something like: Renode v1.17.0
```

**Gotcha:** running unrelated `apt install` commands afterward can silently remove Renode
again if apt resolves a dependency conflict in the background. Always re-check
`dpkg -l | grep renode` (expect status `ii`) after any further `apt` operations.

## 2. Install dotnet (required by Renode)

```bash
sudo apt install -y dotnet-sdk-10.0   # or whatever version your distro's repo carries
dotnet --version
```

## 3. Install Zephyr's build dependencies

```bash
sudo apt install -y --no-install-recommends git cmake ninja-build gperf \
  ccache dfu-util device-tree-compiler wget \
  python3-dev python3-venv python3-tk xz-utils file make gcc \
  gcc-multilib g++-multilib libsdl2-dev libmagic1
```

## 4. Set up the Zephyr workspace

```bash
mkdir -p ~/zephyrproject
python3 -m venv ~/zephyrproject/.venv
source ~/zephyrproject/.venv/bin/activate
pip install west

cd ~/zephyrproject
west init .
west update
west zephyr-export
pip install -r ~/zephyrproject/zephyr/scripts/requirements.txt
```

## 5. Install the Zephyr SDK

```bash
cd ~/zephyrproject/zephyr
west sdk install -t arm-zephyr-eabi
```

## 6. Build and boot hello_world on our target board

```bash
cd ~/zephyrproject/zephyr
west build -p always -b stm32h7_renode_reference_board samples/hello_world
west build -t run
```

Expected output includes:
*** Booting Zephyr OS build ... ***
Hello World! stm32h7_renode_reference_board/stm32h753xx


## Notes
- Every new terminal session needs: `source ~/zephyrproject/.venv/bin/activate`
- Target board: `stm32h7_renode_reference_board` (Antmicro's Renode reference platform, STM32H7/Cortex-M7)
- Renode `run` target runs headless by default (no GUI window) when invoked via `west build -t run`
