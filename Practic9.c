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
	int count1;			//Количество файлов, посчитанных в первом процессе.
	int count2;			//Количество файлов, посчитанных во втором процессе.
	int count3;			//Количество файлов, посчитанных в третьемм процессе.
	int count4;			//Количество файлов, посчитанных в четвёртом процессе.
	char Out1[4096];	//Строка для передачи пути из дочернего процесса в родительский.
	char Out2[4096];	//Строка для передачи пути из дочернего процесса в родительский.
	char Out3[4096];	//Строка для передачи пути из дочернего процесса в родительский.
	char Out4[4096];	//Строка для передачи пути из дочернего процесса в родительский.
	bool IsEnd1;		//Флаг конца работы первого процесса.
	bool IsEnd2;		//Флаг конца работы второго процесса.
	bool IsEnd3;		//Флаг конца работы третьего процесса.
	bool IsEnd4;		//Флаг конца работы четвертого процесса.
	bool IsSet1;		//Флаг заполнения строки OUT1.
	bool IsSet2;		//Флаг заполнения строки OUT2.
	bool IsSet3;		//Флаг заполнения строки OUT3.
	bool IsSet4;		//Флаг заполнения строки OUT4.
	int ps;				//Количество задействованных процессов.
};

struct List 			//Структура для списков с путями.
{
	char file [4096];
	struct List *next;
};

#define SHMNAME "/OURE_SPACE"				 //Название участка разделяемой памяти.
#define SEMNAME "/OURE_SEM"					 //Название семафора
#define SIZE_STRUCT sizeof(struct ListOfFile)//Размер основной (первой) структуры 

struct List *Init(char file[4096])			
{
	//Создание начала списка с папками
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

struct List* addFilename(struct List* Files, char file[4096])
{
	//Добавление элементов в список или создание списка
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
		else printf("\n77 %s",strerror(errno));
	}
	else Files = Init(file);

	return Files;
}

struct List *delBegin(struct List* List)
{
	//Удаление начального элемента из списка
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
        return List;
    }

	return List;
}

struct ListOfFile* InitCildStruct(struct ListOfFile* ChildStruct)
{
	//Начальные значения основной структуры
	ChildStruct->count1 = 0;
	ChildStruct->count2 = 0;
	ChildStruct->count3 = 0;
	ChildStruct->count4 = 0;
	ChildStruct->ps = 0;
	ChildStruct->IsEnd1 = true;
	ChildStruct->IsEnd2 = true;
	ChildStruct->IsEnd3 = true;
	ChildStruct->IsEnd4 = true;
	ChildStruct->IsSet1 = false;
	ChildStruct->IsSet2 = false;
	ChildStruct->IsSet3 = false;
	ChildStruct->IsSet4 = false;
	return ChildStruct;	
}

//Основная функция.
//Обработка папок и заполнения списка с папками.
//Функция возвращает заполненный список с папками
//и количество посчитанных файлов размером больше 5 Кб.
struct List* FindeFile(char Way[4095], struct List* LocalList, int *count)
{
	errno = 0; 			   			 //Глобальная переменная, в которую записывается номер ошибки
	struct dirent* DirBox; 			 //Структура, в которую записывается считанная информация
	char* name;			   			 //Название файла, которое мы извлекаем из  DirBox
	struct stat buf;	   			 //Информация о файле/каталоге
    int FileSize;             		 //Размер файла
	
		DIR *directory = opendir(Way);//Открывает файл для читения
		if (directory == NULL) printf("\nMemory overflov %s",strerror(errno));//Проверка на то, что хватило памяти для названия репозитория
		else
		{
			while ((DirBox = readdir(directory)) != NULL)//проверка на конец файла
			{
				name = DirBox -> d_name;
				char *Str = calloc(strlen(Way)+strlen(name)+2,1);
				char *Str1 = calloc(strlen(Way)+3,1);

				if(Str1 == NULL)printf("\nMemory overflov\n");
				else
				{
					char *Str2 = calloc(strlen(Way)+4,1);//выделенная память
					if(Str2 == NULL)printf("\nMemory overflov\n");
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

						if (Str == NULL) printf("\nMemory overflov\n");
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
										(*count)++;
									}

								}
								else if((S_ISDIR(buf.st_mode) !=0)&&(strcmp(Str,Str1) != 0)&&(strcmp(Str,Str2) != 0)&&(Str1 != NULL)&&(Str2 != NULL))
								{
									//This is directory
									LocalList = addFilename(LocalList,Str);									
								}
								else if((Str1 == NULL)||(Str2 == NULL)) printf("\n Memory overflov");

							}
							else if(errno) printf("\n %s %s",strerror(errno), Str);

