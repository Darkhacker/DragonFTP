/*
   DragonFTP - OpenPS3FTP GUI Port - DeadRSX 0.1
*/
#include <psl1ght/lv2.h>
#include <psl1ght/lv2/filesystem.h>
#include <psl1ght/lv2/timer.h>
#include <psl1ght/lv2/errno.h>
#include <errno.h>
#include <net/net.h>
#include <sys/time.h>
#include <sys/thread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

#include <rsx/commands.h>
#include <rsx/nv40.h>
#include <rsx/gcm.h>
#include <rsx/reality.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>
#include <pngdec/loadpng.h>

#include <deadrsx/deadrsx.h>
#include <deadrsx/texture.h>
#include <deadrsx/nv_shaders.h>
#include <sysutil/video.h>
#include <sysutil/events.h>
#include "font.h"
#include "common.h"

// mumilover stuff
#define DEBUGARRAYMAXLINES 16

#define FTPPORT 21 
#define BUFFER_SIZE 32768 
int DISABLE_PASS = 0; // 0 off 1 on
// used for controller make sure nothing else in app using ci
 PadInfo padinfo;
 PadData paddata;
 int ci;

int debug_app = 0; // turn to 1 if your debugging app       

int app_state = 1;
int currentBuffer = 0;
int ftp_started = 0; // not started
int current_selection = 2;
int screensaver = 1;
int option_down = 1;
int option_right = 1;
int btnPressed;
int exitapp = 0;
int state_holder = 1;
int current_background = 0; // change to 1 for default black for now

int dev_dragon = 0;
Lv2FsStat deEntry;

int fps_holder = 0;
int fps_flip = 0;
u32 *tx_mem;

int movement = 0;
int minbackground = 1;
int set = 0;
u64 sec, nsec, lastsec, setsec, frame_tick;
int update_value,keep_value;
int background_color = 0x0000FF00;

const char* VERSION = "0.25";
char D_USER[100] = "root";
char D_PASS[100] = "password";
char fps_char[100] = "";
char userpass[32];
char status[128];
char version[32];

// mumilover stuff
int debugCount = 0; // use it to know how many of the 15 slots are taken.. like an array level pointer#
char *debugArray[DEBUGARRAYMAXLINES]; //there is room for 16 lines af debug info, each around 60 characters long

u32 font_offset,sprite_offset,background_offset,bsprite_offset;
PngDatas font_image,sprite_image,background_image,bsprite_image;

// for the image
u32 tx_offset;
Image dice;

static void ipaddr_get(u64 unused)
{
int ip_s;
if(sconnect(&ip_s, "8.8.8.8", 53) == 0)
{
netSocketInfo p;
netGetSockInfo(FD(ip_s), &p, 1);
sprintf(status, "IP : %s Port : %i", inet_ntoa(p.local_adr), FTPPORT);
}
sclose(&ip_s);
sys_ppu_thread_exit(0);
}


