#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int sem_info(int boatid, int trapid)
{
	/*printf("Значение boatid = %d\n",
	   semctl(boatid, 0, GETVAL));
	   printf("Значение trapid[0] = %d\n",
	   semctl(trapid, 0, GETVAL));
	   printf("Значение trapid[1] = %d\n",
	   semctl(trapid, 1, GETVAL)); */

	return 0;
}

int Titanic(int boatid, int trapid, int places, int people)
{

	printf("Titanic   [%d]: Cоздан\n", getpid());
	struct sembuf sem = { 0, 0, 0 };
	while (1) {
		//sem = {0, places, 0};         //новая поездка
		sem.sem_num = 0;
		sem.sem_op = places;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		printf("Titanic   [%d]: Новая поездка\n",
		       getpid());

		//sem = { 0, 1, 0 }             //трап на вход открыт
		sem.sem_num = 0;
		sem.sem_op = 1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Titanic   [%d]: Открыл вход на трап\n",
		     getpid());
		sem_info(boatid, trapid);
		//sem = {0, 0, 0};              //свободных мест больше нет
		sem.sem_num = 0;
		sem.sem_op = 0;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		printf
		    ("Titanic   [%d]: Свободные места закончились\n",
		     getpid());

		//sem = {0, -1, 0};             //трап закрыт
		/*sem.sem_num = 0;
		   sem.sem_op = -1;
		   sem.sem_flg = 0;
		   semop(trapid, &sem, 1); */
		printf
		    ("Titanic   [%d]: Закрыл вход на трап\n",
		     getpid());

		printf("Titanic   [%d]: Катаю\n", getpid());
		//катание
		//sem = {0, places, 0}          //Приехали
		sem.sem_num = 0;
		sem.sem_op = places;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		//sem_info(boatid, trapid);
		//sem = {1, 1, 0};              //трап на выход открыт
		sem.sem_num = 1;
		sem.sem_op = 1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Titanic   [%d]: Выход по трапу открыт\n",
		     getpid());

		//sem = {0, 0, 0};
		sem.sem_num = 0;
		sem.sem_op = 0;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);	//корабль пуст
		printf("Titanic   [%d]: Корабль пуст\n",
		       getpid());
		//sem = {1, -1, 0};

		sem.sem_num = 1;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);	//трап на выход закрыт
		printf
		    ("Titanic   [%d]: Закрыл выход по трапу\n",
		     getpid());
		printf
		    ("Titanic   [%d]: Осталось покатать %d человек\n",
		     getpid(), semctl(people, 0, GETVAL));
		if (semctl(people, 0, GETVAL) == 0) {
			printf("Titanic   [%d]: Утонул\n", getpid());
			exit(0);
		}
	}

	exit(0);
	return 0;
}


int passengers(int boatid, int trapid, int people)
{
	int satisfaction = 0;
	struct sembuf sem = { 0, 0, 0 };
	printf("Passanger [%d]: Cоздан\n", getpid());
	//sem_info(boatid, trapid);
	while (1) {
		if (semctl(people, 0, GETVAL) == 0)
			exit(0);
		//sem = { 0, -1, 0 };           //зайти на трап
		sem.sem_num = 0;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		if (semop(trapid, &sem, 1) < 0)
			exit(0);
		printf("Passanger [%d]: Зашел на трап\n",
		       getpid());
		sem_info(boatid, trapid);

		//sem = {0, -1, 0};              //на корабле +1
		sem.sem_num = 0;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);

		if (semctl(boatid, 0, GETVAL) > 0) {
			sem_info(boatid, trapid);
			//sem = {0, 1, 0};      //выйти с трапа
			sem.sem_num = 0;
			sem.sem_op = 1;
			sem.sem_flg = 0;
			semop(trapid, &sem, 1);
			//sem_info(boatid, trapid);
		}
		printf
		    ("Passanger [%d]: Освободил трап\n",
		     getpid());
		//sem_info(boatid, trapid);
		/*//sem = {0, -1, 0};              //на корабле +1
		   sem.sem_num = 0;
		   sem.sem_op = -1;
		   sem.sem_flg = 0;
		   semop(boatid, &sem, 1); */
		printf("Passanger [%d]: На корабле\n", getpid());

		sem_info(boatid, trapid);
		satisfaction++;
		if (satisfaction == 1) {
			//sem = {0, -1, 0};
			sem.sem_num = 0;
			sem.sem_op = -1;
			sem.sem_flg = 0;
			printf("--people\n");
			semop(people, &sem, 1);
		}
		//sem_info(boatid, trapid);
		//Катается
		//printf("Passanger [%d]: Катаюсь\n", getpid());
		//sem = {1, -1, 0};             //выйти на трап
		sem.sem_num = 1;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Passanger [%d]: Спускается по трапу\n",
		     getpid());
		//sem = {1, 1, 0};              //освободить трап
		sem.sem_num = 1;
		sem.sem_op = 1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Passanger [%d]: Трап освобожден\n",
		     getpid());
		//sem = {0, -1, 0};             //окончательно покинул кораболь      
		sem.sem_num = 0;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		printf("Passanger [%d]: Покинул корабль\n",
		       getpid());
		printf
		    ("Passanger [%d]: Удовлетворен уже %d раз\n",
		     getpid(), satisfaction);
		sem_info(boatid, trapid);
	}

	return 0;
}


int main()
{
	int places = 2;
	int pass = 5;
	struct sembuf sem = { 0, 0, 0 };
	int boatid = 0;
	boatid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	int trapid = 0;
	trapid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
	int people = 0;
	people = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);


	if (places > pass)
		places = pass;
	//sem = {0, pass, 0};
	sem.sem_num = 0;
	sem.sem_op = pass;
	sem.sem_flg = 0;
	semop(people, &sem, 1);

	int TitanicPid = fork();
	if (!TitanicPid)
		Titanic(boatid, trapid, places, people);
	int i = 0;
	for (; i < pass; i++) {
		if (!fork()) {
			passengers(boatid, trapid, people);
		}
	}

	waitpid(TitanicPid, 0, 0);
	semctl(boatid, 1, IPC_RMID);
	semctl(trapid, 2, IPC_RMID);
	for (i = 0; i < pass; i++)
		wait(0);
	printf("На пляже обнаружили %d трупов\n",
	       pass);
	semctl(people, 1, IPC_RMID);
	return 0;

}
