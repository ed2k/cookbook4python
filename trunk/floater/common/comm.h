/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Floater Bridge Network.
 *
 * The Initial Developer of the Original Code is
 * Geoff Pike <pike@EECS.Berkeley.EDU>.
 * Portions created by the Initial Developer are Copyright (C) 1996-2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Lex Spoon <lex@cc.gatech.edu>
 *
 * Alternatively, the contents of this file may be used under the
 * terms of either the GNU General Public License Version 2 or later
 * (the "GPL"), in which case the provisions of the GPL are applicable
 * instead of those above. If you wish to allow use of your version of
 * this file only under the terms of the GPL, and not to allow others
 * to use your version of this file under the terms of the MPL,
 * indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the GPL. If
 * you do not delete the provisions above, a recipient may use your
 * version of this file under the terms of either the MPL or the GPL.
 * ***** END LICENSE BLOCK ***** */
#ifndef __COMM_H__
#define __COMM_H__

#include "message.h"

void broadcast(message *m);
void selfmessage(message *m);

#ifdef LOGINSERVER
void loginserver(char *);
#else
void tablehost(char *, char *);
#endif

/* hostname and IP address of machine we're on */
extern char *localIPaddr;

/* hostname and IP of the servers we are currently using; results are
 * garbagized strings */
char *loginservername(void);
char *loginserverport(void);
char *loginserveraddr(void);

char *resultservername(void);
char *resultserverport(void);
char *resultserveraddr(void);

void setserver(const char *name);

/* whether I am hosting a table */
extern fl_bool tablehostmode;


/* used as last argument to join() */
#define NUMTRIES 1

/* number of tries when trying to connect to the result server */
#define NUMRSTRIES 1

void login(char *name, char *password, fl_bool new);
int join(char *addr, char *port, char *capacity, int numtries);

void punt(char *bozo);
void unseat(char *bozo);
void showip(void);
void showparent(void);
void showchildren(void);
void listwho(void);
char **getwho(int *n);
void gettables(void);
void changepw(char *name, char *old, char *new);

void sever_all_communication(void);
void comm_reset(void);

void commsetup();

char *outputname(char *name);

#ifndef SERVER
extern char location[200];
#endif

#define LIMBO 0
#define CONNECTED 2
#define DISCONNECTING 3

#define prioruse(name) atoi(TclDo3("global usedname; info exists {usedname(", name, ")}"))
#define setprioruse(name) TclDo3("setprioruse {", name, "}")

void sendmessage(char *to, message *m);
void requestseat(int which);

void commresult(fl_bool competitive, message *m, char *handname);
void commseens(int delay);
void sendtoallchildren(message *m);

void describetable(char *note);
void redescribetable(void);

fl_bool tableaddr(char *t, char *addr, char *port);

void checktablelisttimeouts(void);
void periodictablereannounce(void);

void try_rejoin(void);

#ifndef SERVER
void updatelocation(void);
#endif


/* stuff to handle spec */
#define no_spectators(hand) (spec = FALSE, TclDo3("catch {gunset is_spec", (hand), "}"))
#define make_spec(name, hand) (((hand) == NULL || (name) == NULL) ? NULL : \
	TclDo5("gset {is_spec", (hand), "(", (name), ")} 1"))
#define is_spec(name, hand) (((hand) == NULL || (name) == NULL) ? FALSE : \
	atob(TclDo5("global is_spec; info exists {is_spec", (hand), \
		    "(", (name), ")}")))

#endif

