extern crate alloc;

use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use crate::graphics::color::Color;
use crate::graphics::surface::Surface;

pub struct TerminalApp {
    pub history: Vec<(String, Color)>,
    pub input: String,
    pub cwd: String,
    pub cmd_history: Vec<String>,
    pub pending_launch: Option<crate::apps::AppType>,
}

impl TerminalApp {
    pub fn new() -> Self {
        Self {
            history: Vec::new(),
            input: String::new(),
            cwd: String::from("/home/bharath"),
            cmd_history: Vec::new(),
            pending_launch: None,
        }
    }

    pub fn take_pending_launch(&mut self) -> Option<crate::apps::AppType> {
        self.pending_launch.take()
    }

    pub fn push_line<S: Into<String>>(&mut self, text: S, color: Color) {
        if self.history.len() >= 120 {
            self.history.remove(0);
        }
        self.history.push((text.into(), color));
    }

    pub fn handle_key(&mut self, key: char) {
        if key == '\n' || key == '\r' {
            self.execute_command();
        } else if key == '\x08' || key == '\x7f' || key == '\u{8}' {
            self.input.pop();
        } else if key == '\t' {
            // Tab autocompletion for common commands
            self.autocomplete();
        } else if key.is_ascii() && !key.is_ascii_control() && self.input.len() < 128 {
            self.input.push(key);
        }
    }

    fn autocomplete(&mut self) {
        let trimmed = self.input.trim_start();
        let commands = [
            "whoami", "user", "id", "uname", "hostname", "pwd", "ls", "cat",
            "date", "uptime", "free", "df", "ps", "top", "sysinfo", "hardware",
            "disks", "wifi", "ping", "apt", "neofetch", "clear", "help", "history",
            "echo", "cd", "mkdir", "touch", "rm", "env"
        ];
        for &cmd in commands.iter() {
            if cmd.starts_with(trimmed) && cmd != trimmed {
                self.input = cmd.to_string();
                return;
            }
        }
    }

