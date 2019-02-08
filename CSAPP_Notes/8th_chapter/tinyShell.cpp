/*
A tiny shell to check out how syscall execve works:
To simulate the shell program, the program accept a filename that is executable, 
and look it up in an directory, and use syscall execve to load the executable file
in that directory. Users can call setenv(dir) to include another directory in the 
environ, which will be a string array in this simple implemtation.
*/

#include <sys/wait.h>
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

void printAnchor();
void split(string s, vector<string> &svec, char sep);
void runcmd(const vector<string>& argv, char* envp[]);
bool builtin_cmd(const vector<string> &arg);
void ls();
void cd(string dir);
int main(int argc, char* argv[], char* envp[])
{
	string cmd;
	pid_t pid;
	
	while (true){
		vector<string> arg;	
		printAnchor();		
		getline(cin, cmd);
		//cout << "Debug: " << cmd << endl;
		split(cmd, arg, ' ');
		//cout << "Debug: " << arg[0] << endl;
		if (!builtin_cmd(arg)){
			if ((pid = fork()) == 0){
				runcmd(arg, envp);			
			}
			wait(NULL);	
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
}

void runcmd(const vector<string> &argv, char* envp[])
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
	if (status == -1){
		cout << "execve error: "<< strerror(errno) << endl;
		exit(1);
	} 
}

void split(string s, vector<string>& argv, char sep = ' ')
{
	string each;
	size_t i = 0;
	while(s[i] == ' '){
		i++;
	}
	s = s.substr(i);
	for (i=0; i<s.size(); ++i){
		if (s[i] != sep)
			each += s[i];
		else {
			argv.push_back(each);
			each = "";
		}	
	}
	if (!each.empty() && each != " ")
		argv.push_back(each);
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

/*
To be improved:
working just fine in the gnome-tty
but when working in the tty, line61 fail to getenv;
*/
	

