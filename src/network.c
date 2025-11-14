#include "network.h"

#include <dirent.h>

typedef struct {
    char name[64];
    char mac[32];
    char operstate[32];
} NetInterface;

static void read_file_trim(const char *path, char *buffer, size_t len)
{
    buffer[0] = '\0';
    FILE *f = fopen(path, "r");
    if (!f) {
        return;
    }
    if (fgets(buffer, (int)len, f)) {
        size_t l = strlen(buffer);
        while (l > 0 && (buffer[l - 1] == '\n' || buffer[l - 1] == '\r')) {
            buffer[--l] = '\0';
        }
    }
    fclose(f);
}

static NetInterface *list_interfaces(size_t *count)
{
    DIR *dir = opendir("/sys/class/net");
    if (!dir) {
        return NULL;
    }

    size_t capacity = 4;
    size_t n = 0;
    NetInterface *items = calloc(capacity, sizeof(NetInterface));

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        if (strcmp(entry->d_name, "lo") == 0) {
            continue;
        }

        if (n == capacity) {
            capacity *= 2;
            NetInterface *tmp = realloc(items, capacity * sizeof(NetInterface));
            if (!tmp) {
                free(items);
                closedir(dir);
                return NULL;
            }
            items = tmp;
        }

        snprintf(items[n].name, sizeof(items[n].name), "%s", entry->d_name);
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/sys/class/net/%s/address", entry->d_name);
        read_file_trim(path, items[n].mac, sizeof(items[n].mac));
        snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", entry->d_name);
        read_file_trim(path, items[n].operstate, sizeof(items[n].operstate));
        if (!items[n].operstate[0]) {
            snprintf(items[n].operstate, sizeof(items[n].operstate), "unknown");
        }
        n++;
    }

    closedir(dir);
    if (count) {
        *count = n;
    }
    return items;
}

static int select_interface(InstallerState *state)
{
    size_t count = 0;
    NetInterface *ifs = list_interfaces(&count);
    if (!ifs || count == 0) {
        ui_message("Interfaces", "No active network interfaces detected.");
        free(ifs);
        return -1;
    }

    char **items = calloc(count, sizeof(char *));
    for (size_t i = 0; i < count; ++i) {
        char line[256];
        snprintf(line, sizeof(line), "%s (%s, %s)", ifs[i].name, ifs[i].mac, ifs[i].operstate);
        items[i] = strdup(line);
    }

    int choice = ui_menu("Network Interface", "Select interface to configure",
                         (const char **)items, count, 0);
    if (choice >= 0) {
        snprintf(state->network_interface, sizeof(state->network_interface), "%s", ifs[choice].name);
        ui_message("Interface Selected", state->network_interface);
    }

    for (size_t i = 0; i < count; ++i) {
        free(items[i]);
    }
    free(items);
    free(ifs);
    return (choice >= 0) ? 0 : -1;
}

static int ensure_interface_selected(const InstallerState *state)
{
    if (!state->network_interface[0]) {
        ui_message("Network", "Please select an interface first.");
        return -1;
    }
    return 0;
}

static int configure_dhcp(InstallerState *state)
{
    if (ensure_interface_selected(state) != 0) {
        return -1;
    }

    const char *iface = state->network_interface;
    run_command("dhcpcd -k %s >/dev/null 2>&1 || true", iface);
    if (run_command("ip link set %s down", iface) != 0) {
        return -1;
    }
    if (run_command("ip addr flush dev %s", iface) != 0) {
        return -1;
    }
    if (run_command("ip link set %s up", iface) != 0) {
        return -1;
    }
    if (run_command("dhcpcd %s", iface) != 0) {
        return -1;
    }

    state->network_dhcp = true;
    state->network_configured = true;
    ui_message("Network", "DHCP configuration applied.");
    return 0;
}

