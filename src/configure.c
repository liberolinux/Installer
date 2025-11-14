#include "configure.h"

static const char *const libero_packages[] = {
    "sys-boot/grub",
    "sys-kernel/gentoo-kernel-bin",
    "app-admin/sudo",
    "net-misc/dhcpcd",
    "net-wireless/wpa_supplicant",
    "sys-fs/dosfstools",
    "sys-fs/squashfs-tools",
    "sys-fs/lvm2",
    "sys-fs/xfsprogs",
    "sys-fs/btrfs-progs",
    "sys-fs/e2fsprogs",
    "sys-fs/reiserfsprogs",
    "sys-fs/jfsutils",
    "sys-fs/xfsdump",
    "sys-fs/fuse",
    "sys-fs/ntfs3g",
    "sys-block/parted",
    "dev-util/dialog",
    "net-misc/ntp",
    "sys-apps/gptfdisk",
    "sys-fs/cryptsetup",
    "sys-fs/mdadm",
    "app-shells/fish",
    "app-misc/tmux",
    "app-misc/tmux-mem-cpu-load",
    "sys-process/htop",
    "dev-vcs/git",
    "app-editors/vim",
    "app-editors/emacs",
    "net-analyzer/nmap",
    "sys-apps/ethtool",
    "mail-client/alpine",
    "net-irc/irssi",
    "net-vpn/tor",
    "net-ftp/ftp",
    "dev-python/docutils",
    "net-misc/openssh",
    "app-misc/neofetch",
    "net-misc/networkmanager",
    "sys-apps/pciutils",
    "sys-apps/usbutils",
    "sys-apps/pv",
    "sys-apps/lshw",
    "sys-power/acpi",
    "app-arch/unzip",
    "app-arch/p7zip",
    "app-arch/zstd",
    "app-arch/lz4",
    "app-arch/xz-utils",
    "app-arch/bzip2",
    "app-arch/gzip",
    "net-misc/iputils",
    "net-analyzer/netcat",
    "net-analyzer/tcpdump",
    "media-sound/alsa-utils",
    "net-analyzer/iftop",
    "net-analyzer/mtr",
    "net-wireless/iw",
    "net-wireless/wireless-tools",
    "app-crypt/gnupg",
    "net-misc/rsync",
    "dev-libs/openssl",
    "dev-debug/gdb",
    "dev-debug/strace",
    "dev-debug/valgrind",
    "sys-apps/lm-sensors",
    "app-misc/mc",
    "sys-apps/dmidecode",
    "media-libs/libsndfile",
    "sys-process/iotop",
    "app-text/tree",
    "app-misc/jq",
    "sys-fs/inotify-tools",
    "app-portage/gentoolkit",
    "app-portage/eix",
    "app-portage/portage-utils",
    "sys-apps/mlocate",
    "app-admin/testdisk",
    "app-admin/sysstat",
    "media-sound/moc",
    "net-misc/whois",
    "net-misc/iperf",
    "net-misc/bridge-utils",
    "net-firewall/iptables",
    "sys-apps/smartmontools",
    "sys-process/daemontools",
    "sys-fs/ncdu",
    "sys-apps/memtester",
    "sys-apps/memtest86+",
    "app-misc/colordiff",
    "sys-fs/hfsutils",
    "sys-fs/hfsplusutils",
    "sys-apps/hdparm",
};

static const size_t libero_packages_count = sizeof(libero_packages) / sizeof(libero_packages[0]);

static int join_root_path(char *dest, size_t len, const char *root, const char *suffix)
{
    if (!dest || !root || !suffix || len == 0) {
        errno = EINVAL;
        return -1;
    }

    size_t root_len = strnlen(root, len);
    size_t suffix_len = strlen(suffix);
    if (root_len >= len || suffix_len >= len || root_len + suffix_len >= len) {
        errno = ENAMETOOLONG;
        return -1;
    }

    memcpy(dest, root, root_len);
    memcpy(dest + root_len, suffix, suffix_len);
    dest[root_len + suffix_len] = '\0';
    return 0;
}

