/*
* CS 630 - Lab 3 Code Submission
* Team 2: Suraj Kumar Ojha, Tapan Basak, Rahul Pavithran
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFLEN 128
#define INSNUM 8
/*internal instructions*/
char *instr[INSNUM] = {"show","set","export","unexport","show","exit","wait","help"};
/*predefined variables*/
/*varvalue[0] stores the rootpid of xssh*/
/*varvalue[3] stores the childpid of the last process that was executed by xssh in the background*/
int varmax = 3;
char varname[BUFLEN][BUFLEN] = {"$\0", "?\0", "!\0",'\0'};
char varvalue[BUFLEN][BUFLEN] = {'\0', '\0', '\0'};
/*remember pid*/
int childnum = 0;
pid_t childpid = 0;
pid_t rootpid = 0;
// Extra - Array to keep track of previous INSNUM child processes. 
pid_t childpids[INSNUM] = {}; 
/*current dir*/
char rootdir[BUFLEN] = "\0";

/*functions for parsing the commands*/
int deinstr(char buffer[BUFLEN]);
void substitute(char *buffer);

/*functions to be completed*/
int xsshexit(char buffer[BUFLEN]);
void show(char buffer[BUFLEN]);
void help(char buffer[BUFLEN]);
int program(char buffer[BUFLEN]);
void catchctrlc();
void ctrlsig(int sig);
void waitchild(char buffer[BUFLEN]);
void set(char buffer[BUFLEN]);
void export(char buffer[BUFLEN]);
void unexport(char buffer[BUFLEN]);

/*for optional exercise, implement the function below*/
int pipeprog(char buffer[BUFLEN]);

/*main function*/
int main()
{
	/*set the variable $$*/
	rootpid = getpid();
	childpid = rootpid;
	sprintf(varvalue[0], "%d\0", rootpid);
	/*capture the ctrl+C*/
	catchctrlc();
	/*run the xssh, read the input instrcution*/
	int xsshprint = 0;
	if(isatty(fileno(stdin))) xsshprint = 1;
	if(xsshprint) printf("xssh>> ");
	char buffer[BUFLEN];
	while(fgets(buffer, BUFLEN, stdin) > 0)
	{
		/*substitute the variables*/
		substitute(buffer);
		/*delete the comment*/
		char *p = strchr(buffer, '#');
		if(p != NULL)
		{
			*p = '\n';
			*(p+1) = '\0';
		}
		/*decode the instructions*/
		int ins = deinstr(buffer);
		/*run according to the decoding*/
		if(ins == 1)
			show(buffer);
		else if(ins == 2)
			set(buffer);
		else if(ins == 3)
			export(buffer);
		else if(ins == 4)
			unexport(buffer);
		else if(ins == 5) show(buffer); //Not used for now
		else if(ins == 6)
			xsshexit(buffer);
		else if(ins == 7)
			waitchild(buffer);
		else if(ins == 8)
			help(buffer);
		else if(ins == 9)
			continue;
		else
		{
			char *ptr = strchr(buffer, '|');
			if(ptr != NULL)
			{
				int err = pipeprog(buffer);
				if(err != 0)break;
			}
			else
			{
				int err = program(buffer);
				if(err != 0)break;
			}
		}
		if(xsshprint) printf("xssh>> ");
		memset(buffer, 0, BUFLEN);
	}
	return -1;
}

/*exit I*/
int xsshexit(char buffer[BUFLEN])
{
	exit(0);
}

/*show W*/
void show(char buffer[BUFLEN])
{
	//FIXME: print the string after "show " in buffer
	//hint: where is the start of this string?
	int startingIndex = 5; // staring index of the string we want to print
	printf("%s", buffer+startingIndex);
}

/*help*/
void help(char buffer[BUFLEN])
{
	printf("help info\n");
}