static int configure_static(InstallerState *state)
{
    if (ensure_interface_selected(state) != 0) {
        return -1;
    }

    char ip[64];
    snprintf(ip, sizeof(ip), "%s", state->static_ip[0] ? state->static_ip : "");
    if (ui_prompt_input("Static IPv4", "IP address (e.g. 192.168.1.10)", ip, sizeof(ip), ip, false) != 0) {
        return -1;
    }

    char prefix_buf[8];
    snprintf(prefix_buf, sizeof(prefix_buf), "%d", state->static_prefix);
    if (ui_prompt_input("Static IPv4", "CIDR prefix (e.g. 24)", prefix_buf, sizeof(prefix_buf), prefix_buf, false) != 0) {
        return -1;
    }
    int prefix = atoi(prefix_buf);
    if (prefix < 0 || prefix > 32) {
        ui_message("Static IPv4", "Prefix must be between 0 and 32.");
        return -1;
    }

    char gateway[64];
    snprintf(gateway, sizeof(gateway), "%s", state->static_gateway);
    if (ui_prompt_input("Static IPv4", "Default gateway", gateway, sizeof(gateway), gateway, false) != 0) {
        return -1;
    }

    char dns[128];
    snprintf(dns, sizeof(dns), "%s", state->static_dns[0] ? state->static_dns : "1.1.1.1 8.8.8.8");
    if (ui_prompt_input("Static IPv4", "DNS servers (space separated)", dns, sizeof(dns), dns, false) != 0) {
        return -1;
    }

    const char *iface = state->network_interface;

    run_command("dhcpcd -k %s >/dev/null 2>&1 || true", iface);
    if (run_command("ip link set %s down", iface) != 0) {
        return -1;
    }
    if (run_command("ip addr flush dev %s", iface) != 0) {
        return -1;
    }
    if (run_command("ip addr add %s/%d dev %s", ip, prefix, iface) != 0) {
        return -1;
    }
    if (run_command("ip link set %s up", iface) != 0) {
        return -1;
    }
    if (gateway[0]) {
        run_command("ip route del default dev %s >/dev/null 2>&1 || true", iface);
        if (run_command("ip route add default via %s dev %s", gateway, iface) != 0) {
            return -1;
        }
    }

    if (dns[0]) {
        write_text_file("/etc/resolv.conf", "");
        char dns_copy[256];
        snprintf(dns_copy, sizeof(dns_copy), "%s", dns);
        char *token = strtok(dns_copy, " ");
        while (token) {
            char line[128];
            snprintf(line, sizeof(line), "nameserver %s\n", token);
            append_text_file("/etc/resolv.conf", line);
            token = strtok(NULL, " ");
        }
    }

    snprintf(state->static_ip, sizeof(state->static_ip), "%s", ip);
    state->static_prefix = prefix;
    snprintf(state->static_gateway, sizeof(state->static_gateway), "%s", gateway);
    snprintf(state->static_dns, sizeof(state->static_dns), "%s", dns);
    state->network_dhcp = false;
    state->network_configured = true;
    ui_message("Static IPv4", "Static configuration applied.");
    return 0;
}

static int test_connectivity(void)
{
    int rc = run_command("ping -c 3 -W 2 8.8.8.8");
    ui_message("Connectivity Test", (rc == 0) ? "Ping successful." : "Ping failed.");
    return rc;
}

int network_workflow(InstallerState *state)
{
    while (1) {
        char subtitle[256];
        snprintf(subtitle, sizeof(subtitle),
                 "Interface: %s | Mode: %s",
                 state->network_interface[0] ? state->network_interface : "<none>",
                 state->network_dhcp ? "DHCP" : "Static");

        const char *items[] = {
            "Select network interface",
            "Configure via DHCP",
            "Configure static IPv4",
            "Test connectivity",
            "Back to main menu",
        };

        int choice = ui_menu("Network Configuration", subtitle, items, 5, 0);
        if (choice < 0 || choice == 4) {
            return 0;
        }

        switch (choice) {
        case 0:
            select_interface(state);
            break;
        case 1:
            configure_dhcp(state);
            break;
        case 2:
            configure_static(state);
            break;
        case 3:
            test_connectivity();
            break;
        default:
            break;
        }
    }
}