static int configure_identity(InstallerState *state)
{
    char hostname[64];
    snprintf(hostname, sizeof(hostname), "%s", state->hostname);
    if (ui_prompt_input("Hostname", "Enter system hostname", hostname, sizeof(hostname), hostname, false) == 0) {
        snprintf(state->hostname, sizeof(state->hostname), "%s", hostname);
    }

    char timezone[64];
    snprintf(timezone, sizeof(timezone), "%s", state->timezone);
    if (ui_prompt_input("Timezone", "Enter timezone (e.g. UTC or Europe/Berlin)", timezone, sizeof(timezone), timezone, false) == 0) {
        snprintf(state->timezone, sizeof(state->timezone), "%s", timezone);
    }

    char locale[64];
    snprintf(locale, sizeof(locale), "%s", state->locale);
    if (ui_prompt_input("Locale", "Locale entry for /etc/locale.gen", locale, sizeof(locale), locale, false) == 0) {
        snprintf(state->locale, sizeof(state->locale), "%s", locale);
    }

    char lang[64];
    snprintf(lang, sizeof(lang), "%s", state->lang);
    if (ui_prompt_input("LANG", "Default LANG (e.g. en_US.UTF-8)", lang, sizeof(lang), lang, false) == 0) {
        snprintf(state->lang, sizeof(state->lang), "%s", lang);
    }

    char keymap[64];
    snprintf(keymap, sizeof(keymap), "%s", state->keymap);
    if (ui_prompt_input("Keymap", "Console keymap (e.g. us)", keymap, sizeof(keymap), keymap, false) == 0) {
        snprintf(state->keymap, sizeof(state->keymap), "%s", keymap);
    }

    return 0;
}

static int configure_root_password(InstallerState *state)
{
    char pass1[128];
    char pass2[128];
    if (ui_prompt_input("Root Password", "Enter root password", pass1, sizeof(pass1), "", true) != 0) {
        return -1;
    }
    if (ui_prompt_input("Root Password", "Confirm root password", pass2, sizeof(pass2), "", true) != 0) {
        return -1;
    }
    if (strcmp(pass1, pass2) != 0) {
        ui_message("Password", "Passwords do not match.");
        return -1;
    }
    snprintf(state->root_password, sizeof(state->root_password), "%s", pass1);
    memset(pass1, 0, sizeof(pass1));
    memset(pass2, 0, sizeof(pass2));
    return 0;
}

static int configure_user(InstallerState *state)
{
    if (!ui_confirm("User Account", "Create a regular user account?")) {
        state->create_user = false;
        state->username[0] = '\0';
        state->user_password[0] = '\0';
        return 0;
    }

    state->create_user = true;
    char username[64];
    snprintf(username, sizeof(username), "%s", state->username[0] ? state->username : "libero");
    if (ui_prompt_input("Username", "Enter username", username, sizeof(username), username, false) != 0) {
        return -1;
    }
    snprintf(state->username, sizeof(state->username), "%s", username);

    char pass1[128];
    char pass2[128];
    if (ui_prompt_input("User Password", "Enter password", pass1, sizeof(pass1), "", true) != 0) {
        return -1;
    }
    if (ui_prompt_input("User Password", "Confirm password", pass2, sizeof(pass2), "", true) != 0) {
        return -1;
    }
    if (strcmp(pass1, pass2) != 0) {
        ui_message("User Password", "Passwords do not match.");
        return -1;
    }
    snprintf(state->user_password, sizeof(state->user_password), "%s", pass1);
    memset(pass1, 0, sizeof(pass1));
    memset(pass2, 0, sizeof(pass2));
    return 0;
}

static int write_make_conf(const InstallerState *state)
{
    char path[PATH_MAX];
    if (join_root_path(path, sizeof(path), state->install_root, "/etc/portage") != 0) {
        return -1;
    }
    ensure_directory(path, 0755);
    if (join_root_path(path, sizeof(path), state->install_root, "/etc/portage/make.conf") != 0) {
        return -1;
    }

    const char *cflags = (state->arch == ARCH_I486) ? "-march=i486 -O2 -pipe" : "-march=i686 -O2 -pipe";
    const char *chost = (state->arch == ARCH_I486) ? "i486-pc-linux-gnu" : "i686-pc-linux-gnu";
    char content[1024];
    snprintf(content, sizeof(content),
             "COMMON_FLAGS=\"%s\"\n"
             "CFLAGS=\"${COMMON_FLAGS}\"\n"
             "CXXFLAGS=\"${COMMON_FLAGS}\"\n"
             "CHOST=\"%s\"\n"
             "MAKEOPTS=\"-j2\"\n"
             "ACCEPT_LICENSE=\"*\"\n"
             "GENTOO_MIRRORS=\"%s\"\n"
             "INPUT_DEVICES=\"libinput\"\n"
             "VIDEO_CARDS=\"\"\n",
             cflags, chost, state->mirror_url);

    return write_text_file(path, content);
}

