#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <ulimit.h>
#include <stdlib.h>
#include <sys/resource.h>


/*
* Что такое также реальный идентификатор? В чём разница с эффективным?
* Эффективные ID - это идентификатор, от имени которого действует пользователь или группа. То есть пользователь с одним ID может представиться другим.
* `man core(5)` - файл, в котором будет храниться образ памяти процесса, созданный в момент его экстренного завершения. Полезен для отладки
*/

char* commands = "ispuU:cC:dvV:";
// Если указать после буквы : или ::
// : - требуется значение для аргумента, будет помещено во внешнюю переменную char* optarg
// :: - опциональное значение
// если после буквы команды идёт текст без пробела, то весь текст будет также помещён в optarg
// Если не указывать : или ::, то optarg = 0

int main(int argc, char** argv, char** envp) {
	int tmpI;
	long int tmpLI;
	char* tmpCP;
	struct rlimit *rlim;

	// optind - внешняя переменная, которая указывает на текущий обрабатываемый элемент argv. 
	// При каждом вызове getopt инкрементируется. Если аргументы закончились, optind будет равняться длине argv, а getopt вернёт -1
	// Если аргумент не был распознан, вернётся '?'
	char c = getopt(argc, argv, commands);
	if (c == -1)
		perror("No arguments\n"); 
	while (c != -1)
	{
		if (c == '?') {
			c = getopt(argc, argv, commands);
			continue;
		}
		switch (c)
		{
		case 'i':
			printf("User ID/EID: %d/%d \nGroup ID/EID: %d/%d \n", 
				getuid(), geteuid(), getgid(), getegid());
			// getuid, geteuid, getgid, getegid - системные вызовы, то есть обращаются к фрагментам код в ядре
			break;
		case 's':
			printf("Setting process Group ID to process ID: %d...\n", getpid());
			if (setpgid(0, 0) == 0) // Системный вызов, эквивалентный setpgid(0,0)
				printf("SUCCESS\nNow we have PID/PGID: %d/%d\n", getpid(), getpgid(0));
			// setpgid(pid, pgid) - устанавливает pgid как идентификатор группы процесса с идентификатором pid
			// Если pid == 0, вызов будет работать с процессом, вызвавшим его
			// Если pgid == 0, в качестве идентификатора будет установлен ID вызвавшего процесса
			break;
		case 'p':
			tmpI = getpid();
			printf("Process ID: %d\nProcess Parent ID: %d\nProcess group ID: %d\n", 
				tmpI, getppid(), getpgid(tmpI));
			break;
		case 'u':
			printf("ulimit is %ld bytes\n", ulimit(UL_GETFSIZE) * 512);
			// Устаревший системный вызов, но, судя по всему, от нас здесь хотят именно её
			// Возвращает лимит на размер создаваемых файлов в единицах по 512 байт
			break;
		case 'U':
			tmpLI = atol(optarg);
			if (ulimit(UL_SETFSIZE, tmpLI) != -1)
				printf("ulimit was changed on %ld bytes\n", ulimit(UL_GETFSIZE) * 512);
			break;
		case 'c':
			// Новый системный вызов для куда более гибкой настройки лимитов.
			/*
				Работает со структурой
				struct rlimit {
					unsigned long int rlim_cur; // Мягкий лимит
					unsigned long int rlim_max; // Жёсткий лимит
				}
			*/
			rlim = (struct rlimit*)malloc(sizeof(struct rlimit));
			if (getrlimit(RLIMIT_CORE, rlim) == 0)
				printf("Limits for core-file size:\nsoft = %lu\nhard = %lu\n",
					rlim->rlim_cur, rlim->rlim_max);
			free(rlim);
			break;
		case 'C':
			tmpLI = atol(optarg);
			rlim = (struct rlimit*)malloc(sizeof(struct rlimit));
			rlim->rlim_max = tmpLI;
			rlim->rlim_cur = tmpLI;
			if (setrlimit(RLIMIT_CORE, rlim) == 0)
				printf("Limits for core-file size was set to %ld\n", tmpLI);
			free(rlim);
			break;
		case 'd':
			tmpCP = (char*)malloc(1024); // В документации было сказано выставить значение переменной среды PATH_MAX, но она не задана
			getwd(tmpCP);
			printf("Working directory: %s\n", tmpCP);
			free(tmpCP);
			break;
		case 'v':
			for (tmpI = 0; envp[tmpI] != NULL; ++tmpI)
				printf("%s\n", envp[tmpI]);
			break;
		case 'V':
			if (putenv(optarg) == 0)
				printf("Added environment variable: %s\n", optarg);
			break;
		default:
			break;
		}
		printf("==================================================\n");
		c = getopt(argc, argv, commands);
	}
	return 0;
}