static void handleclient(u64 conn_s_p)
{
int conn_s = (int)conn_s_p; // main communications socket
int data_s = -1; // data socket

int connactive = 1; // whether the ftp connection is active or not
int dataactive = 0; // prevent the data connection from being closed at the end of the loop
int loggedin = 0; // whether the user is logged in or not

char user[32]; // stores the username that the user entered
char rnfr[256]; // stores the path/to/file for the RNFR command

char cwd[256]; // Current Working Directory
int rest = 0; // for resuming file transfers

char buffer[1024];

// generate pasv output
netSocketInfo p;
netGetSockInfo(FD(conn_s), &p, 1);

srand(conn_s);
int p1 = (rand() % 251) + 4;
int p2 = rand() % 256;

char pasv_output[64];
sprintf(pasv_output, "227 Entering Passive Mode (%i,%i,%i,%i,%i,%i)\r\n", NIPQUAD(p.local_adr.s_addr), p1, p2);

// set working directory
strcpy(cwd, "/");

// welcome message
ssend(conn_s, "220-DragonFTP by DarkhackerPS3\r\n");
sprintf(buffer, "220 Version %s\r\n", VERSION);
ssend(conn_s, buffer);

while(exitapp == 0 && connactive == 1 && recv(conn_s, buffer, 1023, 0) > 0)
{
// get rid of the newline at the end of the string
buffer[strcspn(buffer, "\n")] = '\0';
buffer[strcspn(buffer, "\r")] = '\0';

char cmd[16], param[256];
int split = ssplit(buffer, cmd, 15, param, 255);

if(loggedin == 1)
{
// available commands when logged in
if(strcasecmp(cmd, "CWD") == 0)
{
char tempcwd[256];
strcpy(tempcwd, cwd);

if(split == 1)
{
absPath(tempcwd, param, cwd);
}

if(isDir(tempcwd))
{
strcpy(cwd, tempcwd);
sprintf(buffer, "250 Directory change successful: %s\r\n", cwd);
ssend(conn_s, buffer);
}
else
{
ssend(conn_s, "550 Cannot access directory\r\n");
}
}
else
if(strcasecmp(cmd, "CDUP") == 0)
{
int pos = strlen(cwd) - 2;

for(int i = pos; i > 0; i--)
{
if(cwd[i] == '/' && i < pos)
{
break;
}
else
{
cwd[i] = '\0';
}
}

sprintf(buffer, "250 Directory change successful: %s\r\n", cwd);
ssend(conn_s, buffer);
}
else
if(strcasecmp(cmd, "PASV") == 0)
{
rest = 0;

int data_ls = slisten((p1 * 256) + p2, 1);

if(data_ls > 0)
{
ssend(conn_s, pasv_output);

data_s = accept(data_ls, NULL, NULL);

if(data_s > 0)
{
dataactive = 1;
}
else
{
ssend(conn_s, "451 Data connection failed\r\n");
}
}
else
{
ssend(conn_s, "451 Cannot create data socket\r\n");
}

sclose(&data_ls);
}
else
if(strcasecmp(cmd, "PORT") == 0)
{
if(split == 1)
{
rest = 0;

char data[6][4];
char *splitstr = strtok(param, ",");

int i = 0;
while(i < 6 && splitstr != NULL)
{
strcpy(data[i++], splitstr);
splitstr = strtok(NULL, ",");
}

if(i == 6)
{
char ipaddr[16];
sprintf(ipaddr, "%s.%s.%s.%s", data[0], data[1], data[2], data[3]);

if(sconnect(&data_s, ipaddr, ((atoi(data[4]) * 256) + atoi(data[5]))) == 0)
{
ssend(conn_s, "200 PORT command successful\r\n");
dataactive = 1;
}
else
{
ssend(conn_s, "451 Data connection failed\r\n");
}
}
else
{
ssend(conn_s, "501 Insufficient connection info\r\n");
}
}
else
{
ssend(conn_s, "501 No connection info given\r\n");
}
}
else
if(strcasecmp(cmd, "LIST") == 0)
{
if(data_s > 0)
{
char tempcwd[256];
strcpy(tempcwd, cwd);

if(split == 1)
{
absPath(tempcwd, param, cwd);
}

void listcb(Lv2FsDirent entry)
{
char filename[256];
absPath(filename, entry.d_name, cwd);

Lv2FsStat buf;
lv2FsStat(filename, &buf);

char timebuf[16];
strftime(timebuf, 15, "%b %d %H:%M", localtime(&buf.st_mtime));

sprintf(buffer, "%s%s%s%s%s%s%s%s%s%s 1 root root %llu %s %s\r\n",
((buf.st_mode & S_IFDIR) != 0) ? "d" : "-",
((buf.st_mode & S_IRUSR) != 0) ? "r" : "-",
((buf.st_mode & S_IWUSR) != 0) ? "w" : "-",
((buf.st_mode & S_IXUSR) != 0) ? "x" : "-",
((buf.st_mode & S_IRGRP) != 0) ? "r" : "-",
((buf.st_mode & S_IWGRP) != 0) ? "w" : "-",
((buf.st_mode & S_IXGRP) != 0) ? "x" : "-",
((buf.st_mode & S_IROTH) != 0) ? "r" : "-",
((buf.st_mode & S_IWOTH) != 0) ? "w" : "-",
((buf.st_mode & S_IXOTH) != 0) ? "x" : "-",
(unsigned long long)buf.st_size, timebuf, entry.d_name);

ssend(data_s, buffer);
}

ssend(conn_s, "150 Accepted data connection\r\n");

if(slist(isDir(tempcwd) ? tempcwd : cwd, listcb) >= 0)
{
ssend(conn_s, "226 Transfer complete\r\n");
}
else
{
ssend(conn_s, "550 Cannot access directory\r\n");
}
}
else
{
ssend(conn_s, "425 No data connection\r\n");
}
}
else
if(strcasecmp(cmd, "MLSD") == 0)
{
if(data_s > 0)
{
char tempcwd[256];
strcpy(tempcwd, cwd);

if(split == 1)
{
absPath(tempcwd, param, cwd);
}

void listcb(Lv2FsDirent entry)
{
char filename[256];
absPath(filename, entry.d_name, cwd);

Lv2FsStat buf;
lv2FsStat(filename, &buf);

char timebuf[16];
strftime(timebuf, 15, "%Y%m%d%H%M%S", localtime(&buf.st_mtime));

char dirtype[2];
if(strcmp(entry.d_name, ".") == 0)
{
strcpy(dirtype, "c");
}
else
if(strcmp(entry.d_name, "..") == 0)
{
strcpy(dirtype, "p");
}
else
{
dirtype[0] = '\0';
}

sprintf(buffer, "type=%s%s;siz%s=%llu;modify=%s;UNIX.mode=0%i%i%i;UNIX.uid=root;UNIX.gid=root; %s\r\n",
dirtype,
((buf.st_mode & S_IFDIR) != 0) ? "dir" : "file",
((buf.st_mode & S_IFDIR) != 0) ? "d" : "e", (unsigned long long)buf.st_size, timebuf,
(((buf.st_mode & S_IRUSR) != 0) * 4 +
((buf.st_mode & S_IWUSR) != 0) * 2 +
((buf.st_mode & S_IXUSR) != 0) * 1),
(((buf.st_mode & S_IRGRP) != 0) * 4 +
((buf.st_mode & S_IWGRP) != 0) * 2 +
((buf.st_mode & S_IXGRP) != 0) * 1),
(((buf.st_mode & S_IROTH) != 0) * 4 +
((buf.st_mode & S_IWOTH) != 0) * 2 +
((buf.st_mode & S_IXOTH) != 0) * 1),
entry.d_name);

ssend(data_s, buffer);
}

ssend(conn_s, "150 Accepted data connection\r\n");

if(slist(isDir(tempcwd) ? tempcwd : cwd, listcb) >= 0)
{
ssend(conn_s, "226 Transfer complete\r\n");
}
else
{
ssend(conn_s, "550 Cannot access directory\r\n");
}
}
else
{
ssend(conn_s, "425 No data connection\r\n");
}
}
else
if(strcasecmp(cmd, "STOR") == 0)
{
if(data_s > 0)
{
if(split == 1)
{
char filename[256];
absPath(filename, param, cwd);

ssend(conn_s, "150 Accepted data connection\r\n");

if(recvfile(data_s, filename, BUFFER_SIZE, (s64)rest) == 0)
{
ssend(conn_s, "226 Transfer complete\r\n");
}
else
{
ssend(conn_s, "451 Transfer failed\r\n");
}
}
else
{
ssend(conn_s, "501 No file specified\r\n");
}
}
else
{
ssend(conn_s, "425 No data connection\r\n");
}
}
else
if(strcasecmp(cmd, "RETR") == 0)
{
if(data_s > 0)
{
if(split == 1)
{
char filename[256];
absPath(filename, param, cwd);

if(exists(filename) == 0)
{
ssend(conn_s, "150 Accepted data connection\r\n");

if(sendfile(data_s, filename, BUFFER_SIZE, (s64)rest) == 0)
{
ssend(conn_s, "226 Transfer complete\r\n");
}
else
{
ssend(conn_s, "451 Transfer failed\r\n");
}
}
else
{
ssend(conn_s, "550 File does not exist\r\n");
}
}
else
{
ssend(conn_s, "501 No file specified\r\n");
}
}
else
{
ssend(conn_s, "425 No data connection\r\n");
}
}
else
if(strcasecmp(cmd, "PWD") == 0)
{
sprintf(buffer, "257 \"%s\" is the current directory\r\n", cwd);
ssend(conn_s, buffer);
}
else
if(strcasecmp(cmd, "TYPE") == 0)
{
ssend(conn_s, "200 TYPE command successful\r\n");
dataactive = 1;
}
else
if(strcasecmp(cmd, "REST") == 0)
{
if(split == 1)
{
ssend(conn_s, "350 REST command successful\r\n");
rest = atoi(param);
dataactive = 1;
}
else
{
ssend(conn_s, "501 No restart point\r\n");
}
}
else
if(strcasecmp(cmd, "DELE") == 0)
{
if(split == 1)
{
char filename[256];
absPath(filename, param, cwd);

if(lv2FsUnlink(filename) == 0)
{
ssend(conn_s, "250 File successfully deleted\r\n");
}
else
{
ssend(conn_s, "550 Cannot delete file\r\n");
}
}
else
{
ssend(conn_s, "501 No filename specified\r\n");
}
}
else
if(strcasecmp(cmd, "MKD") == 0)
{
if(split == 1)
{
char filename[256];
absPath(filename, param, cwd);

if(lv2FsMkdir(filename, 0755) == 0)
{
sprintf(buffer, "257 \"%s\" was successfully created\r\n", param);
ssend(conn_s, buffer);
}
else
{
ssend(conn_s, "550 Cannot create directory\r\n");
}
}
else
{
ssend(conn_s, "501 No filename specified\r\n");
}
}
else
if(strcasecmp(cmd, "RMD") == 0)
{
if(split == 1)
{
char filename[256];
absPath(filename, param, cwd);

if(lv2FsRmdir(filename) == 0)
{
ssend(conn_s, "250 Directory was successfully removed\r\n");
}
else
{
ssend(conn_s, "550 Cannot remove directory\r\n");
}
}
else
{
ssend(conn_s, "501 No filename specified\r\n");
}
}
else
if(strcasecmp(cmd, "RNFR") == 0)
{
if(split == 1)
{
absPath(rnfr, param, cwd);

if(exists(rnfr) == 0)
{
ssend(conn_s, "350 RNFR accepted - ready for destination\r\n");
}
else
{
ssend(conn_s, "550 RNFR failed - file does not exist\r\n");
}
}
else
{
ssend(conn_s, "501 No file specified\r\n");
}
}
else
if(strcasecmp(cmd, "RNTO") == 0)
{
if(split == 1)
{
char rnto[256];
absPath(rnto, param, cwd);

if(lv2FsRename(rnfr, rnto) == 0)
{
ssend(conn_s, "250 File was successfully renamed or moved\r\n");
}
else
{
ssend(conn_s, "550 Cannot rename or move file\r\n");
}
}
else
{
ssend(conn_s, "501 No file specified\r\n");
}
}
else
if(strcasecmp(cmd, "SITE") == 0)
{
if(split == 1)
{
char param2[256];
split = ssplit(param, cmd, 31, param2, 255);

if(strcasecmp(cmd, "CHMOD") == 0)
{
if(split == 1)
{
char temp[4], filename[256];
split = ssplit(param2, temp, 3, filename, 255);

if(split == 1)
{
char perms[5];
sprintf(perms, "0%s", temp);

// jjolano epic failed here :D (problem was ONLY the absolute path..)
char absFilePath[256]; // place-holder for absolute path
absPath(absFilePath, filename, cwd); // making sure that we use the absolute path

//tested and working for both dir and files :0)
if(lv2FsChmod(absFilePath, strtol(perms, NULL, 8)) == 0) //cleaned up
{
ssend(conn_s, "250 File permissions successfully set\r\n");
}
else
{
ssend(conn_s, "550 Cannot set file permissions\r\n");
}
}
else
{
ssend(conn_s, "501 Not enough parameters\r\n");
}
}
else
{
ssend(conn_s, "501 No parameters given\r\n");
}
}
else
if(strcasecmp(cmd, "HELP") == 0)
{
ssend(conn_s, "214-Special Dragon commands:\r\n");
ssend(conn_s, " SITE PASSWD <newpassword> - Change your password\r\n");
ssend(conn_s, " SITE EXITAPP - Remotely quit DragonFTP\r\n");
ssend(conn_s, " SITE HELP - Show this message\r\n");
ssend(conn_s, "214 End\r\n");
}
else
if(strcasecmp(cmd, "PASSWD") == 0)
{
if(split == 1)
{
Lv2FsFile fd;
u64 written;

if(lv2FsOpen("/dev_hdd0/game/DARK0RSX5/USRDIR/passwd", LV2_O_WRONLY | LV2_O_CREAT, &fd, 0, NULL, 0) == 0)
{
lv2FsWrite(fd, param2, strlen(param2), &written);
sprintf(buffer, "200 New password: %s\r\n", param2);
ssend(conn_s, buffer);
}
else
{
ssend(conn_s, "550 Cannot change FTP password\r\n");
}

lv2FsClose(fd);
}
else
{
ssend(conn_s, "501 No password given\r\n");
}
}
else
if(strcasecmp(cmd, "EXITAPP") == 0)
{
ssend(conn_s, "221 Exiting DragonFTP\r\n");
exitapp = 1;
}
else
{
ssend(conn_s, "500 Unknown SITE command\r\n");
}
}
else
{
ssend(conn_s, "501 No SITE command specified\r\n");
}
}
else
if(strcasecmp(cmd, "NOOP") == 0)
{
ssend(conn_s, "200 NOOP command successful\r\n");
}
else
if(strcasecmp(cmd, "NLST") == 0)
{
if(data_s > 0)
{
char tempcwd[256];
strcpy(tempcwd, cwd);

if(split == 1)
{
absPath(tempcwd, param, cwd);
}

void listcb(Lv2FsDirent entry)
{
sprintf(buffer, "%s\r\n", entry.d_name);
ssend(data_s, buffer);
}

ssend(conn_s, "150 Accepted data connection\r\n");

if(slist(isDir(tempcwd) ? tempcwd : cwd, listcb) >= 0)
{
ssend(conn_s, "226 Transfer complete\r\n");
}
else
{
ssend(conn_s, "550 Cannot access directory\r\n");
}
}
else
{
ssend(conn_s, "425 No data connection\r\n");
}
}
else
if(strcasecmp(cmd, "MLST") == 0)
{
char tempcwd[256];
strcpy(tempcwd, cwd);

if(split == 1)
{
absPath(tempcwd, param, cwd);
}

void listcb(Lv2FsDirent entry)
{
char filename[256];
absPath(filename, entry.d_name, cwd);

Lv2FsStat buf;
lv2FsStat(filename, &buf);

char timebuf[16];
strftime(timebuf, 15, "%Y%m%d%H%M%S", localtime(&buf.st_mtime));

char dirtype[2];
if(strcmp(entry.d_name, ".") == 0)
{
strcpy(dirtype, "c");
}
else
if(strcmp(entry.d_name, "..") == 0)
{
strcpy(dirtype, "p");
}
else
{
dirtype[0] = '\0';
}

sprintf(buffer, " type=%s%s;siz%s=%llu;modify=%s;UNIX.mode=0%i%i%i;UNIX.uid=root;UNIX.gid=root; %s\r\n",
dirtype, ((buf.st_mode & S_IFDIR) != 0) ? "dir" : "file",
((buf.st_mode & S_IFDIR) != 0) ? "d" : "e", (unsigned long long)buf.st_size, timebuf,
(((buf.st_mode & S_IRUSR) != 0) * 4 +
((buf.st_mode & S_IWUSR) != 0) * 2 +
((buf.st_mode & S_IXUSR) != 0) * 1),
(((buf.st_mode & S_IRGRP) != 0) * 4 +
((buf.st_mode & S_IWGRP) != 0) * 2 +
((buf.st_mode & S_IXGRP) != 0) * 1),
(((buf.st_mode & S_IROTH) != 0) * 4 +
((buf.st_mode & S_IWOTH) != 0) * 2 +
((buf.st_mode & S_IXOTH) != 0) * 1),
entry.d_name);

ssend(conn_s, buffer);
}

ssend(conn_s, "250-Directory Listing\r\n");
slist(isDir(tempcwd) ? tempcwd : cwd, listcb);
ssend(conn_s, "250 End\r\n");
}
else
if(strcasecmp(cmd, "QUIT") == 0 || strcasecmp(cmd, "BYE") == 0)
{
ssend(conn_s, "221 Bye!\r\n");
connactive = 0;
}
else
if(strcasecmp(cmd, "FEAT") == 0)
{
ssend(conn_s, "211-Extensions supported:\r\n");

static char *feat_cmds[] =
{
"PASV",
"PORT",
"SIZE",
"CDUP",
"MLSD",
"MLST type*;size*;modify*;UNIX.mode*;UNIX.uid*;UNIX.gid*;",
"REST STREAM",
"SITE CHMOD",
"SITE PASSWD",
"SITE EXITAPP"
};

const int feat_cmds_count = sizeof(feat_cmds) / sizeof(char *);

for(int i = 0; i < feat_cmds_count; i++)
{
sprintf(buffer, " %s\r\n", feat_cmds[i]);
ssend(conn_s, buffer);
}

ssend(conn_s, "211 End\r\n");
}
else
if(strcasecmp(cmd, "SIZE") == 0)
{
if(split == 1)
{
char filename[256];
absPath(filename, param, cwd);

Lv2FsStat buf;
if(lv2FsStat(filename, &buf) == 0)
{
sprintf(buffer, "213 %llu\r\n", (unsigned long long)buf.st_size);
ssend(conn_s, buffer);
}
else
{
ssend(conn_s, "550 File does not exist\r\n");
}
}
else
{
ssend(conn_s, "501 No file specified\r\n");
}
}
else
if(strcasecmp(cmd, "SYST") == 0)
{
ssend(conn_s, "215 UNIX Type: L8\r\n");
}
else
if(strcasecmp(cmd, "USER") == 0 || strcasecmp(cmd, "PASS") == 0)
{
ssend(conn_s, "230 You are already logged in\r\n");
}
else
{
ssend(conn_s, "500 Unrecognized command\r\n");
}

if(dataactive == 1)
{
dataactive = 0;
}
else
{
sclose(&data_s);
}
}
else
{
// available commands when not logged in
if(strcasecmp(cmd, "USER") == 0)
{
if(split == 1)
{
if(DISABLE_PASS == 1)
{
ssend(conn_s, "230 Welcome to DragonFTP!\r\n");
loggedin = 1;
}
else
{
strcpy(user, param);
sprintf(buffer, "331 User %s OK. Password required\r\n", param);
ssend(conn_s, buffer);
}
}
else
{
ssend(conn_s, "501 No user specified\r\n");
}
}
else
if(strcasecmp(cmd, "PASS") == 0)
{
if(split == 1)
{
if(strcmp(D_USER, user) == 0 && strcmp(userpass, param) == 0)
{
ssend(conn_s, "230 Welcome to DragonFTP!\r\n");
loggedin = 1;
}
else
{
ssend(conn_s, "430 Invalid username or password\r\n");
}
}
else
{
ssend(conn_s, "501 No password given\r\n");
}
}
else
if(strcasecmp(cmd, "QUIT") == 0 || strcasecmp(cmd, "BYE") == 0)
{
ssend(conn_s, "221 Bye!\r\n");
connactive = 0;
}
else
{
ssend(conn_s, "530 Not logged in\r\n");
}
}
}

sclose(&conn_s);
sclose(&data_s);

sys_ppu_thread_exit(0);
}

