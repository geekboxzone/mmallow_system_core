/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _INIT_INIT_H
#define _INIT_INIT_H

#include <sys/types.h>

#include <string>
#include <vector>

#include <cutils/list.h>
#include <cutils/iosched_policy.h>

struct command
{
        /* list of commands in an action */
    struct listnode clist;

    int (*func)(int nargs, char **args);

    int line;
    const char *filename;

    int nargs;
    char *args[1];
};

struct trigger {
    struct listnode nlist;
    const char *name;
};

struct action {
        /* node in list of all actions */
    struct listnode alist;
        /* node in the queue of pending actions */
    struct listnode qlist;
        /* node in list of actions for a trigger */
    struct listnode tlist;

    unsigned hash;

        /* list of actions which triggers the commands*/
    struct listnode triggers;
    struct listnode commands;
    struct command *current;
};

struct socketinfo {
    struct socketinfo *next;
    const char *name;
    const char *type;
    uid_t uid;
    gid_t gid;
    int perm;
    const char *socketcon;
};

struct svcenvinfo {
    struct svcenvinfo *next;
    const char *name;
    const char *value;
};

#define SVC_DISABLED       0x001  // do not autostart with class
#define SVC_ONESHOT        0x002  // do not restart on exit
#define SVC_RUNNING        0x004  // currently active
#define SVC_RESTARTING     0x008  // waiting to restart
#define SVC_CONSOLE        0x010  // requires console
#define SVC_CRITICAL       0x020  // will reboot into recovery if keeps crashing
#define SVC_RESET          0x040  // Use when stopping a process, but not disabling so it can be restarted with its class.
#define SVC_RC_DISABLED    0x080  // Remember if the disabled flag was set in the rc script.
#define SVC_RESTART        0x100  // Use to safely restart (stop, wait, start) a service.
#define SVC_DISABLED_START 0x200  // A start was requested but it was disabled at the time.
#define SVC_EXEC           0x400  // This synthetic service corresponds to an 'exec'.

#define NR_SVC_SUPP_GIDS 12    /* twelve supplementary groups */

#define COMMAND_RETRY_TIMEOUT 5

/* IMPORTANT: THE VALUE OF STORAGE_MEDIA MUST BE CONSISTANT WITH UBOOT */
#define EMMC_RETRY_COUNT 20
#define STORAGE_MEDIA "storagemedia=emmc"

struct service {
    void NotifyStateChange(const char* new_state);

        /* list of all services */
    struct listnode slist;

    char *name;
    const char *classname;

    unsigned flags;
    pid_t pid;
    time_t time_started;    /* time of last start */
    time_t time_crashed;    /* first crash within inspection window */
    int nr_crashed;         /* number of times crashed within window */

    uid_t uid;
    gid_t gid;
    gid_t supp_gids[NR_SVC_SUPP_GIDS];
    size_t nr_supp_gids;

    const char* seclabel;

    struct socketinfo *sockets;
    struct svcenvinfo *envvars;

    struct action onrestart;  /* Actions to execute on restart. */

    std::vector<std::string>* writepid_files_;

    /* keycodes for triggering this service via /dev/keychord */
    int *keycodes;
    int nkeycodes;
    int keychord_id;

    IoSchedClass ioprio_class;
    int ioprio_pri;

    int nargs;
    /* "MUST BE AT THE END OF THE STRUCT" */
    char *args[1];
}; /*     ^-------'args' MUST be at the end of this struct! */

extern bool waiting_for_exec;
extern struct selabel_handle *sehandle;
extern struct selabel_handle *sehandle_prop;

void build_triggers_string(char *name_str, int length, struct action *cur_action);

void handle_control_message(const char *msg, const char *arg);

struct service *service_find_by_name(const char *name);
struct service *service_find_by_pid(pid_t pid);
struct service *service_find_by_keychord(int keychord_id);
void service_for_each(void (*func)(struct service *svc));
void service_for_each_class(const char *classname,
                            void (*func)(struct service *svc));
void service_for_each_flags(unsigned matchflags,
                            void (*func)(struct service *svc));
void service_stop(struct service *svc);
void service_reset(struct service *svc);
void service_restart(struct service *svc);
void service_start(struct service *svc, const char *dynamic_args);
void property_changed(const char *name, const char *value);

int selinux_reload_policy(void);

void zap_stdio(void);

void register_epoll_handler(int fd, void (*fn)());

#endif	/* _INIT_INIT_H */
