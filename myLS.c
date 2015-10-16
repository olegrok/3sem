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

int arguments_parsing(int argc, char **argv, char *flag)
{
	if (argc < 2)
		return 0;
	int for_getopt = 0;
	while (1) {
		int option_index = 0;
		static struct option long_opt[] = {
			{"all", 0, 0, 'a'},
			{"inode", 0, 0, 'i'},
			{"numeric-uid-gid", 0, 0, 'n'},
			{"recursive", 0, 0, 'R'},
			{0, 0, 0, 0}
		};
		for_getopt =
		    getopt_long(argc, argv, "ailnR", long_opt,
				&option_index);
		if (for_getopt == -1) {
			break;
		}
		switch (for_getopt) {
		case 'a':
			flag[0] = 'a';
			break;
		case 'i':
			flag[1] = 'i';
			break;
		case 'l':
			flag[2] = 'l';
			break;
		case 'n':
			flag[3] = 'n';
			break;
		case 'R':
			flag[4] = 'R';
			break;

		case '?':
			break;

		default:
			printf
			    ("?? getopt returned character code 0%o ??\n",
			     for_getopt);
		}
	}


	return 0;

}

char *path_to_name(char *path)
{
	int len = strlen(path);
	for (; len > 0; len--)
		if (path[len] == '/')
			return &path[++len];
	return &path[len];
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

int info(char *path, char *name, const char *flag)
{
	struct stat ms = { };
	if (stat(path, &ms) == -1) {
		perror("Stat failed");
		return -1;
	}
	if (flag[1] == 'i')
		printf("%d ", (int) ms.st_ino);
	if (flag[2] == 'l' || flag[3] == 'n') {
		if (ms.st_mode & S_IFDIR)
			printf("d");
		else
			printf("-");
		bits_to_rwx(ms.st_mode);
		if (flag[3] == 'n') {
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


int directory_parse(char *path, const char *flags)
{
	char *for_concat = 0;
	int len = 0;
	DIR *dr = opendir(path);
	if (dr == NULL) {
		if (errno == ENOTDIR) {
			char *name = path_to_name(path);
			return info(path, name, flags);

		} else {
			perror("Opendir failed");
			return -1;
		}
	}
	struct dirent *curr_dir = 0;
	while (1) {
		curr_dir = readdir(dr);
		if (curr_dir == NULL)
			break;
		if (curr_dir->d_name[0] == '.' && !(flags[0] == 'a'))
			continue;
		len = strlen(path);
		for_concat =
		    (char *) calloc(len + strlen(curr_dir->d_name) + 128,
				    1);
		if (for_concat == NULL) {
			perror("Calloc failed");
			return -1;
		}
		strcpy(for_concat, path);

		if (curr_dir->d_type == DT_DIR) {
			for_concat =
			    concat_path(for_concat, curr_dir->d_name);
			info(for_concat, curr_dir->d_name, flags);
			if (strcmp(curr_dir->d_name, ".")
			    && strcmp(curr_dir->d_name, "..")) {
				if (flags[4] == 'R') {
					printf("\n%s:\n", path);
					directory_parse(for_concat, flags);
					printf("\n");
				}
			}
		} else {
			for_concat =
			    concat_path(for_concat, curr_dir->d_name);
			info(for_concat, curr_dir->d_name, flags);
		}

		free(for_concat);
		path[len] = 0;
	}

	closedir(dr);
	return 0;
}

int main(int argc, char *argv[])
{
	char flag[] = "-----";
	arguments_parsing(argc, argv, flag);

	if (argv[argc - 1][0] == '-' || argc < 2) {
		char curr_path[] = { '.', 0 };
		directory_parse(curr_path, flag);
		return 0;
	}

	int i = 1;
	for (; i < argc; i++)
		if (argv[i][0] != '-')
			break;
	for (; i < argc; i++) {
		if (argv[i][0] == '/')
			printf("\n%s:\n", argv[i]);
		directory_parse(argv[i], flag);
	}
	printf("\n");
	return 0;
}