static int write_locale_files(const InstallerState *state)
{
    char path[PATH_MAX];
    char content[512];

    if (join_root_path(path, sizeof(path), state->install_root, "/etc/locale.gen") != 0) {
        return -1;
    }
    snprintf(content, sizeof(content), "%s\n", state->locale);
    if (write_text_file(path, content) != 0) {
        return -1;
    }

    if (join_root_path(path, sizeof(path), state->install_root, "/etc/env.d/02locale") != 0) {
        return -1;
    }
    snprintf(content, sizeof(content),
             "LANG=\"%s\"\nLC_COLLATE=\"C\"\n", state->lang);
    if (write_text_file(path, content) != 0) {
        return -1;
    }

    if (join_root_path(path, sizeof(path), state->install_root, "/etc/vconsole.conf") != 0) {
        return -1;
    }
    snprintf(content, sizeof(content), "KEYMAP=%s\n", state->keymap);
    if (write_text_file(path, content) != 0) {
        return -1;
    }

    if (join_root_path(path, sizeof(path), state->install_root, "/etc/timezone") != 0) {
        return -1;
    }
    snprintf(content, sizeof(content), "%s\n", state->timezone);
    if (write_text_file(path, content) != 0) {
        return -1;
    }

    if (join_root_path(path, sizeof(path), state->install_root, "/etc/hostname") != 0) {
        return -1;
    }
    snprintf(content, sizeof(content), "%s\n", state->hostname);
    if (write_text_file(path, content) != 0) {
        return -1;
    }

    return 0;
}

static int write_fstab(const InstallerState *state)
{
    char path[PATH_MAX];
    if (join_root_path(path, sizeof(path), state->install_root, "/etc/fstab") != 0) {
        return -1;
    }
    FILE *f = fopen(path, "w");
    if (!f) {
        ui_message("fstab", "Unable to open fstab for writing.");
        return -1;
    }

    char root_uuid[128];
    char boot_uuid[128];
    char efi_uuid[128];
    char swap_uuid[128];

    const char *root_device = state->root_mapper[0] ? state->root_mapper : state->root_partition;
    if (get_block_uuid(root_device, root_uuid, sizeof(root_uuid)) != 0) {
        fclose(f);
        return -1;
    }
    fprintf(f, "UUID=%s\t/\t%s\tdefaults,noatime\t0 1\n", root_uuid, fs_to_string(state->root_fs));

    if (state->boot_partition[0] && get_block_uuid(state->boot_partition, boot_uuid, sizeof(boot_uuid)) == 0) {
        fprintf(f, "UUID=%s\t/boot\text2\tdefaults,noatime\t0 2\n", boot_uuid);
    }
    if (state->efi_partition[0] && get_block_uuid(state->efi_partition, efi_uuid, sizeof(efi_uuid)) == 0) {
        fprintf(f, "UUID=%s\t/boot/efi\tvfat\tdefaults\t0 2\n", efi_uuid);
    }
    const char *swap_device = state->swap_mapper[0] ? state->swap_mapper : state->swap_partition;
    if (state->swap_size_mb > 0 && swap_device[0] && get_block_uuid(swap_device, swap_uuid, sizeof(swap_uuid)) == 0) {
        fprintf(f, "UUID=%s\tnone\tswap\tsw\t0 0\n", swap_uuid);
    }

    fclose(f);
    return 0;
}

static int apply_configuration_files(InstallerState *state)
{
    if (!state->stage3_ready) {
        ui_message("Configuration", "Stage3 must be extracted before writing configuration files.");
        return -1;
    }
    if (write_make_conf(state) != 0) {
        return -1;
    }
    if (write_locale_files(state) != 0) {
        return -1;
    }
    if (write_fstab(state) != 0) {
        return -1;
    }
    ui_message("Configuration", "Configuration files updated.");
    return 0;
}

