// SPDX-License-Identifier: MIT
// Copyright (c) 2024 Maxim Mikityanskiy
// `setcap CAP_SYS_ADMIN+ep jumpns` after compiling.

#define _GNU_SOURCE
#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/statvfs.h>

#define ERR_FAILURE 1
#define ERR_USAGE 2
#define ERR_PERM 3

#define PATH_CONFIG "/etc/jumpns/net/"
#define PATH_NS "/run/netns/"

int main(int argc, char *argv[])
{
	if (argc <= 2) {
		fprintf(stderr, "usage: jumpns <netns name> <executable> [<parameters...>]\n");
		return ERR_USAGE;
	}

	char const *netns_name = argv[1];

	char path[PATH_MAX];
	if (strlcpy(path, PATH_CONFIG, sizeof(path)) >= sizeof(path) ||
	    strlcat(path, netns_name, sizeof(path)) >= sizeof(path)) {
		fprintf(stderr, "netns name is too long\n");
		return ERR_USAGE;
	}
	if (access(path, R_OK) != 0) {
		fprintf(stderr, "access denied: %s is not readable by the current user\n", path);
		return ERR_PERM;
	}

	if (strlcpy(path, PATH_NS, sizeof(path)) >= sizeof(path) ||
	    strlcat(path, netns_name, sizeof(path)) >= sizeof(path)) {
		fprintf(stderr, "netns name is too long\n");
		return ERR_USAGE;
	}
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return ERR_FAILURE;
	}
	if (setns(fd, CLONE_NEWNET) != 0) {
		perror("setns");
		return ERR_FAILURE;
	}
	close(fd);

	// Remount /sys, otherwise it will continue exposing netdevs from the
	// old namespace under /sys/class/net. Unshare a mount namespace to
	// avoid propagating this remount to the original namespace.

	if (unshare(CLONE_NEWNS) != 0) {
		perror("unshare");
		return ERR_FAILURE;
	}
	if (mount("", "/", "none", MS_SLAVE | MS_REC, NULL) != 0) {
		perror("mount");
		return ERR_FAILURE;
	}

	struct statvfs statvfs_buf;
	if (statvfs("/sys", &statvfs_buf) == 0) {
		if (umount2("/sys", MNT_DETACH) < 0) {
			perror("umount");
			return ERR_FAILURE;
		}
		if (mount("sysfs", "/sys", "sysfs", MS_NOSUID | MS_NODEV | MS_NOEXEC, NULL) != 0) {
			perror("mount");
			return ERR_FAILURE;
		}
	}

	execvp(argv[2], argv + 2);

	return ERR_FAILURE;
}