static void handleconnections(u64 unused)
{
int list_s = slisten(FTPPORT, 5);

if(list_s > 0)
{
int conn_s;
sys_ppu_thread_t id;

while(exitapp == 0)
{
if((conn_s = accept(list_s, NULL, NULL)) > 0)
{
sys_ppu_thread_create(&id, handleclient, (u64)conn_s, 1500, 0x1000 + BUFFER_SIZE, 0, "ClientCmdHandler");
//usleep(100000); // this should solve some connection issues
}
}

sclose(&list_s);
}
sys_ppu_thread_exit(0);
}

void screenSaverOPEN() {
      if(screensaver) {
       if(sec > keep_value) {
         switch(current_background) {
         case 0:
         deadrsx_offset(background_offset, -51, -69, 950, 650);
         break;
         case 1:
        deadrsx_sprite(bsprite_offset, -51, -69, 950, 650, 200, 100, 2, 0, 3, 1);
        deadrsx_sprite(bsprite_offset, -51, -69, 950, 650, 200, 100, 2, 0, 3, 1);
        deadrsx_sprite(bsprite_offset, -51, -69, 950, 650, 200, 100, 2, 0, 3, 1);
        deadrsx_sprite(bsprite_offset, -51, -69, 950, 650, 200, 100, 2, 0, 3, 1);
         break;
         }
       }
      }
}