static int script_append(char *script, size_t size, const char *fmt, ...)
{
    size_t len = strlen(script);
    if (len >= size) {
        return -1;
    }
    va_list args;
    va_start(args, fmt);
    int rc = vsnprintf(script + len, size - len, fmt, args);
    va_end(args);
    if (rc < 0 || (size_t)rc >= size - len) {
        return -1;
    }
    return 0;
}

static int append_libero_package_array(char *script, size_t size)
{
    if (script_append(script, size, "libero_packages=(\n") != 0) {
        return -1;
    }
    for (size_t i = 0; i < libero_packages_count; ++i) {
        if (script_append(script, size, "    '%s'\n", libero_packages[i]) != 0) {
            return -1;
        }
    }
    if (script_append(script, size, ")\n") != 0) {
        return -1;
    }
    return 0;
}

static int apply_libero_profile(InstallerState *state)
{
    char hostname_q[256];
    char user_q[256];
    char binhost_q[512];
    shell_escape_single_quotes(state->hostname, hostname_q, sizeof(hostname_q));
    if (state->create_user) {
        shell_escape_single_quotes(state->username, user_q, sizeof(user_q));
    } else {
        user_q[0] = '\0';
    }

    const char *binhost = (state->arch == ARCH_I686) ? LIBERO_BINHOST_I686 : LIBERO_BINHOST_I486;
    shell_escape_single_quotes(binhost, binhost_q, sizeof(binhost_q));

    char script[65536] = {0};
    script_append(script, sizeof(script), "set -euo pipefail\n");
    script_append(script, sizeof(script), "source /etc/profile\n");
    script_append(script, sizeof(script), "LIBERO_NAME='%s'\n", LIBERO_DISTRO_NAME);
    script_append(script, sizeof(script), "LIBERO_VERSION='%s'\n", LIBERO_RELEASE_VERSION);
    script_append(script, sizeof(script), "LIBERO_HOSTNAME='%s'\n", hostname_q);
    script_append(script, sizeof(script), "LIBERO_OS_ID='%s'\n", LIBERO_OS_ID);
    script_append(script, sizeof(script), "LIBERO_OS_COLOR='%s'\n", LIBERO_OS_COLOR);
    script_append(script, sizeof(script), "LIBERO_HOME='%s'\n", LIBERO_HOME_URL);
    script_append(script, sizeof(script), "LIBERO_BINHOST='%s'\n", binhost_q);
    script_append(script, sizeof(script), "LIBERO_USER='%s'\n", state->create_user ? user_q : "");
    script_append(script, sizeof(script), "if [ -n \"$LIBERO_USER\" ]; then LIBERO_USER_HOME=\"/home/$LIBERO_USER\"; else LIBERO_USER_HOME=\"\"; fi\n");

    script_append(script, sizeof(script),
                  "update_make_conf() {\n"
                  "    local key=\"$1\"\n"
                  "    local value=\"$2\"\n"
                  "    if grep -q \"^${key}=\" /etc/portage/make.conf; then\n"
                  "        sed -i \"s|^${key}=.*|${key}=\\\"${value}\\\"|\" /etc/portage/make.conf\n"
                  "    else\n"
                  "        printf '%%s=\"%%s\"\\n' \"$key\" \"$value\" >> /etc/portage/make.conf\n"
                  "    fi\n"
                  "}\n");
    script_append(script, sizeof(script), "mkdir -p /etc/portage/package.use /etc/portage/package.accept_keywords /etc/portage/package.license\n");
    script_append(script, sizeof(script),
                  "cat <<'EOF' >/etc/portage/package.use/libero\n"
                  ">=sys-kernel/installkernel-50 dracut\n"
                  "net-misc/iputils -filecaps\n"
                  "app-portage/portage-utils -openmp\n"
                  "net-wireless/wpa_supplicant dbus\n"
                  "EOF\n");
    script_append(script, sizeof(script),
                  "cat <<'EOF' >/etc/portage/package.accept_keywords/cmake\n"
                  "=dev-build/cmake-3.31.9-r1 **\n"
                  "EOF\n");
    script_append(script, sizeof(script),
                  "cat <<'EOF' >>/etc/portage/package.license/libero\n"
                  "sys-kernel/linux-firmware @BINARY-REDISTRIBUTABLE\n"
                  "EOF\n");
    script_append(script, sizeof(script), "nproc_count=$(nproc 2>/dev/null || echo 2)\n");
    script_append(script, sizeof(script), "update_make_conf MAKEOPTS \"-j${nproc_count}\"\n");
    script_append(script, sizeof(script), "update_make_conf FEATURES \"getbinpkg binpkg-logs\"\n");
    script_append(script, sizeof(script), "update_make_conf PORTAGE_BINHOST \"$LIBERO_BINHOST\"\n");
    script_append(script, sizeof(script), "update_make_conf EMERGE_DEFAULT_OPTS \"--getbinpkg --usepkg\"\n");
    script_append(script, sizeof(script), "emerge --sync --quiet\n");
    script_append(script, sizeof(script), "emerge --quiet-build=y =dev-build/cmake-3.31.9-r1\n");
    script_append(script, sizeof(script),
                  "if command -v getuto >/dev/null 2>&1; then\n"
                  "    getuto || echo \"Warning: getuto failed, continuing\"\n"
                  "else\n"
                  "    echo \"getuto not present; skipping binary package verification setup\"\n"
                  "fi\n");
    script_append(script, sizeof(script), "emerge --quiet-build=y sys-kernel/linux-firmware\n");

    if (append_libero_package_array(script, sizeof(script)) != 0) {
        return -1;
    }
    script_append(script, sizeof(script),
                  "emerge --quiet-build=y --keep-going --with-bdeps=y \"${libero_packages[@]}\"\n");

    script_append(script, sizeof(script),
                  "cat <<EOF >/usr/lib/os-release\n"
                  "NAME=\"%s\"\n"
                  "ID=%s\n"
                  "PRETTY_NAME=\"%s %s\"\n"
                  "ANSI_COLOR=\"%s\"\n"
                  "HOME_URL=\"%s\"\n"
                  "VERSION_ID=\"%s\"\n"
                  "EOF\n",
                  LIBERO_DISTRO_NAME,
                  LIBERO_OS_ID,
                  LIBERO_DISTRO_NAME,
                  LIBERO_RELEASE_VERSION,
                  LIBERO_OS_COLOR,
                  LIBERO_HOME_URL,
                  LIBERO_RELEASE_VERSION);
    script_append(script, sizeof(script), "echo \"${LIBERO_NAME} ${LIBERO_VERSION}\" > /etc/gentoo-release\n");
    script_append(script, sizeof(script), "printf 'nameserver 9.9.9.9\\n' > /etc/resolv.conf\n");

    script_append(script, sizeof(script), "chsh -s /usr/bin/fish root || true\n");
    script_append(script, sizeof(script),
                  "if [ -n \"$LIBERO_USER\" ]; then\n"
                  "    chsh -s /usr/bin/fish \"$LIBERO_USER\" || true\n"
                  "fi\n");

    script_append(script, sizeof(script),
                  "mkdir -p /root/.config/fish\n"
                  "cat <<'EOF' >/root/.config/fish/config.fish\n"
                  "printf '\\033]10;#000000\\007'\n"
                  "printf '\\033]11;#ffffff\\007'\n"
                  "set -g fish_greeting \"Welcome to " LIBERO_DISTRO_NAME " " LIBERO_RELEASE_VERSION "!\"\n"
                  "if status is-login\n"
                  "    cd $HOME\n"
                  "end\n"
                  "\n"
                  "if status is-interactive; and not set -q TMUX\n"
                  "    exec tmux\n"
                  "end\n"
                  "EOF\n");
    script_append(script, sizeof(script),
                  "if [ -n \"$LIBERO_USER_HOME\" ]; then\n"
                  "    mkdir -p \"$LIBERO_USER_HOME/.config/fish\"\n"
                  "    cat <<'EOF' >\"$LIBERO_USER_HOME/.config/fish/config.fish\"\n"
                  "printf '\\033]10;#000000\\007'\n"
                  "printf '\\033]11;#ffffff\\007'\n"
                  "set -g fish_greeting \"Welcome to " LIBERO_DISTRO_NAME " " LIBERO_RELEASE_VERSION "!\"\n"
                  "if status is-login\n"
                  "    cd $HOME\n"
                  "end\n"
                  "\n"
                  "if status is-interactive; and not set -q TMUX\n"
                  "    exec tmux\n"
                  "end\n"
                  "EOF\n"
                  "    chown -R \"$LIBERO_USER:$LIBERO_USER\" \"$LIBERO_USER_HOME/.config\"\n"
                  "fi\n");

    script_append(script, sizeof(script),
                  "cat <<'EOF' >/root/.tmux.conf\n"
                  "set -g status-interval 1\n"
                  "set -g status-left \"#[fg=green,bg=black]#(tmux-mem-cpu-load --colors --interval 1)\"\n"
                  "set -g status-left-length 60\n"
                  "set -g status-right \"[#(date +'%d/%m/%Y %H:%M')]\"\n"
                  "EOF\n");
    script_append(script, sizeof(script),
                  "if [ -n \"$LIBERO_USER_HOME\" ]; then\n"
                  "    cat <<'EOF' >\"$LIBERO_USER_HOME/.tmux.conf\"\n"
                  "set -g status-interval 1\n"
                  "set -g status-left \"#[fg=green,bg=black]#(tmux-mem-cpu-load --colors --interval 1)\"\n"
                  "set -g status-left-length 60\n"
                  "set -g status-right \"[#(date +'%d/%m/%Y %H:%M')]\"\n"
                  "EOF\n"
                  "    chown \"$LIBERO_USER:$LIBERO_USER\" \"$LIBERO_USER_HOME/.tmux.conf\"\n"
                  "fi\n");

    script_append(script, sizeof(script),
                  "if ! grep -q '^root ALL=' /etc/sudoers; then\n"
                  "    echo 'root ALL=(ALL) NOPASSWD: ALL' >> /etc/sudoers\n"
                  "fi\n"
                  "if ! grep -q '^%wheel' /etc/sudoers; then\n"
                  "    echo '%wheel ALL=(ALL) NOPASSWD: ALL' >> /etc/sudoers\n"
                  "fi\n"
                  "if [ -n \"$LIBERO_USER\" ] && ! grep -q \"^${LIBERO_USER} ALL\" /etc/sudoers; then\n"
                  "    echo \"${LIBERO_USER} ALL=(ALL) NOPASSWD: ALL\" >> /etc/sudoers\n"
                  "fi\n");

    script_append(script, sizeof(script),
                  "if [ ! -d /opt/vim_runtime ]; then\n"
                  "    git clone --depth=1 https://github.com/amix/vimrc.git /opt/vim_runtime || true\n"
                  "fi\n"
                  "if [ -d /opt/vim_runtime ]; then\n"
                  "    (cd /opt/vim_runtime && python update_plugins.py || true)\n"
                  "    sh /opt/vim_runtime/install_awesome_parameterized.sh /opt/vim_runtime root ${LIBERO_USER:-} || true\n"
                  "fi\n");

    script_append(script, sizeof(script),
                  "if [ -d /root/.emacs.d ]; then rm -rf /root/.emacs.d; fi\n"
                  "git clone --depth=1 https://github.com/emacs-exordium/exordium.git /root/.emacs.d || true\n"
                  "if command -v emacs >/dev/null 2>&1; then\n"
                  "    emacs --batch -l /root/.emacs.d/init.el --eval='(require (quote package))' --eval='(package-refresh-contents)' --eval='(dolist (pkg package-selected-packages) (unless (package-installed-p pkg) (ignore-errors (package-install pkg))))' --eval='(if (and (fboundp (quote native-comp-available-p)) (native-comp-available-p) (fboundp (quote batch-native-compile))) (batch-native-compile \"/root/.emacs.d\") (byte-recompile-directory \"/root/.emacs.d\" 0))' || true\n"
                  "fi\n");
    script_append(script, sizeof(script),
                  "if [ -n \"$LIBERO_USER_HOME\" ]; then\n"
                  "    rm -rf \"$LIBERO_USER_HOME/.emacs.d\"\n"
                  "    git clone --depth=1 https://github.com/emacs-exordium/exordium.git \"$LIBERO_USER_HOME/.emacs.d\" || true\n"
                  "    chown -R \"$LIBERO_USER:$LIBERO_USER\" \"$LIBERO_USER_HOME/.emacs.d\"\n"
                  "    if command -v emacs >/dev/null 2>&1; then\n"
                  "        sudo -u \"$LIBERO_USER\" emacs --batch -l \"$LIBERO_USER_HOME/.emacs.d/init.el\" --eval='(require (quote package))' --eval='(package-refresh-contents)' --eval='(dolist (pkg package-selected-packages) (unless (package-installed-p pkg) (ignore-errors (package-install pkg))))' --eval='(if (and (fboundp (quote native-comp-available-p)) (native-comp-available-p) (fboundp (quote batch-native-compile))) (batch-native-compile \\\"$LIBERO_USER_HOME/.emacs.d\\\") (byte-recompile-directory \\\"$LIBERO_USER_HOME/.emacs.d\\\" 0))' || true\n"
                  "    fi\n"
                  "fi\n");

    script_append(script, sizeof(script),
                  "cat <<'EOF' >/etc/systemd/system/zram-swap.service\n"
                  "[Unit]\n"
                  "Description=Setup zram swap for Libero system\n"
                  "Documentation=man:zram\n"
                  "DefaultDependencies=no\n"
                  "After=systemd-modules-load.service systemd-udev-settle.service\n"
                  "Before=swap.target sysinit.target\n"
                  "Wants=systemd-modules-load.service\n"
                  "\n"
                  "[Service]\n"
                  "Type=oneshot\n"
                  "RemainAfterExit=yes\n"
                  "ExecStart=/bin/bash -c 'modprobe zram num_devices=1'\n"
                  "ExecStart=/bin/bash -c 'echo lz4 > /sys/block/zram0/comp_algorithm'\n"
                  "ExecStart=/bin/bash -c 'awk \"/MemTotal/{print \\$2}\" /proc/meminfo | awk '{print $1 * 1024 / 2}' > /sys/block/zram0/disksize'\n"
                  "ExecStart=/bin/bash -c 'mkswap /dev/zram0'\n"
                  "ExecStart=/bin/bash -c 'swapon /dev/zram0 -p 10'\n"
                  "ExecStop=/bin/bash -c 'swapoff /dev/zram0'\n"
                  "ExecStop=/bin/bash -c 'echo 1 > /sys/block/zram0/reset'\n"
                  "TimeoutSec=30\n"
                  "\n"
                  "[Install]\n"
                  "WantedBy=swap.target\n"
                  "EOF\n");

    script_append(script, sizeof(script),
                  "systemctl enable dhcpcd.service NetworkManager.service sshd.service zram-swap.service\n");

    if (chroot_run_script(state->install_root, script) != 0) {
        return -1;
    }

    return 0;
}

