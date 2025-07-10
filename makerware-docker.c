// makerware.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

int main(int argc, char **argv) {
   struct stat st;
   char home_vol[512];
   char xauth_path[512];
   char id_buf[2048];
   char hostname[256];
   struct group *gr;
   int i;

   // use HOME from pw database instead of trusting HOME environment variable,
   // since this is used as a mount path and could be abused
   struct passwd *pw = getpwuid(getuid());
   if (!pw) {
      fprintf(stderr, "Error: Could not get user information for uid=%d\n", getuid());
      return 2;
   }
   // verify that pw_dir is an existing directory
   if (stat(pw->pw_dir, &st) != 0) {
      perror("stat");
      return 2;
   }
   if ((st.st_mode & S_IFMT) != S_IFDIR) {
      fprintf(stderr, "Error: user home is not a directory: '%s'\n", pw->pw_dir);
      return 2;
   }
   // create home volume specification
   if (snprintf(home_vol, sizeof(home_vol), "%s:%s", pw->pw_dir, pw->pw_dir) >= sizeof(home_vol)) {
      fprintf(stderr, "Error: home volume exceeds buffer length\n");
      return 2;
   }
   setenv("HOME", pw->pw_dir, 1);

   // set a default DISPLAY variable
   setenv("DISPLAY", ":0", 0);

   // Prepare XAUTH file
   if (snprintf(xauth_path, sizeof(xauth_path), "%s/.docker.xauth", pw->pw_dir) >= sizeof(xauth_path)) {
      fprintf(stderr, "Error: XAUTHORITY path exceeds buffer length\n");
      return 3;
   }
   FILE *f = fopen(xauth_path, "a");  // just touch it
   if (!f) {
      perror("touch(.docker.xauth)");
      return 3;
   }
   fclose(f);
   setenv("XAUTHORITY", xauth_path, 1);

   // Construct the format of the 'id' command using direct pw lookups
   char *pos= id_buf, *lim= id_buf+sizeof(id_buf);
   size_t len;
   int ngroups= getgroups(0, NULL);
   if (ngroups < 0) {
      perror("getgroups");
      return 4;
   }
   gid_t *groups= (gid_t*) malloc(ngroups * sizeof(gid_t));
   if (!groups) {
      perror("malloc");
      return 4;
   }
   if (getgroups(ngroups, groups) < 0) {
      perror("getgroups");
      return 4;
   }
   gr= getgrgid(pw->pw_gid);
   if (!gr) {
      fprintf(stderr, "getgrgid(%d) failed\n", pw->pw_gid);
      return 4;
   }
   len= snprintf(pos, lim - pos, "ID=uid=%d(%s) gid=%d(%s)", (int)pw->pw_uid, pw->pw_name, (int)gr->gr_gid, gr->gr_name);
   if (len >= (lim - pos)) {
      fprintf(stderr, "ID var exceeds buffer length\n");
      return 4;
   }
   pos += len;
   for (i= 0; i < ngroups; i++) {
      if (!(gr= getgrgid(groups[i]))) {
         fprintf(stderr, "getgrgid(%d) failed\n", groups[i]);
         return 4;
      }
      len= snprintf(pos, lim - pos, "%s%d(%s)", (i? "," : " groups="), (int)groups[i], gr->gr_name);
      if (len >= (lim - pos)) {
         fprintf(stderr, "ID var exceeds buffer length\n");
         return 4;
      }
      pos += len;
   }
   
   memset(hostname, 0, sizeof(hostname));
   if (gethostname(hostname, sizeof(hostname)) != 0 || hostname[sizeof(hostname)-1] != '\0') {
      perror("gethostname");
      return 5;
   }

   char *docker_args[] = {
      "docker", "run", "--rm",
      "--privileged",         // privileged required by conveyor service
      "--hostname", hostname, // hostname must match, for X11 access
      "-v", home_vol,         // mount user's home at identical path
      "-e", "HOME", "-w", pw->pw_dir,
      "-v", "/tmp/.X11-unix:/tmp/.X11-unix", "-e", "XAUTHORITY", "-e", "DISPLAY",
      "-e", id_buf,           // ID variable relays all user permissions into container
      "makerware:latest",
      NULL
   };
   int docker_argc= sizeof(docker_args) / sizeof(*docker_args) - 1; // not counting trailing NULL
   // If the user passed additional arguments, build a new argv with those on the end
   char **new_argv= docker_args;
   if (argc > 1) { // not counting argv[0] which is program name
      new_argv= (char**) malloc((docker_argc + argc-1 + 1) * sizeof(char*));
      for (i= 0; i < docker_argc; i++)
         new_argv[i]= docker_args[i];
      for (i= 1; i < argc; i++)
         new_argv[docker_argc + i-1]= argv[i];
      new_argv[docker_argc + argc-1]= NULL;
   }

   execvp("docker", new_argv);
   perror("exec docker failed");
   return 127;
}
