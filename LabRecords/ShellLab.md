# ShellLab

首先要做的事情当然是仔细阅读`writeup`文件:

该实验要求在提供的`tsh.c`代码框架下，实现一个能支持作业调度、进程信号控制以及前后台作业切换的`tiny shell——tsh`。其中要求实现的函数包括：

![implement-func](./pic/implement-func.png)

根据阅读书本上提供的shell例程，我们可以知道最简单的shell是由以下几部分组成:

```c
int main(int argc, char *argv[]);
void eval(char *cmdlind);
int parseline(char *buf, char *argv[]);
int builtin_command(char *argv[]);
```

一个C程序的入口永远是`<main>`函数，所以我们考虑从阅读`<main>`函数入手：

## main
```c
/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];
    int emit_prompt = 1; /* emit prompt (default) */

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Parse the command line */
    while ((c = getopt(argc, argv, "hvp")) != EOF) {
        switch (c) {
        case 'h':             /* print help message */
            usage();
	    break;
        case 'v':             /* emit additional diagnostic info */
            verbose = 1;
	    break;
        case 'p':             /* don't print a prompt */
            emit_prompt = 0;  /* handy for automatic testing */
	    break;
	default:
            usage();
	}
    }

    /* Install the signal handlers */

    /* These are the ones you will need to implement */
    Signal(SIGINT,  sigint_handler);   /* ctrl-c */
    Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
    Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */

    /* This one provides a clean way to kill the shell */
    Signal(SIGQUIT, sigquit_handler); 

    /* Initialize the job list */
    initjobs(jobs);

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	if (emit_prompt) {
	    printf("%s", prompt);
	    fflush(stdout);
	}
	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
	    app_error("fgets error");
	if (feof(stdin)) { /* End of file (ctrl-d) */
	    fflush(stdout);
	    exit(0);
	}

	/* Evaluate the command line */
	eval(cmdline);
	fflush(stdout);
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
```
`<main>`在定义了一系列变量之后进入了一个`while`循环，在循环中调用了`<getopt>`函数，这个函数可以对执行程序时输入的参数进行选择处理。

随后即install了三个信号处理程序:
```c
Signal(SIGINT,  sigint_handler);   /* ctrl-c */
Signal(SIGTSTP, sigtstp_handler);  /* ctrl-z */
Signal(SIGCHLD, sigchld_handler);  /* Terminated or stopped child */
```
这也正是我们需要实现的函数，继续往后读。在初始化任务列表`<initjobs>`后便正式进入了我们的shell循环:

不停地从标准输入`stdin`中按行读取字符串存储到`cmdline`中，并将`cmdline`作为参数传递给`<eval>`函数进行解析执行。

我们现在可以考虑开始编写我们需要实现的函数了，由于`<eval>`比较复杂，而信号处理程序的实现较为简单，所以我们考虑首先来完成三个信号处理程序。


## sig*_handler

### sigint_handler
实现一个SIGINT信号处理函数，当用户按下`Ctrl-c`时，shell会捕捉到内核发出的SIGINT信号并传递给前台作业。

代码实现如下:
```c
void sigint_handler(int sig)
{
	int olderrno = errno;
	pid_t pid;
	sigset_t mask_all, prev_mask;
	Sigfillset(&mask_all);
	Sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
	if(pid = fgpid(jobs) != 0){
		Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
		Kill(-pid, SIGINT);
	}
	errno = olderrno;
    return;
}
```




### sigtstp_handler

```c
void sigtstp_handler(int sig) 
{
	int olderrno = errno;
	pid_t pid;
	sigset_t mask_all, prev_mask;
	Sigfillset(&mask_all);
	Sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
	if(pid = fgpid(jobs) != 0){
		Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
		Kill(-pid, SIGSTOP);
	}
	errno = olderrno;
    return;
```


### sigchld_handler

```c
void sigchld_handler(int sig) 
{
	int olderrno = errno;
	int status;
	pid_t pid;
	struct job_t *job;
	sigset_t mask_all, prev_mask;
	Sigfillset(&mask_all);
	while((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0){
		Sigprocmask(SIG_BLOCK, &mask_all, &prev_mask);
		if(WIFEXITED(status))	/* terminated normally */
			deletejob(jobs, pid);
		else if(WIFSIGNALED(status)){
			printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));
			deletejob(jobs, pid);
		}
		else if(WIFSTOPPED(status)){
			printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(pid), pid, WSTOPSIG(status));
			job = getjobpid(jobs, pid);
			job->state = ST;
		}
		Sigprocmask(SIG_SETMASK, &prev_mask, NULL);
	}
	errno = olderrno; 
    return;
}
```









## eval

根据文档中给的提示:

```c
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
```

`<eval>`代码实现如下：

```c


```









