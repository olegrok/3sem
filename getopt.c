#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int arguments_parsing(int argc, char **argv)
{
	if(argc < 2)
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
		switch(for_getopt) {
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

int main(int argc, char **argv)
{
/*	if(argc < 2)
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
		switch(for_getopt) {
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
			     c);

		}
	}
*/
	printf("key = %d\n", arguments_parsing(argc, argv));

	return 0;
}
