# 8812au.ko for StrykerOSS (aarch64 UML, 7.2.0-rc4)

Out-of-tree `rtl8812au` module built against the exact kernel StrykerOSS boots.

## Why this exists

The UML guest in StrykerOSS runs **7.2.0-rc4-g8897487c5223**, built from
[`zalexdev/linux-um-arm64`](https://github.com/zalexdev/linux-um-arm64) at commit
`8897487c52233cd00cf2850008ca068892f1ae91`. That tree has `CONFIG_RTW88_8812AU=y`
built in, and `rtw_8812au_id_table` matches `0bda:8812`, so the dongle binds and
`wlan0` appears — but `rtw88` advertises only

    NL80211_IFTYPE_STATION | NL80211_IFTYPE_AP | NL80211_IFTYPE_ADHOC

with no `NL80211_IFTYPE_MONITOR`, so `iwconfig wlan0 mode monitor` fails. The
aircrack-ng driver does support monitor mode, hence this build.

## Kernel facts that constrain the build

| item | value |
|---|---|
| release | `7.2.0-rc4-g8897487c5223` |
| arch | `ARCH=um SUBARCH=arm64` |
| compiler | `aarch64-linux-gnu-gcc 14.2.0` |
| `CONFIG_MODVERSIONS` | `y` (symbol CRCs must match) |
| `CONFIG_MODULE_FORCE_LOAD` | not set (`insmod -f` unavailable) |
| `CONFIG_MODULE_SIG` | not set (unsigned modules load) |
| `CONFIG_TRIM_UNUSED_KSYMS` | not set (no symbol whitelist) |
| `CONFIG_CFG80211` / `MAC80211` | `y` (built in) |
| `CONFIG_USBIP_VHCI_HCD` | `y` (USB over USB/IP from the Android side) |

Because `MODULE_FORCE_LOAD` is off, both the vermagic string and every symbol
CRC have to match the running kernel exactly. That is why the workflow compiles
the kernel itself first: `Module.symvers` is the only source of correct CRCs.

## Build

`Actions → build-8812au-uml → Run workflow`, defaults are already correct.

The job:

1. clones `zalexdev/linux-um-arm64`, checks out `8897487c5223`
2. drops in `uml.config` (extracted from the device's own `Image`)
3. pins `CONFIG_LOCALVERSION="-g8897487c5223"` so `UTS_RELEASE` matches
4. builds the kernel → `Module.symvers`
5. builds `aircrack-ng/rtl8812au` against that tree
6. uploads `8812au-uml-aarch64.tar.gz` + `build.log`

## Install inside the guest

`rtw88_8812au` is built in, so it grabs `0bda:8812` the moment vhci presents it.
Unbind it before inserting the out-of-tree module:

```bash
# find the vhci-attached device
for d in /sys/bus/usb/devices/*/; do
  [ "$(cat $d/idVendor 2>/dev/null)" = "0bda" ] && echo "$d $(cat $d/idProduct)"
done

# release it from rtw88 (built in, cannot be rmmod'ed)
echo -n "1-1:1.0" > /sys/bus/usb/drivers/rtw88_8812au/unbind

insmod /root/8812au.ko
dmesg | tail -20
ip link | grep wlan
```

Monitor mode:

```bash
ip link set wlan0 down
iwconfig wlan0 mode monitor
ip link set wlan0 up
iwconfig
```

## If the build fails

The driver's newest version guard is `KERNEL_VERSION(6, 9, 0)`; 7.2 moved several
APIs. `build.log` is uploaded even when the job fails — the first error line is
what matters. Patches go in `patches/` and are applied with `git apply` before
the module step.
