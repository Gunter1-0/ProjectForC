#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <dirent.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

struct ListOfFile
{
	char Name[4096];
	int count;
	bool forkFlag1;
	bool forkFlag2;
	bool forkFlag3;
	bool forkFlag4;
	bool IsNameSet;
};

#define SHMNAME "/OURE_SPACE"
#define SEMNAME "/OURE_SEM"
#define SIZE_INT sizeof(int)
#define SIZE_STRUCT sizeof(struct ListOfFile)

struct List //Список с папками
{
	char file [4096];
	struct List *next;
};

struct List *Init(char file[4096])
{
	struct List *newEl;
	newEl = (struct List*)malloc(sizeof(struct List));
	if(newEl != NULL)
	{		
		strcpy(newEl->file, file);
		newEl->next = NULL;		
	}
	else printf("50 %s\n",strerror(errno));
	return newEl;
}

struct List *addFilename(struct List* Files, char file[4096])
{
		if(Files != NULL)
		{
			struct List *newEl, *point = Files;
			newEl = (struct List*)malloc(sizeof(struct List));
			if(newEl != NULL)
			{

				strcpy(newEl->file, file);
				newEl->next = NULL;
				while(point->next != NULL)point = point->next;
				point->next = newEl;
			}
			else printf("\n69 %s",strerror(errno));

		}
		else Files = Init(file);


	return Files;
}

struct List *delBegin(struct List* List)
{
    struct List *p = List;
    if(List != NULL)
    {
        if(List->next != NULL)
        {
            List = List->next;
            free(p);
        }
        else
        {
            free(List);
            List = NULL;
        }

    }
    else
    {
        printf("\n79 ERROR: %s\n", strerror(errno));
    }

	return List;
}

void StartChild(pid_t pid, bool *flag)
{
	*flag = false;
	int status;
	pid_t s = waitpid(pid, &status, 0);
	if (s == -1)
	{
		printf("\n114 %s",strerror(errno));
	}
	else if(s >= 0)
	{
		if(!WIFEXITED(status))printf("\n119 Status= %d %s\n",status, strerror(errno));
	}

}

bool ChoiseFlag(int num, bool flag1, bool flag2, bool flag3, bool flag4)
{
	if(num == 1) return flag1;
	else if(num == 2) return flag2;
	else if(num == 3) return flag3;
	else if(num == 4) return flag4;	
}

void ChangeFlagToFalse(int num, bool *flag1, bool *flag2, bool *flag3, bool *flag4)
{
	if(num == 1) *flag1 = false;
	else if(num == 2) *flag2 = false;
	else if(num == 3) *flag3 = false;
	else if(num == 4) *flag4 = false;
}

void ChangeFlagToTrue(int num, bool *flag1, bool *flag2, bool *flag3, bool *flag4)
{
	if(num == 1) *flag1 = true;
	else if(num == 2) *flag2 = true;
	else if(num == 3) *flag3 = true;
	else if(num == 4) *flag4 = true;
}


pid_t ChoisePid(int num, pid_t pid1, pid_t pid2, pid_t pid3, pid_t pid4)
{
	if(num == 1) return pid1;
	else if(num == 2) return pid2;
	else if(num == 3) return pid3;
	else if(num == 4) return pid4;
	else if(pid1>0&&pid2>0&&pid3>0&&pid4>0) return getpid();
}

