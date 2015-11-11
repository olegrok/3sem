#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

void change_point(int* point){
	if(*point == 0)
		(*point)++;
	else
		(*point)--;
}
struct lift_options_struct{
		int id;
		int key_in;
		int key_out;
		int door_in;
		int door_out;
		int border;
	};
struct pass_options_struct{
		int id;
		int lift_num;
		int key_in;
		int key_out;
		int door_in;
		int door_out;
		int* places;
		int border;
	};

int P(int semid, int id){
	struct sembuf sem = {id, -1, 0};
	if(semop(semid, &sem, 1) < 0)
			return -1;
	return 0;
}
int V(int semid, int id){
	struct sembuf sem = {id, 1, 0};
	if(semop(semid, &sem, 1) < 0)
			return -1;
	return 0;
}

void Passenger(struct pass_options_struct pass_options, int* non_zombie){
	int id = pass_options.id;
	int lift_num = pass_options.lift_num;
	int key_up = pass_options.key_in;
	int key_down = pass_options.key_out;
	int door_in = pass_options.door_in;
	int door_out = pass_options.door_out;
	int* places = pass_options.places;
	int border = pass_options.border;

	int my_lift = id % lift_num;
	int pass_point = 0;
	int key = 0;
	int move = 0;
	printf("Pass[%d]: 	Прибыл на отдых %d\n", getpid(), my_lift);
	while(1){
		if(move == 5){
			(*non_zombie)--;
			printf("Pass[%d]: 	Отдохнул. RIP\n", getpid());
			exit(0);
		}
		if(pass_point == 0)
			key = key_up;
		else
			key = key_down;

		P(key, my_lift);
		P(door_in, my_lift);
		printf("Pass[%d]: 	Захожу в лифт %d\n", getpid(), my_lift);
		places[my_lift]--;
		printf("Pass[%d]: 	Зашел в лифт %d\n", getpid(), my_lift);
		V(door_in, my_lift);
		if(places[my_lift] > 0)
			V(border, my_lift);
		V(key, my_lift);
		P(door_out, my_lift);
		printf("Pass[%d]: 	Выхожу из лифта %d\n", getpid(), my_lift);
		printf("Pass[%d]: 	Вышел из лифта %d\n", getpid(), my_lift);
		V(door_out, my_lift);
		places[my_lift]--;
		change_point(&pass_point);
		move++;
	}



}

void Lift(int* places, struct lift_options_struct lift_options, int* non_zombie){
	int id = lift_options.id;
	int key_up = lift_options.key_in;
	int key_down = lift_options.key_out;
	int door_in = lift_options.door_in;
	int door_out = lift_options.door_out;
	int border = lift_options.border;
	int key = 0;
	int point = 0;
	printf("Lift[%d]:	Установлен в санатории\n", id);
	while(1){
		if((*non_zombie) == 0)
			exit(0);
		if(point == 0)
			key = key_up;
		else
			key = key_down;
		places[id] = 10;
		printf("Lift[%d]:	Открыл двери [вход]\n", id);
		V(border, id);
		V(key, id);
		V(door_in, id);
		sleep(1);
		P(key, id);
		P(door_in, id);
		if(places > 0)
			P(border, id);
		printf("Lift[%d]:	Закрыл двери [вход]\n", id);
		change_point(&point);
		places[id] = 10 - places[id];
		printf("Lift[%d]:	Изменил своё положение\n", id);
		printf("Lift[%d]:	Открыл двери [выход]\n", id);
		V(door_out, id);
		while(places[id] != 0);
		P(door_out, id);
		printf("Lift[%d]:	Закрыл двери [выход]\n", id);
		printf("Lift[%d]:	Пуст\n", id);
	}


}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("There is no arguments\n");
		return -1;
	}
	int i = 0;
	int pass_num = atoi(argv[1]);
	int lift_num = atoi(argv[2]);
	if(pass_num < 0 || lift_num < 0){
		printf("You entered negative quantity of  passangers or lifts\n");
		return -1;
	}

//sems
	int key_in  = semget(IPC_PRIVATE, lift_num, IPC_CREAT | 0666);
	int key_out = semget(IPC_PRIVATE, lift_num, IPC_CREAT | 0666);
	int door_in  = semget(IPC_PRIVATE, lift_num, IPC_CREAT | 0666);
	int door_out = semget(IPC_PRIVATE, lift_num, IPC_CREAT | 0666);
	int border = semget(IPC_PRIVATE, lift_num, IPC_CREAT | 0666);

//shm
	int places_id = shmget(IPC_PRIVATE, sizeof(int) * lift_num, IPC_CREAT | 0666);
	int non_zombie_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	int* places = shmat(places_id, 0, 0);
	int* non_zombie = shmat(non_zombie_id, 0, 0);
	(*non_zombie) = pass_num;

	struct lift_options_struct lift_options = {0, key_in, key_out, door_in, door_out, border};
	struct pass_options_struct pass_options = {0, lift_num, key_in, key_out, door_in, door_out, places, border};

	for(i = 0; i < lift_num; i++){
		places[i] = 0;
	}

//initialization
	for(i = 0; i < lift_num; i++){
		lift_options.id = i;
		if(fork() == 0){
			Lift(places, lift_options, non_zombie);
			exit(0);
		}
	}
	for(i = 0; i < pass_num; i++){
		pass_options.id = i;
		if(fork() == 0){
			Passenger(pass_options, non_zombie);
			exit(0);
		}
	}
	for(i = 0; i < lift_num; i++)
		wait(0);
	for(i = 0; i < pass_num; i++)
		wait(0);
//sem
	semctl(key_in, lift_num, IPC_RMID);
	semctl(key_out, lift_num, IPC_RMID);
	semctl(door_in, lift_num, IPC_RMID);
	semctl(door_out, lift_num, IPC_RMID);
	semctl(border, lift_num, IPC_RMID);
//shm
	shmdt(places);
	shmdt(non_zombie);
	shmctl(places_id, IPC_RMID, NULL);
	shmctl(non_zombie_id, IPC_RMID, NULL);
	return 0;
}
