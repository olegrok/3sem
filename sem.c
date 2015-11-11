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
boatid[0] - оставшиеся места для входа
boatid[1] - оставшиеся места для выхода
trapid[0] - отвечает за вход на корабль
trapid[1] - отвечает за выход с корабля
people - отвечает за то, чтобы каждый человек покатался

Функции:
Titanic:
	while(1){
	boatid = places			;свободные места на корабле появляются
	trapid[0]++				;открытие трапа
	boatid[0] ? 0			;свободные места закончились?
	
	boatid[0] = places		;на корабле places человек
	trapid[1]++				;открыть трап
	boatid[1] ? 0			;корабль пуст
	trapid[1]--				;трап закрыт
	
	if(people == 0)			;все человечки покатались
		break;				;сжечь трап и утопить корабль

passenger:
	while(1){
	if(people == 0)
		break;				;если корабль сожжен, уйти с пляжа
	trapid[0]--				;зайти на трап
	boatid[0]--				;занять место на корабле
	if(boatid[0] > 0)
		trapid[0]++			;если свободные места есть, то трап будет освобожден
	trapid[1]--				;спуститься по трапц
	trapid[1]++				;освободить трап
	boatid[1]--				;место на корабле освободилось
	}

Условие завершения программы:
Все люди покатались на корабле минимум один раз
*/
int mysemop(int id, int num, int op){
	struct sembuf sem ={num, op, 0};
	return semop(id, &sem, 1);
}
int Titanic(int boatid, int trapid, int places, int people)
{

	printf("Titanic   [%d]: Cоздан\n", getpid());
	while (1) {
		//новая поездка
		printf("Titanic   [%d]: Новая поездка\n",
		       getpid());
		if(mysemop(boatid, 0, places) < 0)
			exit(-1);


		//трап на вход открыт
		printf
		    ("Titanic   [%d]: Открыл вход на трап\n",
		     getpid());
		if(mysemop(trapid, 0, 1) < 0)
			exit(-1);

		//свободных мест больше нет
		if(mysemop(boatid, 0, 0) < 0)
			exit(-1);
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
		if(mysemop(boatid, 1, places) < 0)
			exit(-1);


		//трап на выход открыт
		printf
		    ("Titanic   [%d]: Выход по трапу открыт\n",
		     getpid());
		if(mysemop(trapid, 1, 1) < 0)
			exit(-1);
		//корабль пуст
		if(mysemop(boatid, 1, 0) < 0)
			exit(-1);
		printf("Titanic   [%d]: Корабль пуст\n",
		       getpid());
		//трап на выход закрыт
		if(mysemop(trapid, 1, -1) < 0)
			exit(-1);
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
}


int passenger(int boatid, int trapid, int people)
{
	int satisfaction = 0;
	printf("Passanger [%d]: Cоздан\n", getpid());
	while (1) {
		//зайти на трап
		if(mysemop(trapid, 0, -1) < 0)
			exit(0);
		printf("Passanger [%d]: Зашел на трап\n",
		       getpid());

		//на корабле +1
		printf("Passanger [%d]: Освободил трап\n",
		     getpid());
		printf("Passanger [%d]: На корабле\n", getpid());

		mysemop(boatid, 0, -1);
		if (semctl(boatid, 0, GETVAL) > 0) {
		//выйти с трапа
			mysemop(trapid, 0, 1);
		}
		satisfaction++;
		if (satisfaction == 1) {
			mysemop(people, 0, -1);
		}


		//Катается
		//выйти на трап
		mysemop(trapid, 1, -1);
		printf
		    ("Passanger [%d]: Спускается по трапу\n",
		     getpid());
		printf("Passanger [%d]: Сошел с трапа\n",
		       getpid());
		printf("Passanger [%d]: Покинул корабль\n",
		       getpid());
		printf
		    ("Passanger [%d]: Катался уже %d раз\n",
		     getpid(), satisfaction);
		//освободить трап
		mysemop(trapid, 1, 1);
		mysemop(boatid, 1, -1);
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
	if(places <= 0 || pass <= 0){
		printf("Введены неположительные аргументы\n");
		return -1;
	}
	printf("Количество мест в корабле: %d\nКоличество отдыхающих на пляже: %d\n", places, pass);
	//struct sembuf sem = { 0, 0, 0 };
	int boatid = 0;
	boatid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
	int trapid = 0;
	trapid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
	int people = 0;
	people = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);


	if (places > pass)
		places = pass;
	mysemop(people, 0, pass);
	int PassPid = 0;
	int TitanicPid = fork();
	if (!TitanicPid)
		Titanic(boatid, trapid, places, people);
	int i = 0;
	for (; i < pass; i++) {
		PassPid = fork();
		if(PassPid < 0){
			perror("Fork failed");
			semctl(boatid, 2, IPC_RMID);
			semctl(trapid, 2, IPC_RMID);
			semctl(people, 1, IPC_RMID);
			exit(-1);
		}
		if (!PassPid) {
			passenger(boatid, trapid, people);
		}
	}

	waitpid(TitanicPid, 0, 0);
	semctl(boatid, 2, IPC_RMID);
	semctl(trapid, 2, IPC_RMID);
	for (i = 0; i < pass; i++)
		wait(0);
	printf("Пляж закрылся. Люди разошлись\n");
	semctl(people, 1, IPC_RMID);
	return 0;

}