void drawFrame(int buffer, long frame) {

	realityViewportTranslate(context, 0.0, 0.0, 0.0, 0.0);
	realityViewportScale(context, 1.0, 1.0, 1.0, 0.0); 

        deadrsx_scale();

	realityZControl(context, 0, 1, 1); // disable viewport culling

	// Enable alpha blending.
	realityBlendFunc(context,
		NV30_3D_BLEND_FUNC_SRC_RGB_SRC_ALPHA |
		NV30_3D_BLEND_FUNC_SRC_ALPHA_SRC_ALPHA,
		NV30_3D_BLEND_FUNC_DST_RGB_ONE_MINUS_SRC_ALPHA |
		NV30_3D_BLEND_FUNC_DST_ALPHA_ZERO);
	realityBlendEquation(context, NV40_3D_BLEND_EQUATION_RGB_FUNC_ADD |
		NV40_3D_BLEND_EQUATION_ALPHA_FUNC_ADD);
	realityBlendEnable(context, 1);

	realityViewport(context, res.width, res.height);

	setupRenderTarget(buffer);

        deadrsx_background(background_color);

	// and the depth clear value
	realitySetClearDepthValue(context, 0xffff);
	// Clear the buffers
	realityClearBuffers(context, REALITY_CLEAR_BUFFERS_COLOR_R |
				     REALITY_CLEAR_BUFFERS_COLOR_G |
				     REALITY_CLEAR_BUFFERS_COLOR_B |
				     NV30_3D_CLEAR_BUFFERS_COLOR_A |
				     NV30_3D_CLEAR_BUFFERS_STENCIL |
				     REALITY_CLEAR_BUFFERS_DEPTH);

	// Load shaders, because the rsx won't do anything without them.
	realityLoadVertexProgram_old(context, &nv40_vp);
	realityLoadFragmentProgram_old(context, &nv30_fp); 

        switch(app_state) {
        case 1:

 if(!debug_app) {
        background_color = COLOR_WHITE;
        deadrsx_sprite(sprite_offset, 0.0, 0.0, 847, 511, 200, 200, 1, 1, 2, 2);
        deadrsx_sprite(bsprite_offset, 0.0, 0.0, 847, 511, 200, 100, 2, 0, 3, 1);
        if(debug_app) { drawText(font_offset, 15, fps_char, 0, 0); }
        drawText(font_offset, 38, "DragonFTP Server 0.25", 45, 50);
        drawText(font_offset, 15,  status, 103, 98);
        drawText(font_offset, 25, "Options", 100, 125);
        drawText(font_offset, 25, "Exit Application", 100, 175);
        drawText(font_offset, 30, "Visit www.ps3sdk.com for Updates from DarkhackerPS3", 15, 467);

        if(current_selection == 1) {

        }else if(current_selection == 2) {
        deadrsx_sprite(bsprite_offset, 50, 113, 50, 50, 200, 100, 0, 0, 2, 1);
//        deadrsx_sprite(sprite_offset, 50, 113, 50, 50, 200, 200, 0, 1, 2, 2); // 163
        }else{
        deadrsx_sprite(bsprite_offset, 50, 163, 50, 50, 200, 100, 0, 0, 2, 1);
//        deadrsx_sprite(sprite_offset, 50, 163, 50, 50, 200, 200, 0, 1, 2, 2);
        }

 }else{

        background_color = COLOR_WHITE;
        deadrsx_sprite(sprite_offset, 0.0, 0.0, 847, 511, 200, 200, 1, 1, 2, 2);
        deadrsx_sprite(bsprite_offset, 0.0, 0.0, 847, 511, 200, 100, 2, 0, 3, 1);
        if(debug_app) { drawText(font_offset, 15, fps_char, 0, 0); }
        drawText(font_offset, 38, "DragonFTP Server 0.25", 45, 50);
        drawText(font_offset, 15,  status, 103, 98);
        drawText(font_offset, 25, "Options", 100, 125);
        drawText(font_offset, 25, "Debug", 100, 175);
        drawText(font_offset, 25, "Exit Application", 100, 225);
        drawText(font_offset, 30, "Visit www.ps3sdk.com for Updates from DarkhackerPS3", 15, 467);

        if(current_selection == 1) {

        }else if(current_selection == 2) {
        deadrsx_sprite(bsprite_offset, 50, 113, 50, 50, 200, 100, 0, 0, 2, 1);
        }else if(current_selection == 3){
        deadrsx_sprite(bsprite_offset, 50, 163, 50, 50, 200, 100, 0, 0, 2, 1);
        }else if(current_selection == 4){ 
        deadrsx_sprite(bsprite_offset, 50, 213, 50, 50, 200, 100, 0, 0, 2, 1);
        }


 }
        screenSaverOPEN(); 
 
        break;
        case 2:
        background_color = COLOR_BLACK;
        break;
        case 3:
        // options screen
        background_color = COLOR_WHITE;
        deadrsx_sprite(sprite_offset, 0.0, 0.0, 847, 511, 200, 200, 1, 1, 2, 2);
        deadrsx_sprite(bsprite_offset, 0.0, 0.0, 847, 511, 200, 100, 2, 0, 3, 1);
        if(debug_app) { drawText(font_offset, 15, fps_char, 0, 0); }
        drawText(font_offset, 38, "DragonFTP Options", 45, 50);

 
        drawText(font_offset, 25, "FTP Account :", 100, 125);
        deadrsx_sprite(sprite_offset, 400, 118, 40, 40, 200, 200, 1, 0, 2, 2); // on box
        deadrsx_sprite(sprite_offset, 600, 118, 40, 40, 200, 200, 1, 0, 2, 2); // off box

        if(DISABLE_PASS == 0) {
        deadrsx_sprite(bsprite_offset, 405, 123, 30, 30, 200, 100, 2, 0, 3, 1); // selected on  box
        }else{
        deadrsx_sprite(bsprite_offset, 605, 123, 30, 30, 200, 100, 2, 0, 3, 1); // selected off box
        }
        drawText(font_offset, 30, "On", 460, 125);
        drawText(font_offset, 30, "Off", 660, 125);

        drawText(font_offset, 25, "ScreenSaver :", 100, 175);
        deadrsx_sprite(sprite_offset, 400, 168, 40, 40, 200, 200, 1, 0, 2, 2); // on box
        deadrsx_sprite(sprite_offset, 600, 168, 40, 40, 200, 200, 1, 0, 2, 2); // off box

        if(screensaver == 1) {
        deadrsx_sprite(bsprite_offset, 405, 173, 30, 30, 200, 100, 2, 0, 3, 1); // selected on  box
        }else{
        deadrsx_sprite(bsprite_offset, 605, 173, 30, 30, 200, 100, 2, 0, 3, 1); // selected off box
        }
        drawText(font_offset, 30, "On", 460, 175);
        drawText(font_offset, 30, "Off", 660, 175);


        drawText(font_offset, 25, "Dev_Dragon :", 100, 225);
        deadrsx_sprite(sprite_offset, 400, 218, 40, 40, 200, 200, 1, 0, 2, 2); // on box
        deadrsx_sprite(sprite_offset, 600, 218, 40, 40, 200, 200, 1, 0, 2, 2); // off box

        if(dev_dragon == 1) {
        deadrsx_sprite(bsprite_offset, 405, 223, 30, 30, 200, 100, 2, 0, 3, 1); // selected on  box
        }else{
        deadrsx_sprite(bsprite_offset, 605, 223, 30, 30, 200, 100, 2, 0, 3, 1); // selected off box
        }
        drawText(font_offset, 30, "Mount", 460, 225);
        drawText(font_offset, 30, "Unmount", 660, 225);

        drawText(font_offset, 25, "ScreenSaver Background :", 100, 275);
        deadrsx_sprite(sprite_offset, 400, 268, 40, 40, 200, 200, 1, 0, 2, 2); // on box
        deadrsx_sprite(sprite_offset, 600, 268, 40, 40, 200, 200, 1, 0, 2, 2); // off box

        if(current_background == 1) {
        deadrsx_sprite(bsprite_offset, 405, 273, 30, 30, 200, 100, 2, 0, 3, 1); // selected on  box
        }else{
        deadrsx_sprite(bsprite_offset, 605, 273, 30, 30, 200, 100, 2, 0, 3, 1); // selected off box
        }
        drawText(font_offset, 30, "Black", 460, 275);
        drawText(font_offset, 30, "Dragon", 660, 275);

        drawText(font_offset, 25, "ScreenSaver Timer :", 100, 325);
        char timerholdok[100];
        sprintf(timerholdok, "%i Minutes", minbackground);
        drawText(font_offset, 30, timerholdok, 390,325);          

        drawText(font_offset, 25, "Return to Menu", 100, 375);

        switch(option_down) {
        case 1:
        deadrsx_sprite(bsprite_offset, 50, 113, 50, 50, 200, 100, 0, 0, 2, 1);
        break;
        case 2:
        deadrsx_sprite(bsprite_offset, 50, 163, 50, 50, 200, 100, 0, 0, 2, 1);
        break;
        case 3:
        deadrsx_sprite(bsprite_offset, 50, 213, 50, 50, 200, 100, 0, 0, 2, 1);
        break;
        case 4:
        deadrsx_sprite(bsprite_offset, 50, 263, 50, 50, 200, 100, 0, 0, 2, 1);
        break;
        case 5:
        deadrsx_sprite(bsprite_offset, 50, 313, 50, 50, 200, 100, 0, 0, 2, 1);
        break;
        case 6:
        deadrsx_sprite(bsprite_offset, 50, 363, 50, 50, 200, 100, 0, 0, 2, 1);
        break;
        }

        drawText(font_offset, 30, "Feel free to Support DarkhackerPS3 at www.PS3SDK.com", 5, 467); // fixed spelling error people cool it :D simple typo 

        // keep screenSaverOPEN() below the screen you make because it need to overlay on top of it using buffer
        screenSaverOPEN();

        break;
        case 4:
        if(debug_app) {
        background_color = COLOR_WHITE;
        deadrsx_sprite(sprite_offset, 0.0, 0.0, 847, 511, 200, 200, 1, 1, 2, 2);
        deadrsx_sprite(bsprite_offset, 0.0, 0.0, 847, 511, 200, 100, 2, 0, 3, 1);
        drawText(font_offset, 15, fps_char, 0, 0);
        drawText(font_offset, 38, "DragonFTP Debug Information", 45, 50);

		if(debugCount > 0)
                {
                    int lineStartPosY = 100; //must be corrected if you add more permanent lines
                    for(int i = 0; (i < debugCount) && (i < DEBUGARRAYMAXLINES); i++)
                    {
                        if(debugArray[i] != NULL)
                        {
                            drawText(font_offset, 24, debugArray[i], 45, lineStartPosY);
                            lineStartPosY = lineStartPosY + 25;
                        }
                    }
                }

        }
        break;
        }
  
}
void check_devdragon() {
dev_dragon = exists("/dev_dragon") == 0;
}

