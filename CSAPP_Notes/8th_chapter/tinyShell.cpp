/*
A tiny shell to check out how syscall execve works:
To simulate the shell program, the program accept a filename that is executable, 
and look it up in an directory, and use syscall execve to load the executable file
in that directory. Users can call setenv(dir) to include another directory in the 
environ, which will be a string array in this simple implemtation.
*/

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <dirent.h> //opendir
#define MAXDIRLEN 256
using std::cin;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::getline;

pid_t cpid = 0;

void sigint_handler(int signum);
void printAnchor();
int split(string s, vector<string> &svec, char sep);
int runcmd(const vector<string>& argv, char* envp[]);
bool builtin_cmd(const vector<string> &arg);
void ls();
void cd(string dir);
int main(int argc, char* argv[], char* envp[])
{
	string cmd, word;
	//pid_t pid;
	int status;
	if (signal(SIGINT, sigint_handler) == SIG_ERR){
		cerr << "signal handler setup error" << endl;
		exit(1);
	}
	
	while (true){
		vector<string> arg;	
		printAnchor();		
		getline(cin, cmd);
		//cout << "Debug: " << cmd.size() << endl;
		status = split(cmd, arg, ' ');
		if (status == -1) {			
			continue;
		}
		//cout << "Debug: " << arg[0] << endl;
		if (!builtin_cmd(arg)){
			bool background = arg[arg.size()-1].find('&') != std::string::npos;
			if (background) {arg.pop_back(); cout << "background = true" << endl;}
			if ((cpid = fork()) == 0){//-------------------------------------->breakpoint1
				for (const auto& each: arg) cout << each << endl;
				status = runcmd(arg, envp);
				if (status == -1)
					cout << "execve error: "<< strerror(errno) << endl;
				exit(1);			
			}
			if (!background)  //------------------------------------------> breakpoint2
				while(wait(NULL) <= 0) {}	
			else
				cout << cpid << " " << cmd << endl;
		} 	
	}
	return 0;
}

void printAnchor()
{
	string username, hostname, dirname, usersign;
	vector<string> dir;
	username = getenv("USER");
	//cout << "Debug1" << endl;
	if (username == "root") usersign = "##";
	else usersign = "$$";
	hostname = getenv("HOSTNAME");
	if ((dirname = getenv("PWD")) == getenv("HOME")){
		dirname = "~";
	} else {
		split(dirname, dir, '/');
		dirname = dir[dir.size()-1];
	}
	//cout << "Debug2" << endl;
	printf("[%s@%s %s]%s ", username.c_str(), hostname.c_str(), dirname.c_str(), usersign.c_str());
	return;
}

int runcmd(const vector<string> &argv, char* envp[])
{
	vector<string> path;
	string target;
	size_t i;
	int status = 0;
	//make c stype arguement list
	char** c_argv;
	c_argv = (char**)malloc(argv.size());
	for (size_t i = 0; i < argv.size(); ++i){
		c_argv[i] = (char*)malloc(256);
		strncpy(c_argv[i], argv[i].c_str(), 256);		
	}
	split(getenv("PATH"), path, ':');
	if (argv[0][0] == '.'){
		target = argv[0].substr(1);
		target = getenv("PWD") + target;
		//for (auto each: argv) cout << each << endl;
		strncpy(c_argv[0], target.c_str(), 256);	
		status = execve(target.c_str(), c_argv, envp);
	} else {
		for (i=0; i<path.size(); ++i){
			target = path[i] + "/" + argv[0];
			//cout << "target = " << target << endl;
			//for (auto each: argv) cout << each << " ";
			//cout << endl;
			strncpy(c_argv[0], target.c_str(), 256);
			status = execve(target.c_str(), c_argv, envp);
		}
	}
	return status;
}

