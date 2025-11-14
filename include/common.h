#ifndef LIBERO_INSTALLER_COMMON_H
#define LIBERO_INSTALLER_COMMON_H

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define INSTALLER_NAME "Libero Gentoo Installer"
#define INSTALLER_VERSION "1.0"

#define INSTALL_ROOT_DEFAULT "/mnt/gentoo"
#define INSTALL_CACHE_DIR "/var/tmp/libero-installer"
#define INSTALL_LOG_PATH "/var/log/libero-installer.log"
#define MIRROR_URL_MAX 512
#define REMOTE_URL_MAX 2048

#define LIBERO_DISTRO_NAME "Libero GNU/Linux"
#define LIBERO_RELEASE_VERSION "1.2"
#define LIBERO_OS_ID "libero"
#define LIBERO_OS_COLOR "1;34"
#define LIBERO_HOME_URL "https://libero.eu.org/"
#define LIBERO_BINHOST_I486 "https://distfiles.gentoo.org/releases/x86/binpackages/23.0/i486/"
#define LIBERO_BINHOST_I686 "https://distfiles.gentoo.org/releases/x86/binpackages/23.0/i686/"

#define STAGE3_BASE_URL "https://distfiles.gentoo.org/releases/x86/autobuilds"
#define PORTAGE_BASE_URL "https://distfiles.gentoo.org/snapshots"
#define PORTAGE_SNAPSHOT_NAME "portage-latest.tar.xz"

#define DEFAULT_HOSTNAME "gentoo"
#define DEFAULT_TIMEZONE "UTC"
#define DEFAULT_KEYMAP "us"
#define DEFAULT_LOCALE "en_US.UTF-8 UTF-8"
#define DEFAULT_LANG "en_US.UTF-8"
#define DEFAULT_VG_NAME "libero"
#define DEFAULT_LUKS_NAME "cryptroot"

#define MAX_CMD_LEN 4096
#define MAX_MESSAGE_LEN 1024

#endif /* LIBERO_INSTALLER_COMMON_H */
