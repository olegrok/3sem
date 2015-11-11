#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <libgen.h>

struct key {
	int opt_all;
	int opt_inode;
	int opt_longlist;
	int opt_numeric;
	int opt_recursive;
} key;




char *bits_to_rwx(int bits)
{
	int bit = 0;
	int i = 0;
	char *rwx[] =
	    { "---", "--x", "-w-", "-wx", "r--", "r-x", "rw-", "rwx" };
	for (; i < 3; i++) {
		bit = (bits & 0777) & (7 * (1 << (3 * (2 - i))));
		bit >>= 3 * (2 - i);
		printf("%s", rwx[bit]);
	}
	printf(" ");
	return NULL;

}

int arguments_parsing(int argc, char **argv, struct key *keys)
{
	if (argc < 2)
		return 0;
	int key_flag = 0;
	while (1) {
		int option_index = 0;
		static struct option long_opt[] = {
			{"all", 0, 0, 'a'},
			{"inode", 0, 0, 'i'},
			{"numeric-uid-gid", 0, 0, 'n'},
			{"recursive", 0, 0, 'R'},
			{0, 0, 0, 0}
		};
		key_flag =
		    getopt_long(argc, argv, "ailnR", long_opt,
				&option_index);
		if (key_flag == -1) {
			break;
		}
		switch (key_flag) {
		case 'a':
			keys->opt_all++;
			break;
		case 'i':
			keys->opt_inode++;
			break;
		case 'l':
			keys->opt_longlist++;
			break;
		case 'n':
			keys->opt_numeric++;
			break;
		case 'R':
			keys->opt_recursive++;
			break;

		case '?':
			break;

		default:
			printf
			    ("?? getopt returned character code 0%o ??\n",
			     key_flag);
		}
	}


	return 0;
}

char *concat_path(char *path, char *name)
{

	int len_path = strlen(path);
	if (path[len_path - 1] == '/')
		path[len_path - 1] = 0;
	path = strcat(path, "/");
	path = strcat(path, name);
	return path;
}

int info(char *path, char *name, const struct key *keys)
{
	struct stat ms = { };
	if (stat(path, &ms) == -1) {
		perror("Stat failed");
		return -1;
	}
	if (keys->opt_inode)
		printf("%-9d", (int) ms.st_ino);
	if (keys->opt_longlist || keys->opt_numeric) {
		if (S_ISDIR(ms.st_mode))
			printf("d");
		else
			printf("-");
		bits_to_rwx(ms.st_mode);
		if (keys->opt_numeric) {
			printf("%d ", ms.st_uid);
			printf("%d ", ms.st_gid);
		} else {
			struct group *grp = 0;
			struct passwd *usr = 0;
			grp = getgrgid(ms.st_gid);
			if (grp == NULL) {
				perror("Getgrgid filed");
				return -1;
			}
			usr = getpwuid(ms.st_uid);
			if (usr == NULL) {
				perror("Getpwuid filed");
				return -1;
			}
			printf("%s ", usr->pw_name);
			printf("%s ", grp->gr_name);
		}
		char *time = ctime(&ms.st_mtime);
		time[strlen(time) - 1] = 0;
		printf("%s ", time);
		printf("%s\n", name);
	} else {
		printf("%s\n", name);
	}
	return 0;
}


int directory_parse(char *path, const struct key *keys)
{
	char *new_str = 0;
	int len = 0;
	DIR *dr = opendir(path);
	if (dr == NULL) {
		if (errno == ENOTDIR) {
			return info(path, basename(path), keys);

		} else {
			perror("Opendir failed");
			return -1;
		}
	}
	printf("\nin %s:\n", basename(path));
	struct dirent *curr_dir = 0;
	while (1) {
		curr_dir = readdir(dr);
		if (curr_dir == NULL)
			break;
		if (curr_dir->d_name[0] == '.' && !(keys->opt_all))
			continue;
		len = strlen(path);
		new_str = calloc(len + strlen(curr_dir->d_name) + 2, 1);
		if (new_str == NULL) {
			perror("Calloc failed");
			return -1;
		}
		strcpy(new_str, path);

		if (curr_dir->d_type == DT_DIR) {
			new_str =
			    concat_path(new_str, curr_dir->d_name);
			info(new_str, curr_dir->d_name, keys);
			if (strcmp(curr_dir->d_name, ".")
			    && strcmp(curr_dir->d_name, "..")) {
				if (keys->opt_recursive) {
					directory_parse(new_str, keys);
					printf("\n");
				}
			}
		} else {
			new_str =
			    concat_path(new_str, curr_dir->d_name);
			info(new_str, curr_dir->d_name, keys);
		}

		free(new_str);
	}

	closedir(dr);
	return 0;
}

int main(int argc, char *argv[])
{
	struct key keys = { 0, 0, 0, 0, 0 };
	arguments_parsing(argc, argv, &keys);
	if (argv[argc - 1][0] == '-' || argc < 2) {
		directory_parse(".", &keys);
		return 0;
	}

	int i = optind;
	for (; i < argc; i++) {
		directory_parse(argv[i], &keys);
	}
	printf("\n");
	return 0;
}