static int run_base_install(InstallerState *state)
{
    if (!state->stage3_ready || !state->disk_prepared) {
        ui_message("Install", "Disk and stage3 must be ready first.");
        return -1;
    }
    if (!state->root_password[0]) {
        if (configure_root_password(state) != 0) {
            return -1;
        }
    }

    char hostname_q[256];
    char timezone_q[256];
    char locale_q[256];
    char lang_q[256];
    char keymap_q[256];
    char root_pw_q[256];
    char user_q[256];
    char user_pw_q[256];

    shell_escape_single_quotes(state->hostname, hostname_q, sizeof(hostname_q));
    shell_escape_single_quotes(state->timezone, timezone_q, sizeof(timezone_q));
    shell_escape_single_quotes(state->locale, locale_q, sizeof(locale_q));
    shell_escape_single_quotes(state->lang, lang_q, sizeof(lang_q));
    shell_escape_single_quotes(state->keymap, keymap_q, sizeof(keymap_q));
    shell_escape_single_quotes(state->root_password, root_pw_q, sizeof(root_pw_q));
    if (state->create_user) {
        shell_escape_single_quotes(state->username, user_q, sizeof(user_q));
        shell_escape_single_quotes(state->user_password, user_pw_q, sizeof(user_pw_q));
    }

    char script[8192] = {0};
    script_append(script, sizeof(script), "set -euo pipefail\n");
    script_append(script, sizeof(script), "source /etc/profile\n");
    script_append(script, sizeof(script), "emerge-webrsync\n");
    script_append(script, sizeof(script), "emaint sync --auto\n");
    script_append(script, sizeof(script), "eselect profile set default/linux/x86/23.0/systemd\n");
    script_append(script, sizeof(script), "emerge --quiet-build=y --update --deep --newuse @world\n");
    script_append(script, sizeof(script),
                  "emerge --quiet-build=y sys-kernel/gentoo-kernel-bin grub:2 dhcpcd NetworkManager sudo\n");
    script_append(script, sizeof(script), "locale-gen\n");
    script_append(script, sizeof(script), "eselect locale set '%s'\n", lang_q);
    script_append(script, sizeof(script), "env-update\n");
    script_append(script, sizeof(script), "echo '%s' > /etc/hostname\n", hostname_q);
    script_append(script, sizeof(script), "ln -sf '/usr/share/zoneinfo/%s' /etc/localtime\n", timezone_q);
    script_append(script, sizeof(script), "echo '%s' > /etc/timezone\n", timezone_q);
    script_append(script, sizeof(script), "printf 'root:%s\\n' | chpasswd\n", root_pw_q);
    script_append(script, sizeof(script), "systemctl enable NetworkManager.service\n");
    script_append(script, sizeof(script), "systemctl enable sshd.service\n");
    script_append(script, sizeof(script), "systemctl enable dhcpcd.service\n");

    if (state->create_user) {
        script_append(script, sizeof(script),
                      "useradd -m -G wheel,audio,video,usb,plugdev '%s' || true\n", user_q);
        script_append(script, sizeof(script), "printf '%s:%s\\n' | chpasswd\n", user_q, user_pw_q);
    }

    if (state->keymap[0]) {
        script_append(script, sizeof(script),
                      "echo 'KEYMAP=%s' > /etc/vconsole.conf\n", keymap_q);
    }

    if (chroot_run_script(state->install_root, script) != 0) {
        ui_message("Install", "Base system installation failed.");
        return -1;
    }

    if (apply_libero_profile(state) != 0) {
        ui_message("Install", "Libero profile installation failed.");
        return -1;
    }

    ui_message("Install", "Base system packages and Libero profile installed.");
    return 0;
}