/*export variable --- set the variable name in the varname list*/
void export(char buffer[BUFLEN])
{
        int i, j;
	//flag == 1, if variable name exists in the varname list
        int flag = 0;
	//parse and store the variable name in buffer[]
        char str[BUFLEN];
	int start = 7;
	while(buffer[start]==' ')start++;
        for(i = start; (i < strlen(buffer))&&(buffer[i]!='#')&&(buffer[i]!=' ')&&(buffer[i]!='\n'); i++)
        {
                str[i-start] = buffer[i];
        }
        str[i-start] = '\0';
	//hint: try to print "str" and "varname[j]" to see what's stored there
        for(j = 0; j < varmax; j++)
        {
		//FIXME: if the variable name (in "str") exist in the
		//varname list (in "varname[j]"), set the flag to be 1
		//using strcmp()
			if(!strcmp(varname[j], str)){
				flag = 1;
				break;
			}
        }
        if(flag == 0) //variable name does not exist in the varname list
        {
		//FIXME: copy the variable name to "varname[varmax]" using strcpy()
			strcpy(varname[varmax], str);
		//FIXME: set the corresponding value in "varvalue[varmax]" to empty string '\0'
			varvalue[varmax][varmax] = '\0';
		//FIXME: update the 'varmax' (by +1)
			varmax += 1;
		//FIXME: print "-xssh: Export variable str.", where str is newly exported variable name
			printf("-xssh: Exported variable %s.\n", str);
        }
        else //variable name already exists in the varname list
		{
		//FIXME: print "-xssh: Existing variable str is value.", where str is newly exported variable name and value is its corresponding value (stored in varvalue list)
			printf("-xssh: Existing variable %s is %s.\n", varname[j], varvalue[j]);
        }
}

/*unexport the variable --- remove the variable name in the varname list*/
void unexport(char buffer[BUFLEN])
{
        int i, j;
	//flag == 1, if variable name exists in the varname list
        int flag = 0;
	//parse and store the variable name in buffer[]
        char str[BUFLEN];
	int start = 9;
	while(buffer[start]==' ')start++;
        for(i = start; (i < strlen(buffer))&&(buffer[i]!='#')&&(buffer[i]!=' ')&&(buffer[i]!='\n'); i++)
        {
                str[i-start] = buffer[i];
        }
        str[i-start] = '\0';
        for(j = 0; j < varmax; j++)
        {
		//FIXME: if the variable name (in "str") exist in the
		//varname list (in "varname[j]"), set the flag to be 1
		//using strcmp() --- same with export()
			if(!strcmp(varname[j], str)){
				flag = 1;
				break;
			}
        }
        if(flag == 0) //variable name does not exist in the varname list
        {
		//FIXME: print "-xssh: Variable str does not exist.",
		//where str is the variable name to be unexported
			printf("-xssh: Variable %s does not exist.\n", str);
        }
        else //variable name already exists in the varname list
		{
		//FIXME: clear the found variable by setting its
		//"varname" and "varvalue" both to '\0'
			varname[j][0] = '\0';
			varvalue[j][0] = '\0';
		//FIXME: print "-xssh: Variable str is unexported.",
		//where str is the variable name to be unexported
			printf("-xssh: Variable %s is unexported.\n", str);
        }
}

