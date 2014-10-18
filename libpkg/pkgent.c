#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#ifndef _BSD_SOURCE
#	define _BSD_SOURCE /* for struct dirent.d_type */
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pkgent.h"

bool pkgent_register(const struct pkg_entry *entry, const char *root)
{
	char path[PATH_MAX];
	FILE *fh;
	bool ret = false;

	(void) sprintf(path, "%s"INST_DATA_DIR"/%s/entry", root, entry->name);

	fh = fopen(path, "w");
	if (NULL == fh)
		goto end;

	if (0 < fprintf(fh,
	                "%s|%s|%s|%s|%s|%s|%s\n",
	                entry->name,
	                entry->ver,
	                entry->desc,
	                entry->fname,
	                entry->arch,
	                entry->deps,
	                entry->reason))
		ret = true;

	(void) fclose(fh);

end:
	return ret;
}

bool pkgent_unregister(const char *name, const char *root)
{
	char path[PATH_MAX];

	(void) sprintf(path, "%s"INST_DATA_DIR"/%s/entry", root, name);
	if (-1 == unlink(path))
		return false;

	return true;
}

bool pkgent_parse(struct pkg_entry *entry)
{
	char *last;
	ssize_t len;

	entry->name = entry->buf;

	entry->ver = strchr(entry->name, '|');
	if (NULL == entry->ver)
		goto invalid;
	entry->ver[0] = '\0';
	++entry->ver;

	entry->desc = strchr(entry->ver, '|');
	if (NULL == entry->desc)
		goto invalid;
	entry->desc[0] = '\0';
	++entry->desc;

	entry->fname = strchr(entry->desc, '|');
	if (NULL == entry->fname)
		goto invalid;
	entry->fname[0] = '\0';
	++entry->fname;

	entry->arch = strchr(entry->fname, '|');
	if (NULL == entry->arch)
		goto invalid;
	entry->arch[0] = '\0';
	++entry->arch;

	entry->deps = strchr(entry->arch, '|');
	if (NULL == entry->deps)
		goto invalid;
	entry->deps[0] = '\0';
	++entry->deps;

	entry->reason = strchr(entry->deps, '|');
	if (NULL == entry->reason)
		last = entry->deps;
	else {
		entry->reason[0] = '\0';
		++entry->reason;
		last = entry->reason;
	}

	len = (ssize_t) strlen(last) - 1;
	if ('\n' == last[len])
		last[len] = '\0';

	return true;

invalid:
	return false;
}

bool pkgent_get(const char *name, struct pkg_entry *entry, const char *root)
{
	char path[PATH_MAX];
	int fd;
	ssize_t len;
	bool ret = false;

	(void) sprintf(path, "%s"INST_DATA_DIR"/%s/entry", root, name);
	fd = open(path, O_RDONLY);
	if (-1 == fd)
		goto end;

	len = read(fd, (void *) entry->buf, sizeof(entry->buf) - 1);
	if (0 < len) {
		entry->buf[len] = '\0';
		ret = pkgent_parse(entry);
	}

	(void) close(fd);

end:
	return ret;
}

bool pkgent_foreach(const char *root,
                    bool (*cb)(const struct pkg_entry *entry, void *arg),
                    void *arg)
{
	char path[PATH_MAX];
	struct dirent name;
	struct pkg_entry entry;
	struct dirent *namep;
	DIR *dir;
	bool ret = false;

	(void) sprintf(path, "%s"INST_DATA_DIR, root);

	dir = opendir(path);
	if (NULL == dir)
		goto end;

	do {
		if (0 != readdir_r(dir, &name, &namep))
			break;
		if (NULL == namep) {
			ret = true;
			break;
		}

		if (DT_DIR != namep->d_type)
			continue;
		if ('.' == namep->d_name[0])
			continue;

		if (false == pkgent_get(namep->d_name, &entry, root))
			break;

		if (false == cb(&entry, arg))
			break;
	} while (1);

	(void) closedir(dir);

end:
	return ret;
}