static int install_bootloader(InstallerState *state)
{
    if (!state->stage3_ready) {
        ui_message("Bootloader", "Stage3 must be extracted first.");
        return -1;
    }
    if (!state->target_disk[0]) {
        ui_message("Bootloader", "No target disk selected.");
        return -1;
    }

    char script[1024] = {0};
    script_append(script, sizeof(script), "set -euo pipefail\n");
    if (state->boot_mode == BOOTMODE_UEFI) {
        script_append(script, sizeof(script),
                      "grub-install --target=i386-efi --efi-directory=/boot/efi --bootloader-id=Gentoo --recheck\n");
    } else {
        script_append(script, sizeof(script),
                      "grub-install --target=i386-pc %s --recheck\n", state->target_disk);
    }
    script_append(script, sizeof(script), "grub-mkconfig -o /boot/grub/grub.cfg\n");

    if (chroot_run_script(state->install_root, script) != 0) {
        ui_message("Bootloader", "Failed to install GRUB.");
        return -1;
    }
    state->bootloader_installed = true;
    ui_message("Bootloader", "GRUB installed successfully.");
    return 0;
}

int configure_workflow(InstallerState *state)
{
    const char *items[] = {
        "Set hostname, locale, timezone, keymap",
        "Set root password",
        "Configure regular user",
        "Write configuration files (make.conf, fstab, locale)",
        "Install Libero packages and profile",
        "Install GRUB bootloader",
        "Back to main menu",
    };

    while (1) {
        char subtitle[256];
        snprintf(subtitle, sizeof(subtitle),
                 "Hostname: %s | Locale: %s | User: %s",
                 state->hostname,
                 state->lang,
                 state->create_user ? state->username : "<none>");

        int choice = ui_menu("Configure Gentoo", subtitle, items, 7, 0);
        if (choice < 0 || choice == 6) {
            return 0;
        }

        switch (choice) {
        case 0:
            configure_identity(state);
            break;
        case 1:
            configure_root_password(state);
            break;
        case 2:
            configure_user(state);
            break;
        case 3:
            apply_configuration_files(state);
            break;
        case 4:
            run_base_install(state);
            break;
        case 5:
            install_bootloader(state);
            break;
        default:
            break;
        }
    }
}