int split(string s, vector<string>& argv, char sep = ' ')
{
	if (s.empty()) return -1;
	string each;
	size_t i = 0;
	while(s[i] == ' '){
		i++;
	}
	if (i == s.size())
		return -1;
	s = s.substr(i);
	for (i=0; i<s.size(); ++i){
		if (s[i] != sep && s[i] != '&')
			each += s[i];
		else if (s[i] == '&'){
			if (!each.empty()) argv.push_back(each); 
			argv.push_back("&");
			return 0;
		}
		else {
			argv.push_back(each);
			each = "";
		}	
	}
	if (!each.empty())
		argv.push_back(each);
	return 0;
}	

bool builtin_cmd(const vector<string> &arg)
{	
	
	if (arg[0] == "quit")
		exit(0);
	if (arg[0] == "pwd"){
		cout << getenv("PWD") << endl;
		return true;
	} 
	if (arg[0] == "ls"){
		ls();
		return true;
	}
	if (arg[0] == "cd"){
		if (arg.size() == 1)
			cd(getenv("HOME"));
		else
			cd(arg[1]);
		return true;
	}
	if (arg[0] == "getcwd"){
		char buf[256];
		cout << getcwd(buf, 256) << endl;
		return true;
	}
	return false;
}
	
void ls()
{
	vector<string> dirvec;
	const char* dirname = getenv("PWD");
	DIR *dir = opendir(dirname);
	struct dirent* entry;
	if (!dir){
		cerr << "Opendir error..." << endl;
		exit(1);
	}
	errno = 0;
	
	while(true) {
		entry = readdir(dir);
		if (entry)
			dirvec.push_back(entry->d_name);
		else
			break;
	}
	cout << "总用量  " << dirvec.size() << endl;
	for (const auto &each: dirvec)
		cout << each << endl;
}

void cd(string dir)
{
	int status;
	string pwd = getenv("PWD");

	if (dir == ".." && pwd != "/"){
		auto ri = pwd.rfind('/');
		dir = pwd.substr(0, ri);
	} 
	else if (dir == "-")
		dir = getenv("OLDPWD");
	else if (dir[0] != '/'){
		if (pwd[pwd.size()-1] != '/')		
			dir = pwd + "/" + dir;
		else
			dir = pwd + dir;
	}
	setenv("OLDPWD", pwd.c_str(), 1);
	setenv("PWD", dir.c_str(), 1);
	status = chdir(dir.c_str());
	if (status == -1){
		cout << strerror(errno) << endl;	
	}
}

void sigint_handler(int signum)
{
	if (cpid != 0){
		kill(cpid, 9);
		cpid = 0;
		cout << endl;	
	}
	return;
}
/*
[yangjunfeng@yang 8th_chapter]$$ ps -a
ps
-a
  PID TTY          TIME CMD
 4730 pts/1    00:00:40 gedit
 7089 pts/0    00:00:00 tsh
 7091 pts/0    00:06:09 hello
 7288 pts/0    00:00:00 ps
[yangjunfeng@yang 8th_chapter]$$ pwd
/home/yangjunfeng/LinuxEnd/CSAPP/Github/CSAPP_Notes/8th_chapter
[yangjunfeng@yang 8th_chapter]$$ ps -a
ps
-a
execve error: No such file or directory
[yangjunfeng@yang 8th_chapter]$$ 
[yangjunfeng@yang 8th_chapter]$$ ps -a
ps
-a
  PID TTY          TIME CMD
 4730 pts/1    00:00:40 gedit
 7089 pts/0    00:00:00 tsh
 7091 pts/0    00:06:35 hello
 7301 pts/0    00:00:00 ps
[yangjunfeng@yang 8th_chapter]$$ 
*/
/*
在运行了builtin_cmd之后，无法运行外部可执行程序，之后按下一次回车之后才可以。
*/
	

/*程序目前存在多进程竞争冒险问题*/
/*
标注为breakpoint的两句语句存在竞争，在fork出一个子进程之后，系统有可能运行child进程也有可能运行parent进程


*/


