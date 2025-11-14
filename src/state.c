#include "state.h"

void installer_state_init(InstallerState *state)
{
    if (!state) {
        return;
    }

    memset(state, 0, sizeof(*state));

    state->arch = ARCH_I486;
    state->root_fs = FS_EXT4;
    state->swap_size_mb = 1024;
    state->boot_mode = (access("/sys/firmware/efi/efivars", F_OK) == 0) ? BOOTMODE_UEFI : BOOTMODE_LEGACY;
    state->use_luks = false;
    state->use_lvm = false;
    state->disk_prepared = false;
    state->network_configured = false;
    state->stage3_ready = false;
    state->bootloader_installed = false;
    state->static_prefix = 24;

    snprintf(state->install_root, sizeof(state->install_root), "%s", INSTALL_ROOT_DEFAULT);
    snprintf(state->mirror_url, sizeof(state->mirror_url), "%s", STAGE3_BASE_URL);
    snprintf(state->portage_url, sizeof(state->portage_url), "%s/%s", PORTAGE_BASE_URL, PORTAGE_SNAPSHOT_NAME);
    snprintf(state->stage3_local, sizeof(state->stage3_local), INSTALL_CACHE_DIR "/stage3.tar.xz");
    snprintf(state->stage3_digest_local, sizeof(state->stage3_digest_local), INSTALL_CACHE_DIR "/stage3.tar.xz.DIGESTS");
    snprintf(state->portage_local, sizeof(state->portage_local), INSTALL_CACHE_DIR "/%s", PORTAGE_SNAPSHOT_NAME);
    snprintf(state->vg_name, sizeof(state->vg_name), "%s", DEFAULT_VG_NAME);
    snprintf(state->luks_name, sizeof(state->luks_name), "%s", DEFAULT_LUKS_NAME);

    snprintf(state->hostname, sizeof(state->hostname), "%s", DEFAULT_HOSTNAME);
    snprintf(state->timezone, sizeof(state->timezone), "%s", DEFAULT_TIMEZONE);
    snprintf(state->keymap, sizeof(state->keymap), "%s", DEFAULT_KEYMAP);
    snprintf(state->locale, sizeof(state->locale), "%s", DEFAULT_LOCALE);
    snprintf(state->lang, sizeof(state->lang), "%s", DEFAULT_LANG);
    state->create_user = true;
    snprintf(state->username, sizeof(state->username), "%s", "libero");
    state->root_password[0] = '\0';
    state->user_password[0] = '\0';

    state->network_interface[0] = '\0';
    state->network_dhcp = true;
    state->static_ip[0] = '\0';
    state->static_gateway[0] = '\0';
    state->static_dns[0] = '\0';

    state->target_disk[0] = '\0';
    state->disk_model[0] = '\0';
    state->disk_size_mb = 0;

    state->boot_partition[0] = '\0';
    state->efi_partition[0] = '\0';
    state->root_partition[0] = '\0';
    state->swap_partition[0] = '\0';
    state->root_mapper[0] = '\0';
    state->swap_mapper[0] = '\0';

    snprintf(state->stage3_url, sizeof(state->stage3_url), "%s", "");
    snprintf(state->stage3_digest_url, sizeof(state->stage3_digest_url), "%s", "");
}

const char *arch_to_string(GentooArch arch)
{
    switch (arch) {
    case ARCH_I486:
        return "i486";
    case ARCH_I686:
        return "i686";
    default:
        return "unknown";
    }
}

const char *boot_mode_to_string(BootMode mode)
{
    switch (mode) {
    case BOOTMODE_LEGACY:
        return "Legacy BIOS (MBR)";
    case BOOTMODE_UEFI:
        return "UEFI (GPT)";
    default:
        return "Unknown";
    }
}

const char *fs_to_string(FilesystemType fs)
{
    switch (fs) {
    case FS_EXT4:
        return "ext4";
    case FS_XFS:
        return "xfs";
    case FS_BTRFS:
        return "btrfs";
    default:
        return "unknown";
    }
}
