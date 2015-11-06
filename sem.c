#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
/*
Несколько слов об алгоритме:

Используемые семафоры:
boatid - количество свободных мест на корабле
trapid[0] - отвечает за вход на корабль	
trapid[1] - отвечает за выход с корабля
people - отвечает за то, чтобы каждый человек покатался

Функции:
Titanic:
	while(1){
	boatid = places				;свободные места на корабле появляются
	trapid[0]++				;открытие трапа
	boatid ? 0				;свободные места закончились?
	
	boatid = places				;на корабле places человек
	trapid[1]++				;открыть трап
	boatid ? 0				;корабль пуст
	trapid[1]--				;трап закрыт
	
	if(people == 0)				;все человечки покатались
		break;				;сжечь трап и утопить корабль

passenger:
	while(1){
	if(people == 0)
		break;				;если корабль сожжен, уйти с пляжа
	trapid[0]--				;зайти на трап
	boatid--				;занять место на корабле
	if(boatid > 0)
		trapid[0]++			;если свободные места есть, то трап будет освобожден
	trapid[1]--				;спуститься по трапц
	trapid[1]++				;освободить трап
	boatid--				;место на корабле освободилось
	}

Условие завершения программы:
Все люди покатались на корабле минимум один раз
*/
int Titanic(int boatid, int trapid, int places, int people)
{

	printf("Titanic   [%d]: Cоздан\n", getpid());
	struct sembuf sem = { 0, 0, 0 };
	while (1) {
		//новая поездка
		sem.sem_num = 0;
		sem.sem_op = places;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		printf("Titanic   [%d]: Новая поездка\n",
		       getpid());

		//трап на вход открыт
		sem.sem_num = 0;
		sem.sem_op = 1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Titanic   [%d]: Открыл вход на трап\n",
		     getpid());
		//свободных мест больше нет
		sem.sem_num = 0;
		sem.sem_op = 0;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		printf
		    ("Titanic   [%d]: Свободные места закончились\n",
		     getpid());

		//трап закрыт
		printf
		    ("Titanic   [%d]: Закрыл вход на трап\n",
		     getpid());

		printf("Titanic   [%d]: Катаю\n", getpid());
		//катание
		//Приехали
		sem.sem_num = 0;
		sem.sem_op = places;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		//трап на выход открыт
		sem.sem_num = 1;
		sem.sem_op = 1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Titanic   [%d]: Выход по трапу открыт\n",
		     getpid());


		sem.sem_num = 0;
		sem.sem_op = 0;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);	//корабль пуст
		printf("Titanic   [%d]: Корабль пуст\n",
		       getpid());

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


int passenger(int boatid, int trapid, int people)
{
	int satisfaction = 0;
	struct sembuf sem = { 0, 0, 0 };
	printf("Passanger [%d]: Cоздан\n", getpid());
	while (1) {
		if (semctl(people, 0, GETVAL) == 0)
			exit(0);
		//зайти на трап
		sem.sem_num = 0;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		if (semop(trapid, &sem, 1) < 0)
			exit(0);
		printf("Passanger [%d]: Зашел на трап\n",
		       getpid());

		//на корабле +1
		sem.sem_num = 0;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);

		if (semctl(boatid, 0, GETVAL) > 0) {
		//выйти с трапа
			sem.sem_num = 0;
			sem.sem_op = 1;
			sem.sem_flg = 0;
			printf("Passanger [%d]: Освободил трап\n",
		     getpid());
			semop(trapid, &sem, 1);
		}
		else
			printf("Passanger [%d]: Освободил трап\n",
		     getpid());
		printf("Passanger [%d]: На корабле\n", getpid());

		satisfaction++;
		if (satisfaction == 1) {
			sem.sem_num = 0;
			sem.sem_op = -1;
			sem.sem_flg = 0;
			semop(people, &sem, 1);
		}
		//Катается
		//выйти на трап
		sem.sem_num = 1;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(trapid, &sem, 1);
		printf
		    ("Passanger [%d]: Спускается по трапу\n",
		     getpid());
		//освободить трап
		sem.sem_num = 1;
		sem.sem_op = 1;
		sem.sem_flg = 0;
		printf("Passanger [%d]: Сошел с трапа. Покинул корабль\n",
		       getpid());  
		semop(trapid, &sem, 1);   
		sem.sem_num = 0;
		sem.sem_op = -1;
		sem.sem_flg = 0;
		semop(boatid, &sem, 1);
		
		printf
		    ("Passanger [%d]: Катался уже %d раз\n",
		     getpid(), satisfaction);
	}

	return 0;
}


int main(int argc, char* argv[])
{
	if(argc < 3){
		printf("Неверное количество аргументов\n");
		return -1;
	}
	int places = atoi(argv[1]);
	int pass = atoi(argv[2]);
	printf("Количество мест в корабле: %d\nКоличество отдыхающих на пляже: %d\n", places, pass);
	struct sembuf sem = { 0, 0, 0 };
	int boatid = 0;
	boatid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
	int trapid = 0;
	trapid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
	int people = 0;
	people = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);


	if (places > pass)
		places = pass;
	
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
			passenger(boatid, trapid, people);
		}
	}

	waitpid(TitanicPid, 0, 0);
	semctl(boatid, 1, IPC_RMID);
	semctl(trapid, 2, IPC_RMID);
	for (i = 0; i < pass; i++)
		wait(0);
	printf("Пляж закрылся. Люди разошлись\n");
	semctl(people, 1, IPC_RMID);
	return 0;

}