void mount_dragon(){
//dev_dragon = 1;
Lv2Syscall8(837, (u64)"CELL_FS_IOS:BUILTIN_FLSH1", (u64)"CELL_FS_FAT", (u64)"/dev_dragon", 0, 0, 0, 0, 0);
}

void unmount_dragon(){
//dev_dragon = 0;
Lv2Syscall1(838, (u64)"/dev_dragon");
}

void appExit() {
  sysUnregisterCallback(EVENT_SLOT0);
  netDeinitialize();
  exit(0);
}

static void eventHandle(u64 status, u64 param, void * userdata) {
    (void)param;
    (void)userdata;
	if(status == EVENT_REQUEST_EXITAPP){
	  appExit();
	}else if(status == EVENT_MENU_OPEN){
          state_holder = app_state;
	  app_state = 2;
	}else if(status == EVENT_MENU_CLOSE){
	  app_state = state_holder;
	}else if(status == EVENT_DRAWING_BEGIN){
	}else if(status == EVENT_DRAWING_END){
	}else{
		printf("Unhandled event: %08llX\n", (unsigned long long int)status);
	}
}

void loading() {
   deadrsx_loadfile("/dev_hdd0/game/DARK0RSX5/USRDIR/Image/font.png", font_image, &font_offset);
   deadrsx_loadfile("/dev_hdd0/game/DARK0RSX5/USRDIR/Image/sprite.png", sprite_image, &sprite_offset);
   deadrsx_loadfile("/dev_hdd0/game/DARK0RSX5/USRDIR/Image/backup_sprite.png", bsprite_image, &bsprite_offset);
   deadrsx_loadfile("/dev_hdd0/game/DARK0RSX5/USRDIR/Image/background.png", background_image, &background_offset);
}
void update_movement() {
 lastsec = sec;
 movement = 1;
}
void getTime() {
        lv2GetCurrentTime(&sec, &nsec);
        update_value = minbackground * 60;
        keep_value = lastsec + update_value;
        if(set == 0) {
        set = 1;
        lastsec = sec;
        frame_tick = sec;
        }
}
void fps_counter() {
if(sec > frame_tick) {
frame_tick = sec;
fps_flip = fps_holder;
fps_holder = 0;
}
fps_holder += 1;
sprintf(fps_char, "fps : %i", fps_flip);
}