							free(Str);

						}
						free(Str2);
					}
					free(Str1);
				}
			}
			if (errno) printf("\n %s %s PID = %d",strerror(errno), Way, getpid());
			closedir(directory);
			printf("\nocunt in func = %d\n", *count);	 
		}		

	return LocalList;
}

bool Condition(int ps, pid_t pid1, pid_t pid2, pid_t pid3, pid_t pid4)
{
	//Проверка идентификаторов порождённх процессов в зависимости от их количества.
	if(ps == 1) return pid1>0;
	else if(ps == 2) return pid1>0&&pid2>0;
	else if(ps == 3) return pid1>0&&pid2>0&&pid3>0;
	else if(ps == 4) return pid1>0&&pid2>0&&pid3>0&&pid4>0;
	else if(ps == 0) return true;
	else false;	
}

int main(void)
{
	char Way[4095] = "/home/lenovo/Desktop/Колледж/EXD";//Основная папка
	char Way1[4095],Way2[4095],Way3[4095],Way4[4095]; 	//Путь к папке для процесса
	int num = 0;										//Номер процесса
	int count = 0;										//Количество файлов
	errno = 0; 			   			 					//Глобальная переменная, в которую записывается номер ошибки
	int Space = shm_open(SHMNAME, O_CREAT|O_RDWR,0600);	//Общая память
	struct List *Parent = NULL;							//Основной списсок с папками в родительском процессе
	pid_t pid1, pid2, pid3, pid4;						//Идентификаторы процессов 
	if(Space < 0) printf("\n shm_open %s \n", strerror(errno));
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
				sem_t* main_sem = sem_open(SEMNAME, O_CREAT,0666,5);
				if(main_sem != SEM_FAILED)
				{
					Child = InitCildStruct(Child);//Заполнение структуры
					//Основная работа
					
					//Первый проход по корневой папке
					if(sem_wait(main_sem) == 0) Parent = FindeFile(Way, Parent, &count);
					else printf("\n sem_wait %s",strerror(errno));
					
					//Обработка списка с процессами
					while(Parent != NULL)
					{
						//Порождение новых процессов для каждой папки
						if(Parent != NULL&&Child->IsEnd1&&Condition(Child->ps, pid1, pid2, pid3, pid4))
						{
							Child->ps++;
							Child->IsEnd1 = false;
							strcpy(Way1, Parent->file);
							Parent = delBegin(Parent);
							pid1 = fork();
							num = 1;
						}
						if(Parent != NULL&&Child->IsEnd2&&Condition(Child->ps, pid1, pid2, pid3, pid4))
						{
							Child->ps++;
							Child->IsEnd2 = false;
							strcpy(Way2, Parent->file);
							Parent = delBegin(Parent);
							pid2 = fork();
							num = 2;
						}
						if(Parent != NULL&&Child->IsEnd3&&Condition(Child->ps, pid1, pid2, pid3, pid4))
						{
							Child->ps++;
							Child->IsEnd3 = false;
							strcpy(Way3, Parent->file);
							Parent = delBegin(Parent);
							pid3 = fork();
							num = 3; 
						}
						if(Parent != NULL&&Child->IsEnd4&&Condition(Child->ps, pid1, pid2, pid3, pid4))
						{
							Child->ps++;
							Child->IsEnd4 = false;
							strcpy(Way4, Parent->file);
							Parent = delBegin(Parent);
							pid4 = fork();
							num = 4; 
						}
						
						if(Condition(Child->ps, pid1, pid2, pid3, pid4))
						{							
							//Parent process
							//Получаем из дочерних процессов пути к папкам
							while (!Child->IsEnd1||!Child->IsEnd2||!Child->IsEnd3||!Child->IsEnd4)
							{
								if(Child->IsSet1)
								{
									Parent = addFilename(Parent,Child->Out1);								
									Child->IsSet1 = false;
								}
								if(Child->IsSet2)
								{
									Parent = addFilename(Parent,Child->Out2);								
									Child->IsSet2 = false;
								}
								if(Child->IsSet3)
								{
									Parent = addFilename(Parent,Child->Out3);								
									Child->IsSet3 = false;
								}
								if(Child->IsSet4)
								{
									Parent = addFilename(Parent,Child->Out4);								
									Child->IsSet4 = false;
								}
		
							}
								
						}
						else if(pid1 == -1||pid2 == -1||pid3 == -1||pid4 == -1) 
						{
							//ERROR
							//Обработка ошибок
							printf("\nFork ERROR: %s",strerror(errno));
						}
						else if(pid1 == 0||pid2 == 0||pid3 == 0||pid4 == 0)
						{
							//Child process
							//Обработка папок в дочерних процессах. 
							//Подсчет общего количества файлов в каждом процессе.
							if(num == 1)
							{
								struct List *First = NULL;
								int fs1 = Child->count1;
								if(sem_wait(main_sem) == 0) First = FindeFile(Way1, First, &fs1);
								else printf("\n sem_wait %s",strerror(errno));
								Child->count1 =fs1;
								printf("\ncount1 = %d ps = %d\n",Child->count1, fs1);
								while (First!=NULL)
								{
									if(!Child->IsSet1)
									{
										strcpy(Child->Out1,First->file);
										Child->IsSet1 = true;
										First = delBegin(First);
									}
								
								}
								Child->IsEnd1 = true;
							} 
							else if(num == 2)
							{
								struct List *Second = NULL;
								int fs2 = Child->count2;
								if(sem_wait(main_sem) == 0) Second = FindeFile(Way2, Second, &fs2);
								else printf("\n sem_wait %s",strerror(errno));
								Child->count2 = fs2;
								printf("\ncount2 = %d ps = %d\n",Child->count2, fs2);
								while (Second!=NULL)
								{
									if(!Child->IsSet2)
									{
										strcpy(Child->Out2,Second->file);
										Child->IsSet2 = true;
										Second = delBegin(Second);
									}
								}
								Child->IsEnd2 = true;
							}
							else if(num == 3)
							{
								struct List *Third = NULL;
								int fs3 = Child->count3;
								if(sem_wait(main_sem) == 0) Third = FindeFile(Way3, Third, &fs3);
								else printf("\n sem_wait %s",strerror(errno));
								Child->count3 = fs3;
								printf("\ncount3 = %d ps = %d\n",Child->count3, fs3);
								while (Third!=NULL)
								{
									if(!Child->IsSet3)
									{
										strcpy(Child->Out3,Third->file);
										Child->IsSet3 = true;
										Third = delBegin(Third);
									}
								}
								Child->IsEnd3 = true;
							} 
							else if(num == 4) 
							{
								struct List *Fourth = NULL;
								int fs4 = Child->count4;
								if(sem_wait(main_sem) == 0) Fourth = FindeFile(Way4, Fourth, &fs4);
								else printf("\n sem_wait %s",strerror(errno));
								Child->count4 = fs4;
								printf("\ncount4 = %d ps = %d\n",Child->count4, fs4);
								while (Fourth!=NULL)
								{
									if(!Child->IsSet4)
									{
										strcpy(Child->Out4,Fourth->file);
										Child->IsSet4 = true;
										Fourth = delBegin(Fourth);
									}
								}
								Child->IsEnd4 = true;
							}
							if(sem_post(main_sem)!=0) printf("\n sem_post in parent %s\n", strerror(errno));
							
							Child->ps--;
							return 0; 
						}
					}
					//Общее количество файлов для вывода
					count += Child->count1+Child->count2+Child->count3+Child->count4;
					if(sem_post(main_sem)!=0) printf("\n422 sem_post in parent %s\n", strerror(errno));
					//Все освобождаем
					if (sem_close(main_sem) != 0)
					{
						printf("\n426 sem_close %s\n",strerror(errno));
					}

				}
				else
				{
					printf("\n432 sem_open %s\n",strerror(errno));
				}

				//Все освобождаем
				if(munmap(adr, SIZE_STRUCT) != 0) printf("\n munmap = %s", strerror(errno));
				if(close(Space) != 0)printf("\n close = %s", strerror(errno));
				if(shm_unlink(SHMNAME))printf("\n unlink = %s", strerror(errno));

			}
			else
			{
				printf("\n mmap %s\n",strerror(errno));
			}

		}
		else
		{
			printf("\n mmap %s\n",strerror(errno));
		}

	}
	printf("\n COUNT = %d\n", count);
	return 0;
}