int main(void)
{
	char Way[4095] = "/home/lenovo/Desktop/Колледж/EXD";// основная папка
	int count = 0;
	errno = 0; 			   			 //Глобальная переменная, в которую записывается номер ошибки
	struct dirent* DirBox; 			 //Структура, в которую записывается считанная информация
	char* name;			   			 //Название файла, которое мы извлекаем из  DirBox
	struct stat buf;	   			 //Информация о файле/каталоге
    int FileSize;             		 //Размер файла
 	pid_t pid = getpid();
	pid_t pid1;
 	pid_t pid2;
 	pid_t pid3;
 	pid_t pid4;
	bool stopflag1 = true;
	bool stopflag2 = true;
	bool stopflag3 = true;
	bool stopflag4 = true;
	struct List *Head = NULL;
	int num = 0;
	int Space = shm_open(SHMNAME, O_CREAT|O_RDWR,0600);	//Общая память
	if(Space < 0) printf("\n42 shm_open %s \n", strerror(errno));
	else
	{
		//Выделяем память
		if(ftruncate(Space,SIZE_STRUCT) == 0)
		{
			//Отражение файлов в памяти
			void *adr = mmap(NULL, SIZE_STRUCT, PROT_READ|PROT_WRITE, MAP_SHARED, Space,0);
			if(adr != MAP_FAILED)
			{
				struct ListOfFile *Child = (struct ListOfFile *) adr;
				sem_t* main_sem = sem_open(SEMNAME, O_CREAT,0600,5);
				if(main_sem != SEM_FAILED)
				{
					printf("\nHello\n");
					if(sem_wait(main_sem) == 0)
					{
						printf("\nBey\n");
						//Основная работа
						Child->count = 0;
						Child->IsNameSet = false;
						Child->forkFlag1 = true;
						Child->forkFlag2 = true;
						Child->forkFlag3 = true;
						Child->forkFlag4 = true;
						DIR *directory = opendir(Way);//Открывает файл для читения
						if (directory == NULL) printf("\n60 %s",strerror(errno));//Проверка на то, что хватило памяти для названия репозитория
						else
						{
							while ((DirBox = readdir(directory)) != NULL)//проверка на конец файла
							{

								name = DirBox -> d_name;
								char *Str = calloc(strlen(Way)+strlen(name)+2,1);
								char *Str1 = calloc(strlen(Way)+3,1);

								if(Str1 == NULL)printf("70 Memory overflov");
								else
								{
									char *Str2 = calloc(strlen(Way)+4,1);//выделенная память
									if(Str2 == NULL)printf("74 Memory overflov");
									else
									{
										strcpy(Str,Way);
										strcat(Str,"/");
										strcat(Str,name);

										strcpy(Str1,Way);
										strcat(Str1,"/");
										strcat(Str1,".");

										strcpy(Str2,Way);
										strcat(Str2,"/");
										strcat(Str2,"..");

										if (Str == NULL) printf("89 Memory overflov");
										else
										{
											if(stat(Str,&buf) == 0)
											{
												if(S_ISREG(buf.st_mode))
												{
													//This is file
													FileSize = buf.st_size/1024;
													if(FileSize > 5)
													{
														printf("Name: %s Size(Kb)%d\n", name, FileSize);
														count++;

													}

												}
												else if((S_ISDIR(buf.st_mode) !=0)&&(strcmp(Str,Str1) != 0)&&(strcmp(Str,Str2) != 0)&&(Str1 != NULL)&&(Str2 != NULL))
												{
													//идентификатор процесса или группы процессов
													if(stopflag1){ stopflag1 = false; pid1 = fork(); num = 1;}
													if((stopflag2)&&(pid1 > 0)){ stopflag2 = false; pid2 = fork(); num = 2;}
													if((stopflag3)&&(pid2 > 0)){ stopflag3 = false; pid3 = fork(); num = 3;}
													if((stopflag4)&&(pid3 > 0)){ stopflag4 = false; pid4 = fork(); num = 4;}
													pid_t p = ChoisePid(num, pid1, pid2 , pid3, pid4);
													printf("\nPID = %d, num = %d\n", p, num);													
													if(p > 0)
													{
														//Parent
														printf("\nparent PID = %d num = %d\n", getpid(), num);													
														printf("\n255 Hello Nina %s\n", Str);
														Head = addFilename(Head, Str);														
														printf("\n257 Hello Natasha\n");
													}
													else if(p == 0)
													{
														//Child
														printf("\nchild PID = %d num = %d\n", getpid(), num);													
														printf("\n267 Bool %d %d %d %d \n",Child->forkFlag1,Child->forkFlag2,Child->forkFlag3,Child->forkFlag4);
												
														printf("\n261 Hello Lena\n");
														if(ChoiseFlag(num, Child->forkFlag1, Child->forkFlag2, Child->forkFlag3, Child->forkFlag4))
														{
															printf("\n267 Bool %d %d %d %d \n",Child->forkFlag1,Child->forkFlag2,Child->forkFlag3,Child->forkFlag4);
															ChangeFlagToFalse(num, &Child->forkFlag1, &Child->forkFlag2, &Child->forkFlag3, &Child->forkFlag4);
															if(sem_wait(main_sem) == 0)//Семафор не работает
															{
																// Потомок
																count = 0;
																strcpy(Way, Head->file);
																printf("\n275 %s\n", Way);
																directory = opendir(Way);
																if (directory == NULL) printf("\n141 %s",strerror(errno));

															}
															else printf("\n147 sem_wait %s",strerror(errno));

														}
														else
														{
															//Нужно организовать передачу пути
															printf("\n281 ERROR POINT \n");
															strcpy(Child->Name, Str);
															Child->IsNameSet = true;
															printf("\n284 ERROR POINT \n");
														}

													}
													else if((p == -1)&&(errno != 0)) printf("\n162 ERROR: %s",strerror(errno));

												}
												else if((Str1 == NULL)||(Str2 == NULL)) printf("\n167 Memory overflov");

											}
											else if(errno) printf("\n170 %s %s",strerror(errno), Str);

											free(Str);
										}
										free(Str2);
									}
									free(Str1);
								}

							}
							if (errno) printf("\n305 %s %s PID = %d num = %d",strerror(errno), Way, getpid(), num);
							closedir(directory);
							printf("\n304 Hello, Fread\n");
							if(ChoisePid(num, pid1, pid2 , pid3, pid4) > 0)
							{
								printf("\n307 Hello, Mike\n");
								while (Head != NULL)
								{
									//printf("\n310 Bool %d %d %d %d \n",Child->forkFlag1,Child->forkFlag2,Child->forkFlag3,Child->forkFlag4);
									//printf("\n318 %s\n", Head->file);
									if(Child->forkFlag1||Child->forkFlag2||Child->forkFlag3||Child->forkFlag4)
									{
										printf("\n313 Hello, Mari\n");
										if((pid1 > 0)&&(Child->forkFlag1))
										{						
											printf("\n316 Hello, Dana\n");
											StartChild(pid1, &Child->forkFlag1);
											Head = delBegin(Head);
										}
										if((pid2 > 0)&&(Child->forkFlag2))
										{
											printf("\n322 Hello, Juli\n");
											StartChild(pid2, &Child->forkFlag2);
											Head = delBegin(Head);
										}
										if((pid3 > 0)&&(Child->forkFlag3))
										{ 
											printf("\n328 Hello, Anjela\n");
											StartChild(pid3, &Child->forkFlag3);
											Head = delBegin(Head);
										}
										if((pid4 > 0)&&(Child->forkFlag4))
										{
											printf("\n334 Hello, Stasy\n");
											StartChild(pid4, &Child->forkFlag4);
											Head = delBegin(Head);
										}
									}									
									if(Child->IsNameSet)
									{
										printf("\n318 ERROR POINT %s\n", Child->Name );
										Head = addFilename(Head, Child->Name);
										Child->IsNameSet = false;
										printf("\n321 ERROR POINT \n");

									}
									
								}

							}

							if(ChoisePid(num, pid1, pid2 , pid3, pid4) == 0)//Fix a parent process
							{
								Child->count += count;
								ChangeFlagToTrue(num, &Child->forkFlag1, &Child->forkFlag2, &Child->forkFlag3, &Child->forkFlag4);
							}
							else if((ChoisePid(num, pid1, pid2 , pid3, pid4)>0)&&(Child->count != 0))
 							{
								count+=Child->count;
							}

							if(sem_post(main_sem)!=0) printf("\n189 sem_post in parent %s\n", strerror(errno));

						}
						//Все освобождаем
						if (sem_close(main_sem) != 0)
						{
							printf("\n195 sem_close %s\n",strerror(errno));
						}

					}
					else
					{
						printf("\n201 sem_wait %s",strerror(errno));
					}


				}
				else
				{
					printf("\n208 sem_open %s\n",strerror(errno));
				}

				//Все освобождаем
				if(munmap(adr, SIZE_STRUCT) != 0&&(pid>0)) printf("\n212 munmap = %s", strerror(errno));
				if(close(Space) != 0&&(pid>0))printf("\n213 close = %s", strerror(errno));
				if(shm_unlink(SHMNAME)&&(pid>0))printf("\n214 unlink = %s PID = %d", strerror(errno), pid);

			}
			else
			{
				printf("\n218 mmap %s\n",strerror(errno));
			}

		}
		else
		{
			printf("\n224 mmap %s\n",strerror(errno));
		}

	}
	printf("\n COUNT = %d\n", count);
	return 0;
}

/*
							if(ChoisePid(num, pid1, pid2 , pid3, pid4) > 0)
							{
								printf("\nHello\n");
								while (Head != NULL)
								{
									if((pid1 > 0)&&(Child->forkFlag1)){ StartChild(pid1,status, &Child->forkFlag1);}
									else if((pid2 > 0)&&(Child->forkFlag2)){ StartChild(pid2,status, &Child->forkFlag2);}
									else if((pid3 > 0)&&(Child->forkFlag3)){ StartChild(pid3,status, &Child->forkFlag3);}
									else if((pid4 > 0)&&(Child->forkFlag4)){ StartChild(pid4,status, &Child->forkFlag4);}
									if(Child->IsNameSet)
									{
										Head = addFilename(Head, Child->Name);
										Child->IsNameSet = false;
									}
									Head = delBegin(Head);
								}

							}

*/