void ps_controller() {
	ioPadGetInfo(&padinfo);
	for(ci=0; ci<MAX_PADS; ci++){
        
           switch(app_state) {
           case 1:
			if(padinfo.status[ci]){
				ioPadGetData(ci, &paddata);
                                if(paddata.BTN_CROSS){
                                update_movement();
                                if(btnPressed == 0) {
                                btnPressed = 1;
                                 if(debug_app) {
                                  switch(current_selection) {
                                  case 1:
                                  // not used anymore
                                  break;
                                  case 2:
                                  sleep(1);
                                  app_state = 3;
                                  break;
                                  case 3:
                                  sleep(1);
                                  app_state = 4;
                                  break;
                                  case 4:
                                  sleep(1);                                  
                                  appExit();
                                  break;
                                  }
                                 }else{

                                  switch(current_selection) {
                                  case 1:
                                  // not used anymore
                                  break;
                                  case 2:
                                  sleep(1);
                                  app_state = 3;
                                  break;
                                  case 3:
                                  sleep(1);
                                  appExit();
                                  break;
                                  }

                                 }
                                }
  
				}else if(paddata.BTN_UP) {
                                update_movement();
                                if(btnPressed == 0){
                                btnPressed = 1;
                                if(current_selection > 2) { current_selection -= 1; }
                                }

                                }else if(paddata.BTN_DOWN) {
                                update_movement();
                                 
                                if(debug_app) {
                                if(btnPressed == 0){
                                btnPressed = 1;
                                if(current_selection < 4) { current_selection += 1; }
                                }
                                }else{
                                if(btnPressed == 0){
                                btnPressed = 1;
                                if(current_selection < 3) { current_selection += 1; }
                                }
                                }

                                }else if(paddata.BTN_RIGHT) {
                                update_movement();

                                }else if(paddata.BTN_LEFT){
                                update_movement();

                                }else{
                                btnPressed = 0;
                                }
			}

           break;

           case 3:

			if(padinfo.status[ci]){
				ioPadGetData(ci, &paddata);

                                if(paddata.BTN_CROSS){
                                update_movement();
                                if(btnPressed == 0) {
                                btnPressed = 1;
                                switch(option_down) { 
      
                                case 6:
                                sleep(1);
                                app_state = 1;
                                break;   
                    
                                }

                                }
  
				}else if(paddata.BTN_UP) {
                                update_movement();
                                if(btnPressed == 0){
                                btnPressed = 1;
                                if(option_down > 1) {
                                option_down -= 1;
                                }

                                }

                                }else if(paddata.BTN_DOWN) {
                                update_movement();
                                if(btnPressed == 0){
                                btnPressed = 1;
                                if(option_down < 6) {
                                option_down += 1;
                                }

                                }

                                }else if(paddata.BTN_RIGHT) {
                                update_movement();
                                switch(option_down) {
                                case 1:
                                DISABLE_PASS = 1;
                                break;
                            
                                case 2:
                                screensaver = 0;
                                break;
       
                                case 3:
                                unmount_dragon();
                                break;
                                
                                case 4:
                                current_background = 0;
                                break;
 
                                case 5:
                                if(btnPressed == 0) {
                                btnPressed = 1;
                                if(minbackground < 120) { // timer could be set to 2 hour that good 
                                minbackground += 1;
                                }
                                }
                                break;
                                }

                                }else if(paddata.BTN_LEFT){
                                update_movement();
                                switch(option_down) {
                                case 1:
                                DISABLE_PASS = 0;
                                break;
                            
                                case 2:
                                screensaver = 1;
                                break;

                                case 3:
                                mount_dragon();
                                break;
 
                                case 4:
                                current_background = 1;
                                break;

                                case 5:
                                if(btnPressed == 0) {
                                btnPressed = 1;
                                if(minbackground > 1) {
                                minbackground -= 1;
                                }
                                }
                                break;

                                }

                                }else{
                                btnPressed = 0;
                                }
			}

           break;
 
           case 4:
			if(padinfo.status[ci]){
				ioPadGetData(ci, &paddata);

                                if(paddata.BTN_CROSS){
                                app_state = 1; // returns to main_menu
                                }

                        }
           break;

           }     
			
	}
}