/*set the variable --- set the variable value for the given variable name*/
void set(char buffer[BUFLEN])
{
	int i, j;
	//flag == 1, if variable name exists in the varname list
	int flag = 0;
	//parse and store the variable name in buffer[]
	char str[BUFLEN];
	int start = 4;
	while(buffer[start]==' ')start++;
	for(i = start; (i < strlen(buffer))&&(buffer[i]!=' ')&&(buffer[i]!='#'); i++)
	{
		str[i-start] = buffer[i];
	}
	str[i-start] = '\0';
	while(buffer[i]==' ')i++;
	if(buffer[i]=='\n')
	{
		printf("No value to set!\n");
		return;
	}
	for(j = 0; j < varmax; j++)
	{
		//FIXME: if the variable name (in "str") exist in the
		//varname list (in "varname[j]"), set the flag to be 1
		//using strcmp() --- same with export()
		if(!strcmp(varname[j], str)){
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		//FIXME: print "-xssh: Variable str does not exist.",
		//where str is the variable name to be unexported
		printf("-xssh: Variable %s does not exist.\n", str);
	}
	else
	{
		//hint: try to print "buffer[i]" to see what's stored there
		//hint: may need to add '\0' by the end of a string
		//FIXME: set the corresponding varvalue to be value (in buffer[i]) using strcpy()
		strcpy(varvalue[j], buffer+i);
		while (buffer[i] != ' ' && buffer[i] != '\n') i++;
		varvalue[j][strlen(varvalue[j]) - 1] = '\0';
		// FIXME: print "-xssh: Set existing variable str to value.", where str is newly exported variable name and value is its corresponding value (stored in varvalue list)
		printf("-xssh: Set existing variable %s to %s.\n", str, varvalue[j]);
	}
}

/*catch the ctrl+C*/
void catchctrlc()
{
	printf("catching ctrl+C\n");
}

/*ctrl+C handler*/
void ctrlsig(int sig)
{
	fflush(stdout);
}

/*wait instruction*/
void waitchild(char buffer[BUFLEN])
{
	int i;
	int start = 5;
	int status;
	/*store the childpid in pid*/
	char number[BUFLEN] = {'\0'};
	while(buffer[start]==' ')start++;
	for(i = start; (i < strlen(buffer))&&(buffer[i]!='\n')&&(buffer[i]!='#'); i++)
	{
		number[i-start] = buffer[i];
	}
        number[i-start] = '\0';
	char *endptr;
	int pid = strtol(number, &endptr, 10);
	/*simple check to see if the input is valid or not*/
	if((*number != '\0')&&(*endptr == '\0'))
	{
		//FIXME: if pid is not -1, try to wait the background process pid
		//FIXME: if successful, print "-xssh: Have finished waiting process pid", where pid is the pid of the background process
		//FIXME: if not successful, print "-xssh: Unsuccessfully wait the background process pid", where pid is the pid of the background process
			if (pid != -1) {
				// waitpid returns the status of the terminated child in status variable
				// passed as second argument in the above line of code
				// if the status is normal exit, if block will execute
				// in cases of error in proces termination, else block will execute
				// the third argument 'Options' is set to 0 to make sure that parent process waits
				// until we have a status of the child
				pid_t cpid = waitpid(pid, &status, 0);
				if (WIFEXITED(status)) {
					printf("-xssh: Have finished waiting process %d\n", pid);
					childnum--;
				} else {
					printf("-xssh: Unsuccessfully wait the background process %d\n", pid);
				}
			} else {
				// need to calculate all the background processes
				printf("-xssh: wait %d background processes\n", childnum);
				// Extra - While number of child processes is greater than 0, wait each child process
				while (childnum > 0) {
					pid_t cpid = waitpid(childpids[childnum-1], &status, 0);
					if (WIFEXITED(status)) {
						printf("-xssh: Have finished waiting process %d\n", childpids[childnum-1]);
						childnum--;
					} else {
						printf("-xssh: Unsuccessfully wait the background process %d\n", childpids[childnum-1]);
					}
				}
			}

		//FIXME: if pid is -1, print "-xssh: wait childnum background processes" where childnum stores the number of background processes, and wait all the background processes
		//hint: remember to set the childnum correctly after waiting!

	}
	else printf("-xssh: wait: Invalid pid\n");
}

/*execute the external command*/
int program(char buffer[BUFLEN])
{
	/*if backflag == 0, xssh need to wait for the external command to complete*/
	/*if backflag == 1, xssh need to execute the external command in the background*/
	int backflag = 0;
	char *ptr = strchr(buffer, '&');
	// Extra - Run command in foreground if backgound process limit is reached.
	if(ptr != NULL && childnum < (INSNUM - 1)) backflag = 1;

	// Extra - Returning -3 if we have reached the limit of 7 background processes.
	if (childnum >= (INSNUM-1)) {
		printf("Number of processes has reached the limit of %d. Running in foreground ...\n", INSNUM);
	}

	pid_t pid;
	//FIXME: create a new process for executing the external command
	pid = fork();
	//FIXME: remember to check if the process creation is successful or not. if not, print error message and return -2, see codes below;
	if (pid < 0) {
		printf("-xssh: Unable to create process to execute external command.");
		return -2;
	}
	//FIXME: write the code to execute the external command in the newly created process, using execvp()
	//hint: the external command is stored in buffer, but before execute it you may need to do some basic validation check or minor changes, depending on how you execute
	//FIXME: remember to check if the external command is executed successfully; if not, print error message "-xssh: Unable to execute the instruction buffer", where buffer is replaced with the actual external command to be printed
	//hint: after executing the extenal command using execvp(), you need to return -1;
	/*for optional exercise, implement stdin/stdout redirection in here*/
	else if (pid == 0){
		// printf("Replace me for executing external commands\n");
		// Remove trailing whitespaces and & flag.
		buffer[strcspn(buffer, "&\r\n")] = 0;
		int n = 5;
		char** argv = (char**) malloc(n * sizeof(char**));
		char* token = strtok(buffer, " ");
		int i = 0;
		while (token != NULL){
			argv[i] = token;
			token = strtok(NULL, " ");
			i++;
			if (i == n-1) {
				n = n + 5;
					argv = (char**) realloc(argv, n * sizeof(char**));
			}
		}
		argv[i] = NULL;

		if(execvp(argv[0], argv) < 0){
			printf("-xssh: Unable to execute the instruction %s\n", argv[0]);
			free(argv);
			return -1;
		}
	}
	//FIXME: in the xssh process, remember to act differently, based on whether backflag is 0 or 1
	//hint: the codes below are necessary to support command "wait -1", but you need to put them in the correct place
	else {
		int status;
		sprintf(varvalue[2], "%d\0", pid);
		childpid = pid;
		childpids[childnum] = pid;
		childnum++;
		if (backflag == 0) {
			waitpid(childpid, &status, 0);
			childnum--;
		}
		return 0;
	}
		//childnum++;
		//	childpid = pid;
		//	childnum--; //this may or may not be needed, depending on where you put the previous line
	//hint: the code below is necessary to support command "show $!", but you need to put it in the correct place
		//	sprintf(varvalue[2], "%d\0", pid);
		//	return 0;
}

/*for optional exercise, implement the function below*/
/*execute the pipe programs*/
int pipeprog(char buffer[BUFLEN])
{
	printf("-xssh: For optional exercise: currently not supported.\n");
	return 0;
}

/*substitute the variable with its value*/
void substitute(char *buffer)
{
	char newbuf[BUFLEN] = {'\0'};
	int i;
	int pos = 0;
	for(i = 0; i < strlen(buffer);i++)
	{
		if(buffer[i]=='#')
		{
			newbuf[pos]='\n';
			pos++;
			break;
		}
		else if(buffer[i]=='$')
		{
			if((buffer[i+1]!='#')&&(buffer[i+1]!=' ')&&(buffer[i+1]!='\n'))
			{
				i++;
				int count = 0;
				char tmp[BUFLEN];
				for(; (buffer[i]!='#')&&(buffer[i]!='\n')&&(buffer[i]!=' '); i++)
				{
					tmp[count] = buffer[i];
					count++;
				}
				tmp[count] = '\0';
				int flag = 0;
        			int j;
				for(j = 0; j < varmax; j++)
        			{
                			if(strcmp(tmp,varname[j]) == 0)
					{
						flag = 1;
						break;
                			}
        			}
        			if(flag == 0)
        			{
					printf("-xssh: Does not exist variable $%s.\n", tmp);
        			}
        			else
				{
					strcat(&newbuf[pos], varvalue[j]);
					pos = strlen(newbuf);
        			}
				i--;
			}
			else
			{
				newbuf[pos] = buffer[i];
				pos++;
			}
		}
		else
		{
			newbuf[pos] = buffer[i];
			pos++;
		}
	}
	if(newbuf[pos-1]!='\n')
	{
		newbuf[pos]='\n';
		pos++;
	}
	newbuf[pos] = '\0';
	strcpy(buffer, newbuf);
	//printf("Decode: %s", buffer);
}

/*decode the instruction*/
int deinstr(char buffer[BUFLEN])
{
	int i;
	int flag = 0;
	for(i = 0; i < INSNUM; i++)
	{
		flag = 0;
		int j;
		int stdlen = strlen(instr[i]);
		int len = strlen(buffer);
		int count = 0;
		j = 0;
		while(buffer[count]==' ')count++;
		if((buffer[count]=='\n')||(buffer[count]=='#'))
		{
			flag = 0;
			i = INSNUM;
			break;
		}
		for(j = count; (j < len)&&(j-count < stdlen); j++)
		{
			if(instr[i][j] != buffer[j])
			{
				flag = 1;
				break;
			}
		}
		if((flag == 0) && (j == stdlen) && (j <= len) && (buffer[j] == ' '))
		{
			break;
		}
		else if((flag == 0) && (j == stdlen) && (j <= len) && (i == 5))
		{
			break;
		}
		else if((flag == 0) && (j == stdlen) && (j <= len) && (i == 7))
		{
			break;
		}
		else
		{
			flag = 1;
		}
	}
	if(flag == 1)
	{
		i = 0;
	}
	else
	{
		i++;
	}
	return i;
}