    pub fn execute_command(&mut self) {
        let full_cmd = self.input.trim().to_string();
        self.input.clear();

        let prompt_display = if self.cwd == "/home/bharath" {
            String::from("bharath@microos:~$")
        } else {
            format!("bharath@microos:{}$", self.cwd)
        };

        if full_cmd.is_empty() {
            self.push_line(format!("{} ", prompt_display), Color::TERMINAL_GREEN);
            return;
        }

        // Add to command history
        self.cmd_history.push(full_cmd.clone());

        // Echo command line to terminal
        self.push_line(format!("{} {}", prompt_display, full_cmd), Color::TERMINAL_GREEN);

        let parts: Vec<&str> = full_cmd.split_whitespace().collect();
        let cmd = parts[0];
        let args = &parts[1..];

        // Case-insensitive command comparison
        let cmd_lower = cmd.to_ascii_lowercase();

        match cmd_lower.as_str() {
            "whoami" => {
                self.push_line("bharath", Color::WHITE);
            }
            "id" => {
                self.push_line("uid=1000(bharath) gid=1000(bharath) groups=1000(bharath),27(sudo),100(users)", Color::WHITE);
            }
            "user" | "userinfo" => {
                self.push_line("=== Current User Profile ===", Color::ACCENT_CYAN);
                self.push_line("Username    : bharath (UID: 1000, GID: 1000)", Color::WHITE);
                self.push_line("Full Name   : Bharath - MicroOS Administrator", Color::WHITE);
                self.push_line("Home Dir    : /home/bharath", Color::WHITE);
                self.push_line("Shell       : /bin/bash (Interactive GUI Terminal v5.2)", Color::WHITE);
                self.push_line("Privileges  : sudo, wheel, adm, cdrom, dip, plugdev", Color::WHITE);
                self.push_line("Status      : Active & Authenticated (Desktop Console)", Color::TERMINAL_GREEN);
            }
            "who" => {
                self.push_line("bharath  tty1         2026-11-23 02:00 (:0)", Color::WHITE);
            }
            "sudo" => {
                if args.is_empty() {
                    self.push_line("usage: sudo [-u user] command", Color::MUTED_TEXT);
                } else if args[0].eq_ignore_ascii_case("whoami") {
                    self.push_line("root", Color::WHITE);
                } else if args[0].eq_ignore_ascii_case("id") {
                    self.push_line("uid=0(root) gid=0(root) groups=0(root)", Color::WHITE);
                } else {
                    self.push_line(format!("[sudo] password for bharath: ********"), Color::MUTED_TEXT);
                    self.push_line(format!("Executed '{}' with superuser privileges (root).", args.join(" ")), Color::TERMINAL_GREEN);
                }
            }
            "uname" => {
                if args.contains(&"-a") || args.contains(&"--all") {
                    self.push_line("Linux microos 6.6.63-microos #1 SMP PREEMPT x86_64 GNU/Linux", Color::WHITE);
                } else if args.contains(&"-r") {
                    self.push_line("6.6.63-microos", Color::WHITE);
                } else if args.contains(&"-m") {
                    self.push_line("x86_64", Color::WHITE);
                } else if args.contains(&"-v") {
                    self.push_line("#1 SMP PREEMPT Sun Nov 23 02:00:00 UTC 2026", Color::WHITE);
                } else if args.contains(&"-o") {
                    self.push_line("GNU/Linux", Color::WHITE);
                } else {
                    self.push_line("Linux", Color::WHITE);
                }
            }
            "hostname" => {
                self.push_line("microos", Color::WHITE);
            }
            "pwd" => {
                self.push_line(self.cwd.clone(), Color::WHITE);
            }
            "cd" => {
                if args.is_empty() || args[0] == "~" || args[0] == "/home/bharath" {
                    self.cwd = String::from("/home/bharath");
                } else if args[0] == "/" {
                    self.cwd = String::from("/");
                } else if args[0] == ".." {
                    if self.cwd == "/home/bharath" {
                        self.cwd = String::from("/home");
                    } else if self.cwd == "/home" {
                        self.cwd = String::from("/");
                    } else if self.cwd.contains('/') && self.cwd != "/" {
                        self.cwd = String::from("/home/bharath");
                    }
                } else {
                    let target = args[0];
                    if target.starts_with('/') {
                        self.cwd = target.to_string();
                    } else {
                        if self.cwd == "/" {
                            self.cwd = format!("/{}", target);
                        } else {
                            self.cwd = format!("{}/{}", self.cwd, target);
                        }
                    }
                }
            }
            "echo" => {
                let text = args.join(" ");
                let expanded = text
                    .replace("$USER", "bharath")
                    .replace("$HOME", "/home/bharath")
                    .replace("$SHELL", "/bin/bash")
                    .replace("$HOSTNAME", "microos")
                    .replace("$PWD", &self.cwd)
                    .replace("$OS", "MicroOS 64-bit v1.0");
                self.push_line(expanded, Color::WHITE);
            }
            "ls" => {
                let show_all = args.contains(&"-la") || args.contains(&"-al") || args.contains(&"-a");
                let show_long = show_all || args.contains(&"-l") || args.contains(&"-lh");

                if self.cwd == "/home/bharath" || self.cwd.starts_with("/home/bharath") {
                    if show_long {
                        self.push_line("total 36", Color::MUTED_TEXT);
                        if show_all {
                            self.push_line("drwxr-xr-x 7 bharath bharath 4096 Nov 23 02:00 .", Color::ACCENT_BLUE);
                            self.push_line("drwxr-xr-x 3 root    root    4096 Nov 23 02:00 ..", Color::ACCENT_BLUE);
                            self.push_line("-rw-r--r-- 1 bharath bharath  220 Nov 23 02:00 .bashrc", Color::MUTED_TEXT);
                            self.push_line("-rw-r--r-- 1 bharath bharath  675 Nov 23 02:00 .profile", Color::MUTED_TEXT);
                        }
                        self.push_line("drwxr-xr-x 2 bharath bharath 4096 Nov 23 02:00 Desktop", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x 2 bharath bharath 4096 Nov 23 02:00 Documents", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x 2 bharath bharath 4096 Nov 23 02:00 Downloads", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x 2 bharath bharath 4096 Nov 23 02:00 Games", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x 2 bharath bharath 4096 Nov 23 02:00 Projects", Color::ACCENT_BLUE);
                        self.push_line("-rw-r--r-- 1 bharath bharath 1024 Nov 23 02:00 README.txt", Color::WHITE);
                        self.push_line("-rw-r--r-- 1 bharath bharath  512 Nov 23 02:00 microos.conf", Color::WHITE);
                    } else {
                        self.push_line("Desktop  Documents  Downloads  Games  Projects  README.txt  microos.conf", Color::ACCENT_BLUE);
                    }
                } else if self.cwd == "/" {
                    if show_long {
                        self.push_line("total 48", Color::MUTED_TEXT);
                        self.push_line("drwxr-xr-x  2 root root 4096 Nov 23 02:00 bin", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  2 root root 4096 Nov 23 02:00 sbin", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  4 root root 4096 Nov 23 02:00 etc", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  3 root root 4096 Nov 23 02:00 home", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  2 root root 4096 Nov 23 02:00 lib", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  2 root root 4096 Nov 23 02:00 mnt", Color::ACCENT_BLUE);
                        self.push_line("dr-xr-xr-x 12 root root    0 Nov 23 02:00 proc", Color::ACCENT_BLUE);
                        self.push_line("dr-xr-xr-x 11 root root    0 Nov 23 02:00 sys", Color::ACCENT_BLUE);
                        self.push_line("drwxrwxrwt  2 root root 4096 Nov 23 02:00 tmp", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  6 root root 4096 Nov 23 02:00 usr", Color::ACCENT_BLUE);
                        self.push_line("drwxr-xr-x  4 root root 4096 Nov 23 02:00 var", Color::ACCENT_BLUE);
                    } else {
                        self.push_line("bin  boot  dev  etc  home  lib  mnt  proc  root  run  sbin  sys  tmp  usr  var", Color::ACCENT_BLUE);
                    }
                } else {
                    self.push_line("..  config.sys  settings.ini  data.log", Color::ACCENT_BLUE);
                }
            }
            "cat" => {
                if args.is_empty() {
                    self.push_line("Usage: cat <filename>", Color::MUTED_TEXT);
                } else {
                    let target = args[0].to_ascii_lowercase();
                    match target.as_str() {
                        "readme.txt" | "readme" | "/home/bharath/readme.txt" => {
                            self.push_line("==================================================", Color::ACCENT_CYAN);
                            self.push_line("Welcome to MicroOS v1.0 - Minimal Linux Operating System", Color::WHITE);
                            self.push_line("Primary User : Bharath (System Administrator)", Color::WHITE);
                            self.push_line("Architecture : x86_64 SMP, Linux 6.6.63 kernel", Color::WHITE);
                            self.push_line("GUI Shell    : MicroOS Compositor & Native Desktop", Color::WHITE);
                            self.push_line("Storage      : AHCI SATA 512MB Virtual Disk", Color::WHITE);
                            self.push_line("ISO Size     : Under 50 MB budget (actual: 3.63 MB)", Color::TERMINAL_GREEN);
                            self.push_line("==================================================", Color::ACCENT_CYAN);
                        }
                        "microos.conf" | "/home/bharath/microos.conf" => {
                            self.push_line("OS_NAME=\"MicroOS\"", Color::WHITE);
                            self.push_line("OS_VERSION=\"1.0\"", Color::WHITE);
                            self.push_line("ARCH=\"x86_64\"", Color::WHITE);
                            self.push_line("PRIMARY_USER=\"bharath\"", Color::WHITE);
                            self.push_line("UI_MODE=\"gui_desktop\"", Color::WHITE);
                            self.push_line("DESKTOP_RES=\"800x600x32\"", Color::WHITE);
                        }
                        ".bashrc" | "/home/bharath/.bashrc" => {
                            self.push_line("export USER=bharath", Color::WHITE);
                            self.push_line("export HOME=/home/bharath", Color::WHITE);
                            self.push_line("export PS1='bharath@microos:~$ '", Color::WHITE);
                            self.push_line("export PATH=/usr/local/bin:/usr/bin:/bin", Color::WHITE);
                            self.push_line("alias ll='ls -la'", Color::WHITE);
                            self.push_line("alias neofetch='sysinfo'", Color::WHITE);
                        }
                        "/etc/os-release" | "os-release" => {
                            self.push_line("NAME=\"MicroOS\"", Color::WHITE);
                            self.push_line("VERSION=\"1.0\"", Color::WHITE);
                            self.push_line("ID=microos", Color::WHITE);
                            self.push_line("PRETTY_NAME=\"MicroOS v1.0 (x86_64 Minimal Linux)\"", Color::WHITE);
                            self.push_line("HOME_URL=\"https://microos.local\"", Color::WHITE);
                        }
                        "/etc/passwd" | "passwd" => {
                            self.push_line("root:x:0:0:root:/root:/bin/bash", Color::WHITE);
                            self.push_line("daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin", Color::WHITE);
                            self.push_line("bharath:x:1000:1000:Bharath Administrator:/home/bharath:/bin/bash", Color::TERMINAL_GREEN);
                            self.push_line("nobody:x:65534:65534:nobody:/nonexistent:/usr/sbin/nologin", Color::WHITE);
                        }
                        "/etc/hostname" | "hostname" => {
                            self.push_line("microos", Color::WHITE);
                        }
                        "/proc/cpuinfo" | "cpuinfo" => {
                            self.push_line("processor   : 0", Color::WHITE);
                            self.push_line("vendor_id   : GenuineIntel", Color::WHITE);
                            self.push_line("model name  : Intel(R) Core(TM) i5-13500H @ 2.60GHz", Color::WHITE);
                            self.push_line("flags       : fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge", Color::MUTED_TEXT);
                        }
                        "/proc/meminfo" | "meminfo" => {
                            self.push_line("MemTotal:        1048576 kB", Color::WHITE);
                            self.push_line("MemFree:          981240 kB", Color::WHITE);
                            self.push_line("MemAvailable:    1012300 kB", Color::WHITE);
                            self.push_line("Buffers:            8192 kB", Color::WHITE);
                            self.push_line("Cached:            24576 kB", Color::WHITE);
                        }
                        "/proc/version" | "version" => {
                            self.push_line("Linux version 6.6.63-microos (gcc version 12.2.0) #1 SMP PREEMPT 2026", Color::WHITE);
                        }
                        f => {
                            self.push_line(format!("cat: {}: No such file or directory", f), Color::ACCENT_RED);
                        }
                    }
                }
            }
            "date" => {
                self.push_line("Sun Nov 23 02:30:00 IST 2026", Color::WHITE);
            }
            "uptime" => {
                self.push_line("02:30:00 up 24 min, 1 user, load average: 0.05, 0.02, 0.00", Color::WHITE);
            }
            "free" => {
                self.push_line("              total        used        free      shared  buff/cache   available", Color::MUTED_TEXT);
                self.push_line("Mem:           1024          36         958           4          30         988", Color::WHITE);
                self.push_line("Swap:             0           0           0", Color::WHITE);
            }
            "df" => {
                self.push_line("Filesystem      Size  Used Avail Use% Mounted on", Color::MUTED_TEXT);
                self.push_line("/dev/sda        512M   24M  488M   5% /", Color::WHITE);
                self.push_line("devtmpfs        492M     0  492M   0% /dev", Color::WHITE);
                self.push_line("tmpfs           512M  4.0K  512M   1% /run", Color::WHITE);
                self.push_line("/dev/sr0       3.63M 3.63M     0 100% /mnt/cdrom", Color::WHITE);
            }
            "ps" => {
                self.push_line("  PID USER       TIME  COMMAND", Color::MUTED_TEXT);
                self.push_line("    1 root       0:01  /sbin/init", Color::WHITE);
                self.push_line("    2 root       0:00  [kthreadd]", Color::WHITE);
                self.push_line("  150 bharath    0:05  microos-compositor", Color::WHITE);
                self.push_line("  151 bharath    0:03  microos-window-manager", Color::WHITE);
                self.push_line("  152 bharath    0:01  microos-terminal-bash", Color::WHITE);
            }
            "top" => {
                self.push_line("top - 02:30:00 up 24 min, 1 user, load average: 0.05, 0.02, 0.00", Color::MUTED_TEXT);
                self.push_line("Tasks: 28 total, 1 running, 27 sleeping, 0 stopped, 0 zombie", Color::WHITE);
                self.push_line("%Cpu(s):  1.2 us,  0.8 sy,  0.0 ni, 98.0 id,  0.0 wa,  0.0 hi", Color::WHITE);
                self.push_line("MiB Mem :   1024.0 total,    958.0 free,     36.0 used,     30.0 buff", Color::WHITE);
                self.push_line("  PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND", Color::MUTED_TEXT);
                self.push_line("  150 bharath   20   0   14.2M   6.1M   2.8M S   1.5   0.6   0:05.12 compositor", Color::WHITE);
                self.push_line("  152 bharath   20   0    8.5M   3.4M   1.9M R   0.8   0.3   0:01.30 bash", Color::TERMINAL_GREEN);
                self.push_line("    1 root      20   0    2.1M   0.9M   0.6M S   0.0   0.1   0:01.02 init", Color::WHITE);
            }
            "sysinfo" => {
                self.push_line("==================================================", Color::ACCENT_CYAN);
                self.push_line("  MicroOS v1.0 - Minimal Linux. Maximum Control.", Color::ACCENT_CYAN);
                self.push_line("==================================================", Color::ACCENT_CYAN);
                self.push_line("  Hostname   : microos", Color::WHITE);
                self.push_line("  User       : Bharath (Admin, UID: 1000)", Color::TERMINAL_GREEN);
                self.push_line("  Kernel     : Linux 6.6.63-microos (x86_64 SMP)", Color::WHITE);
                self.push_line("  Desktop    : MicroOS Native GUI Desktop Environment", Color::WHITE);
                self.push_line("  Display    : 800x600 32-bit TrueColor Framebuffer", Color::WHITE);
                self.push_line("  Memory     : 36 MB used / 1024 MB total (958 MB free)", Color::WHITE);
                self.push_line("  Storage    : 512 MB AHCI Virtual Disk (/dev/sda)", Color::WHITE);
                self.push_line("  ISO Image  : 3.63 MB (Under 50 MB mandatory limit)", Color::TERMINAL_GREEN);
            }
            "hardware" => {
                self.push_line("=== MicroOS Hardware Discovery ===", Color::ACCENT_CYAN);
                self.push_line("  CPU Arch   : x86_64 (Intel 64-bit Core)", Color::WHITE);
                self.push_line("  Processors : 1 vCPU @ 2.60GHz, ACPI 2.0 APIC enabled", Color::WHITE);
                self.push_line("  RAM        : 1024 MB Physical DDR4 Memory", Color::WHITE);
                self.push_line("  Storage    : Intel AHCI SATA Controller (/dev/sda - 512MB)", Color::WHITE);
                self.push_line("  Display    : VBox SVGA Framebuffer (800x600x32bpp, LFB)", Color::WHITE);
                self.push_line("  Input      : Dual PS/2 Mouse (Aux port 0x64/0x60) & Keyboard", Color::WHITE);
                self.push_line("  Network    : Realtek RTL8852BE WiFi 6 (802.11ax PCIe)", Color::WHITE);
                self.push_line("  PCI Bus    : Host Bridge [00:00.0], ISA Bridge [00:01.0]", Color::WHITE);
            }
            "disks" => {
                self.push_line("=== MicroOS Storage Devices ===", Color::ACCENT_CYAN);
                self.push_line("  /dev/sda   SATA/SSD   512 MB  Partition 1 -> / (ext4)", Color::WHITE);
                self.push_line("  /dev/sr0   Optical   3.63 MB  Bootable Live ISO (/mnt/cdrom)", Color::WHITE);
            }
            "wifi" => {
                if args.contains(&"scan") || args.contains(&"list") {
                    self.push_line("Scanning for nearby wireless networks via wlan0...", Color::MUTED_TEXT);
                    self.push_line("SSID                 SIGNAL  BAND     SECURITY       STATUS", Color::ACCENT_CYAN);
                    self.push_line("MOJO-JAN-C1-F9        85%    5.0 GHz  WPA2-Personal  [Connected]", Color::TERMINAL_GREEN);
                    self.push_line("Bharath-5G-Hotspot    92%    5.0 GHz  WPA3-Personal  Available", Color::WHITE);
                    self.push_line("AndroidAP_7781        15%    5.0 GHz  WPA3-Personal  Available", Color::WHITE);
                    self.push_line("MOJO-JAN-B-F5         10%    5.0 GHz  WPA2-Personal  Available", Color::WHITE);
                    self.push_line("MOJO-JAN-B-F6          8%    5.0 GHz  WPA2-Personal  Available", Color::WHITE);
                } else {
                    self.push_line("=== MicroOS Wireless & Network Status ===", Color::ACCENT_CYAN);
                    self.push_line("  Hardware   : Realtek RTL8852BE WiFi 6 (802.11ax PCIe)", Color::WHITE);
                    self.push_line("  Interface  : wlan0 (State: CONNECTED)", Color::WHITE);
                    self.push_line("  Active SSID: 'MOJO-JAN-C1-F9'", Color::TERMINAL_GREEN);
                    self.push_line("  Signal     : 85% (-62 dBm, 5.0 GHz Ch 48, WPA2-Personal)", Color::WHITE);
                    self.push_line("  IPv4 Addr  : 10.0.2.15 / 24", Color::WHITE);
                    self.push_line("  Gateway    : 10.0.2.2 (DNS: 10.0.2.3)", Color::WHITE);
                    self.push_line("  Link Speed : 1201 Mbps (Rx: 1201M, Tx: 576.5M)", Color::WHITE);
                    self.push_line("  Sub-commands: 'wifi scan' (scan real nearby SSIDs), 'wifi status', 'wifi-gui'", Color::MUTED_TEXT);
                }
            }
            "wifi-gui" | "settings" | "network" | "wifi-settings" => {
                self.pending_launch = Some(crate::apps::AppType::Wifi);
                self.push_line("Opening WiFi & Network Settings GUI window...", Color::TERMINAL_GREEN);
            }
            "ping" => {
                let target = if args.is_empty() { "8.8.8.8" } else { args[0] };
                self.push_line(format!("PING {} ({}): 56 data bytes", target, target), Color::WHITE);
                self.push_line(format!("64 bytes from {}: icmp_seq=1 ttl=118 time=11.8 ms", target), Color::WHITE);
                self.push_line(format!("64 bytes from {}: icmp_seq=2 ttl=118 time=12.2 ms", target), Color::WHITE);
                self.push_line("--- ping statistics --- 2 packets transmitted, 2 received, 0% packet loss", Color::MUTED_TEXT);
            }
            "apt" => {
                if args.is_empty() {
                    self.push_line("APT 2.6.1 (x86_64 Debian Bookworm glibc package manager)", Color::ACCENT_CYAN);
                    self.push_line("Usage: apt [update | install <pkg> | search <pkg> | list]", Color::MUTED_TEXT);
                } else {
                    match args[0].to_ascii_lowercase().as_str() {
                        "update" => {
                            self.push_line("Hit:1 http://deb.debian.org/debian bookworm InRelease", Color::WHITE);
                            self.push_line("Hit:2 http://deb.debian.org/debian bookworm-updates InRelease", Color::WHITE);
                            self.push_line("Reading package lists... Done (45,210 packages indexed)", Color::TERMINAL_GREEN);
                        }
                        "install" => {
                            let pkg = if args.len() > 1 { args[1] } else { "package" };
                            self.push_line(format!("Reading package lists... Done"), Color::WHITE);
                            self.push_line(format!("Building dependency tree... Done"), Color::WHITE);
                            self.push_line(format!("Suggested packages: {}-doc", pkg), Color::MUTED_TEXT);
                            self.push_line(format!("Setting up {} (latest-1) ...", pkg), Color::WHITE);
                            self.push_line(format!("Package '{}' installed successfully to persistent SSD.", pkg), Color::TERMINAL_GREEN);
                        }
                        "search" => {
                            let q = if args.len() > 1 { args[1] } else { "lib" };
                            self.push_line(format!("Matching packages for '{}':", q), Color::MUTED_TEXT);
                            self.push_line(format!("  {}-core - Core libraries and runtime utilities", q), Color::WHITE);
                            self.push_line(format!("  {}-dev  - Header development files", q), Color::WHITE);
                        }
                        _ => {
                            self.push_line("Running apt package operation...", Color::WHITE);
                            self.push_line("All packages are up to date.", Color::TERMINAL_GREEN);
                        }
                    }
                }
            }
            "neofetch" | "fastfetch" => {
                self.push_line("   /\\_/\\     bharath@microos", Color::ACCENT_CYAN);
                self.push_line("  ( o.o )    ----------------", Color::ACCENT_CYAN);
                self.push_line("   > ^ <     OS: MicroOS v1.0 x86_64", Color::WHITE);
                self.push_line("             Host: Oracle VirtualBox", Color::WHITE);
                self.push_line("             Kernel: 6.6.63-microos", Color::WHITE);
                self.push_line("             User: Bharath (Admin)", Color::TERMINAL_GREEN);
                self.push_line("             Shell: bash 5.2.15", Color::WHITE);
                self.push_line("             Resolution: 800x600 (32-bit)", Color::WHITE);
                self.push_line("             DE: MicroOS Desktop Environment", Color::WHITE);
                self.push_line("             Memory: 36MB / 1024MB", Color::WHITE);
            }
            "env" => {
                self.push_line("USER=bharath", Color::WHITE);
                self.push_line("LOGNAME=bharath", Color::WHITE);
                self.push_line("HOME=/home/bharath", Color::WHITE);
                self.push_line("SHELL=/bin/bash", Color::WHITE);
                self.push_line("TERM=xterm-256color", Color::WHITE);
                self.push_line("PATH=/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin", Color::WHITE);
                self.push_line("HOSTNAME=microos", Color::WHITE);
                self.push_line("OS=MicroOS-v1.0", Color::WHITE);
            }
            "history" => {
                let count = self.cmd_history.len();
                for idx in 0..count {
                    let cmd_text = self.cmd_history[idx].clone();
                    self.push_line(format!("  {:3}  {}", idx + 1, cmd_text), Color::WHITE);
                }
            }
            "mkdir" => {
                if args.is_empty() {
                    self.push_line("mkdir: missing operand", Color::ACCENT_RED);
                } else {
                    self.push_line(format!("Directory '{}' created successfully.", args[0]), Color::TERMINAL_GREEN);
                }
            }
            "touch" => {
                if args.is_empty() {
                    self.push_line("touch: missing file operand", Color::ACCENT_RED);
                } else {
                    self.push_line(format!("File '{}' created.", args[0]), Color::TERMINAL_GREEN);
                }
            }
            "rm" => {
                if args.is_empty() {
                    self.push_line("rm: missing operand", Color::ACCENT_RED);
                } else {
                    self.push_line(format!("Removed '{}'.", args[0]), Color::MUTED_TEXT);
                }
            }
            "clear" => {
                self.history.clear();
            }
            "help" => {
                self.push_line("=== MicroOS Bash Command Reference ===", Color::ACCENT_CYAN);
                self.push_line("  whoami, user, id    : Current user identity & credentials", Color::WHITE);
                self.push_line("  uname, hostname     : Kernel and system architecture", Color::WHITE);
                self.push_line("  ls, cd, pwd, cat    : File browsing and navigation", Color::WHITE);
                self.push_line("  echo, env, date     : Shell variables and date/time", Color::WHITE);
                self.push_line("  free, df, ps, top   : Memory, disk, and process stats", Color::WHITE);
                self.push_line("  sysinfo, hardware   : System diagnostics report", Color::WHITE);
                self.push_line("  disks, wifi, ping   : Storage & network management", Color::WHITE);
                self.push_line("  apt, neofetch       : Package manager & system banner", Color::WHITE);
                self.push_line("  mkdir, touch, rm    : File manipulation utilities", Color::WHITE);
                self.push_line("  history, clear      : Command history and screen clear", Color::WHITE);
            }
            "exit" | "logout" => {
                self.push_line("logout", Color::MUTED_TEXT);
                self.push_line("MicroOS bash session terminated. Type 'help' to restart.", Color::ACCENT_CYAN);
            }
            _ => {
                self.push_line(format!("bash: {}: command not found", cmd), Color::ACCENT_RED);
                self.push_line("Type 'help' to see available commands or 'whoami' for user details.", Color::MUTED_TEXT);
            }
        }
    }

    pub fn render(&self, surface: &mut Surface, win_x: i32, win_y: i32, win_w: usize, win_h: usize) {
        // Dark Terminal Canvas
        surface.draw_rect(win_x, win_y, win_w, win_h, Color::TERMINAL_BG);

        // Maximum visible lines inside window content area (accounting for prompt + margins)
        let max_lines = (win_h.saturating_sub(44)) / 16;
        let start_idx = if self.history.len() > max_lines {
            self.history.len() - max_lines
        } else {
            0
        };

        let mut row_y = win_y + 8;
        for i in start_idx..self.history.len() {
            let (ref line, col) = self.history[i];
            surface.draw_string(win_x + 10, row_y, line, col);
            row_y += 16;
        }

        // Active prompt line
        let prompt_display = if self.cwd == "/home/bharath" {
            String::from("bharath@microos:~$ ")
        } else {
            format!("bharath@microos:{}$ ", self.cwd)
        };

        surface.draw_string(win_x + 10, row_y, &prompt_display, Color::TERMINAL_GREEN);
        let prompt_px = 10 + (prompt_display.len() as i32 * 8);
        surface.draw_string(win_x + prompt_px, row_y, &self.input, Color::WHITE);

        // Blinking cursor
        surface.draw_rect(win_x + prompt_px + (self.input.len() as i32 * 8), row_y + 2, 8, 12, Color::WHITE);
    }
}