s32 main(s32 argc, const char* argv[])
{
        netInitialize();
        check_devdragon();
        sprintf(version, "Version %s", VERSION);
        strcpy(userpass, D_PASS);


	deadrsx_init();
	ioPadInit(7);
	sysRegisterCallback(EVENT_SLOT0, eventHandle, NULL);

        loading(); // this is where all are image loading happens 

	u32 *frag_mem = rsxMemAlign(256, 256);
	printf("frag_mem = 0x%08lx\n", (u64) frag_mem);
	realityInstallFragmentProgram_old(context, &nv30_fp, frag_mem);

	long frame = 0; 
	btnPressed = 0;

        sys_ppu_thread_t id;
        sys_ppu_thread_create(&id, handleconnections, 0, 1500, 0x1000, 0, "ServerConnectionHandler");
        sys_ppu_thread_create(&id, ipaddr_get, 0, 1500, 0x1000, 0, "RetrieveIPAddress");

	while(1){
                getTime(); // so we can have sleep function xD
                if(debug_app){fps_counter();} // see the frames per second going by xD
                check_devdragon(); // checks to see if dev_dragon mounted to update var to change the option on the screen
                ps_controller(); // controller input xD


                // do not put anything below this that not there already 
		waitFlip(); // Wait for the last flip to finish, so we can draw to the old buffer
		drawFrame(currentBuffer, frame++); // Draw into the unused buffer
		flip(currentBuffer); // Flip buffer onto screen
		currentBuffer = !currentBuffer;
		sysCheckCallback();

	}
        netDeinitialize();	
	return 0;
}

