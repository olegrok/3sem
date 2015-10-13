#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>


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

void directory_parse(char *path)
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
			len = strlen(path);
			path = concat_path(path, curr_dir->d_name);
			if (curr_dir->d_name[0] != '.') {
				printf("\nI'm in DIR %s\n",
				       curr_dir->d_name);
				directory_parse(path);
				printf("\nExit  %s\n", curr_dir->d_name);
			}
			path[len] = 0;
		} else
			printf("FILE %s\n", curr_dir->d_name);


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
	char *for_time = 0;
	struct dirent *curr_dir =
	    (struct dirent *) calloc(1, sizeof(struct dirent));
	printf("Name: %s\n", argv[1]);
	stat(argv[1], &ms);
	printf("Inode number: %d\n", ms.st_ino);
	printf("UID: %d\n", ms.st_uid);
	printf("GID %d\n", ms.st_gid);
	printf("Protection %o\n", ms.st_mode & 0777);
	
	//for_time = ctime(&ms.st_atim);
	printf("Last status change:       %s", ctime(&ms.st_ctime));
        printf("Last file access:         %s", ctime(&ms.st_atime));
        printf("Last file modification:   %s", ctime(&ms.st_mtime));


	char *buf = (char *) calloc(sizeof(char), 256);
	strcpy(buf, argv[1]);
	//DIR* dr = opendir(argv[1]);


	/*while(curr_dir = readdir(dr))
	   {
	   printf("%s\n", curr_dir -> d_name);
	   }
	 */

	directory_parse(buf);


	free(buf);

	free(curr_dir);
//      closedir(dr);
	return 0;
}



/*
struct stat {
               dev_t     st_dev;          ID of device containing file 
               ino_t     st_ino;          inode number 
               mode_t    st_mode;         protection 
               nlink_t   st_nlink;        number of hard links 
               uid_t     st_uid;          user ID of owner 
               gid_t     st_gid;          group ID of owner 
               dev_t     st_rdev;         device ID (if special file) 
               off_t     st_size;         total size, in bytes 
               blksize_t st_blksize;      blocksize for filesystem I/O 
               blkcnt_t  st_blocks;       number of 512B blocks allocated 
               Since Linux 2.6, the kernel supports nanosecond
                  precision for the following timestamp fields.
                  For the details before Linux 2.6, see NOTES. 
               struct timespec st_atim;   time of last access 
               struct timespec st_mtim;   time of last modification
               struct timespec st_ctim;   time of last status change 
           #define st_atime st_atim.tv_sec       Backward compatibility 
           #define st_mtime st_mtim.tv_sec
           #define st_ctime st_ctim.tv_sec
           };
*/
