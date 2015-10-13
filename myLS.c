#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <math.h>

char* bits_to_rwx(int bits)
{
    int bit = 0;
    int i = 0;
    for(; i < 3; i++)
    {
        bit = (bits & 0777) & (7 * (int)pow(8, 2 - i));
        bit >>= 3 * (2 - i);
        switch(bit)
        {
            case 0: printf("---");break;
            case 1: printf("--x");break;
            case 2: printf("-w-");break;
            case 3: printf("-wx");break;
            case 4: printf("r--");break;
            case 5: printf("r-x");break;
            case 6: printf("rw-");break;
            case 7: printf("rwx");break;

        }
    }
    printf("   ");
    return NULL;

}

int arguments_parsing(int argc, char **argv)
{
	if (argc < 2)
		return 0;
	int for_getopt = 0;
	int flags = 0;
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
		if (for_getopt == -1)
			break;
		switch (for_getopt) {
		case 'a':
			flags |= 1;
			break;
		case 'i':
			flags |= 2;
			break;
		case 'l':
			flags |= 4;
			break;
		case 'n':
			flags |= 8;
			break;
		case 'R':
			flags |= 16;
			break;

		case '?':
			break;

		default:
			printf
			    ("?? getopt returned character code 0%o ??\n",
			     for_getopt);

		}
	}


	return flags;

}



char *concat_path(char *path, char *name)
{
	int len = strlen(path);
	if (path[len - 1] == '/')
		path[len - 1] = 0;
	path = strcat(path, "/");
	path = strcat(path, name);
	return path;


}

void info(char* path, char name[], int flag, unsigned char type)
{	
	struct stat ms = { };
	struct dirent *curr_dir =
	    (struct dirent *) calloc(1, sizeof(struct dirent));
	stat(path, &ms);
	
	if(flag & 2)
		printf("%d   ", (int)ms.st_ino);
	if(type == DT_DIR)
		printf("d");
	else
		printf("-");
	bits_to_rwx(ms.st_mode);
	if(flag & 8){	
		printf("%d  ", ms.st_uid);
		printf("%d  ", ms.st_gid);
	}
	printf("%s  ", name);
	printf("%s", ctime(&ms.st_mtime));
	
	free(curr_dir);
}


void directory_parse(char *path, int flags)
{
	int len = 0;
	DIR *dr = opendir(path);
	char *buf = (char *) calloc(sizeof(char), 256);
	struct dirent *curr_dir =
	    (struct dirent *) calloc(1, sizeof(struct dirent));
	strcpy(buf, path);
	while (curr_dir = readdir(dr)) {
		if (curr_dir->d_type == DT_DIR) {
			printf("DIR: %s\n", curr_dir->d_name);
			info(path, curr_dir->d_name, flags, curr_dir->d_type);
			len = strlen(path);
			path = concat_path(path, curr_dir->d_name);
			if (curr_dir->d_name[0] != '.') {
				if(flags & 16){
					printf("\nI'm in DIR %s\n",
				       curr_dir->d_name);
					directory_parse(path, flags);
					printf("\nExit  %s\n", curr_dir->d_name);
				}
			}
			path[len] = 0;
		} else
			{
				info(path, curr_dir->d_name, flags, curr_dir->d_type);
			}


	}
	free(buf);
	closedir(dr);
	free(curr_dir);
}




int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		printf("There is no arguments\n");
		return 0;
	}

	int flags = 0;
	flags = arguments_parsing(argc, argv);
	int fd = 0;
	struct stat ms = { };
	struct dirent *curr_dir =
	    (struct dirent *) calloc(1, sizeof(struct dirent));
	printf("Name: %s\n", argv[argc - 1]);
	stat(argv[argc - 1], &ms);
	printf("Inode number: %d\n", (int)ms.st_ino);
	printf("UID: %d\n", ms.st_uid);
	printf("GID %d\n", ms.st_gid);
	printf("Protection %o\n", ms.st_mode & 0777);
	
	//for_time = ctime(&ms.st_atim);
	printf("Last status change:       %s", ctime(&ms.st_ctime));
        printf("Last file access:         %s", ctime(&ms.st_atime));
        printf("Last file modification:   %s", ctime(&ms.st_mtime));


	char *buf = (char *) calloc(sizeof(char), 256);
	strcpy(buf, argv[argc - 1]);
	//DIR* dr = opendir(argv[1]);


	/*while(curr_dir = readdir(dr))
	   {
	   printf("%s\n", curr_dir -> d_name);
	   }
	 */

	directory_parse(buf, flags);


	free(buf);

	free(curr_dir);
//      closedir(dr);
	return 0;
}

