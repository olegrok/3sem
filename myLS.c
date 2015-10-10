#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


char* concat_path(char* path, char* name)
{
	int len = strlen(path);
	if(path[len - 1] == '/')
		path[len - 1] = 0;
	path = strcat(path, "/");
	path = strcat(path, name);
	printf("Result: %s\n", path);	
	return path;


}
void directory_parse(char* path) 
{
	DIR* dr = opendir(path);
	char* buf = (char*)calloc(sizeof(char), 256);
	struct dirent* curr_dir = (struct dirent*)calloc(1, sizeof(struct dirent));
	strcpy(buf, path);
	while(curr_dir = readdir(dr))
        {
		if(curr_dir -> d_type == DT_DIR){
			printf("DIR: ");
                	printf("%s\n", curr_dir -> d_name);
			strcpy(buf, path);
			printf("buf = %s\n", buf);
			buf = concat_path(buf, curr_dir -> d_name);
		}
		else
			printf("%s\n", curr_dir -> d_name);

		
        }
	free(buf);
	closedir(dr);
	free(curr_dir);
}
int main(int argc, char *argv[])
{
	int fd = 0;
	struct stat ms = {};
	char time[64] = {};
	struct dirent* curr_dir = (struct dirent*)calloc(1, sizeof(struct dirent));
	printf("Name: %s\n",argv[1]);
	stat(argv[1], &ms);
	printf("Inode number: %d\n", ms.st_ino); 
	printf("UID: %d\n", ms.st_uid);
	printf("GID %d\n", ms.st_gid);
	printf("Protection %o\n", ms.st_mode & 0777);
	
	char* buf = (char*)calloc(sizeof(char), 256);
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
//	closedir(dr);
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
