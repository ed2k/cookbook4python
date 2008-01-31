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
 *   Syela
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
/* #include <sys/types.h> */
/* #include <sys/socket.h> */
/* #include <netinet/in.h> */
/* #include <arpa/inet.h> */
/* #include <netdb.h> */
/* #include <sys/param.h> */
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "floater.h"
#include "comm.h"
#include "br.h"
#include "deal.h"
#include "calendar.h"
#include "UI.h"


/* see documentation/internals/network.txt for information on the design */


/* This code is not very specific to bridge.  Anything relating to lho
   or rho or a compass direction is bridge related, and a few things
   are bracketed by comments saying "bridge specific."  The rest is
   generic.  */

#define isspec(x) FALSE

#define BOGUS_NAME " - BOGUS - "

#ifdef LOGINSERVER
#define MAXTCHILDREN 1
#else
#define MAXTCHILDREN 4
#endif

/* if I become a table server, do I try to connect to the table tree? */
#define jointableservertree TclBool("jointableservertree")

char myname[FLOATER_MAXNAME+1] = {'\0'};
char myoutputname[FLOATER_MAXNAME+1] = {'\0'};
semistatic char *mytabledesc = NULL; /* number of players, etc., plus the note */
semistatic char *mynote = NULL; /* just the note */

char tablehostname[FLOATER_MAXNAME+1] = {'?', '\0'};
semistatic char tablehostaddr[MAXADDRLEN+1] = {'\0'};
semistatic char tablehostport[MAXPORTLEN+1] = {'\0'};
semistatic char tablerootname[FLOATER_MAXNAME+1] = {'?', '\0'};
semistatic char tablerootaddr[MAXADDRLEN+1] = {'\0'};
semistatic char tablerootport[MAXPORTLEN+1] = {'\0'};

#ifndef SERVER
char location[200] = {'\0'};
#endif

fl_bool triedtologin = FALSE;
fl_bool loggedin = FALSE;

/* whether I am root of the table tree */
semistatic fl_bool tablerootmode = FALSE;

fl_bool tablehostmode = FALSE;

#define accepting_tableservers tablehostmode

#define nokibitzers TclBool("nokibitzers")
#define accepting_kibitzers (!nokibitzers && tablehostmode && \
			     (state == CONNECTED))

#define if_not_connected_reset_tablehostname() \
     if (state != CONNECTED) \
       tablehostname[0] = tablehostport[0] = tablehostaddr[0] = '\0';


int state = LIMBO;

char *localIPaddr = NULL;

/* port we're listening on locally */
semistatic char *localport = NULL;

semistatic char *Floater_hostname = NULL;

semistatic char *connstr = NULL;

/* typedef int (*rejoinptr)(char *, char *, char *, int); */

static char rejoinaddr[MAXADDRLEN];
static char rejoinport[MAXPORTLEN];
static char rejoincap[2];
static int rejoinnumtries = NUMTRIES;

static stringlist *who = NULL; /* who is at the table */

/* if we get a JOIN_ELSEWHERE message, do we believe it? */
static fl_bool expecting_join_elsewhere = FALSE;

/* When changing your password, new password stored here */
static char *newpw = NULL;

/* When changing your email, new email address stored here */
static char *newemail = NULL;

/* a few forward declarations */
static void sendstrmessage(char *to, char *s);
static char *msgtotext(message *m);
static void rebroadcast(message *m);
static void handlemessage(message *m, char *msg);
static void announce_who(void);
static void announce_tables(void);
static void rejoin(void);
static void setmyname(char *newname, fl_bool loggedin);
static void disconnect_all(void);
static char *connect_to_resultserver(void);
static int openport(char *port);
static int opennormalport(void);

#define defaultnote TclDo("gset defaultnote")

#define time_to_rejoin() atoi(TclDo("rejoinnow"))
#define reset_time_to_rejoin() TclDo("reset_rejoinnow")
#define time_to_find_rho() atoi(TclDo("findrhonow"))
#define reset_time_find_rho() TclDo("reset_find_rho")

#define isparentfile(x) ((parent != NULL) && streq((x), parent->connstr))
#define isparentname(x) ((parent != NULL) && streq((x), parent->name))
#define iskchildname(x) (namemember(x, kchildren))
#define iskchildfile(x) (filemember(x, kchildren))
#define istchildname(x) (namemember(x, tchildren))
#define istchildfile(x) (filemember(x, tchildren))

#ifdef LOGINSERVER
#define testConn(IPaddr, port, s) \
  tempcat((s), TclDo5("testConn {", (IPaddr), "} {", (port) , "}"))

#define nameinuse(name) \
  atoi(TclDo3("global nameinuse; info exists {nameinuse(", upcase(name), ")}"))



char *testConn2(char *clientversion, char *IPaddr, char *port, char *s)
{
  if (streq(clientversion, "8") || streq(clientversion, "8x"))
    return testConn(IPaddr, port, s);
  else {
    TclDo5("testConnInBackground {", (IPaddr), "} {", (port) , "}");
    return s;
  }
}

void newaccount(char *name, char *email)
{
  char password[9], bits[48];
  int i;
  char *upname = upcase(name); 

  /* put 48 random bits in bits[] */
  churnsimpleseed(name);
  putbits(unifmod(1 << 24), 24, bits);
  putbits(unifmod(1 << 24), 24, bits + 24);
  
  /* Convert 48 random bits into 8 printable characters */
  put6atatime(bits, 48, password);
/*  password[8] = '\0';*/
  password[5] = '\0';
  for (i = 0; i < 5; i++)
    if (password[i] == '0' || password[i] == 'O') password[i] = '3';
    else if (password[i] == '1' || password[i] == 'l') password[i] = '4';

  TclDo3("permset {nameinuse(", upname, ")} 1");
  TclDo3("permdo {lappend names {", name, "}}");
  TclDo7("newaccount {", name, "} {", email, "} {", password, "}");
}
/* #define correctly_capitalized_name(name) TclDo3("name_fetch {", (name), "}") */
/* #define password(name) TclDo3("password_fetch {", (name), "}") */
#define password(name) TclDo3("tryget {pw(", (name), ")}")
#define email_info(name) TclDo3("tryget_nocase email {", (name), "}")
#define have_email_info(name) (strlen(email_info(name)) > 0)
#define set_location_info(name, info)				\
    TclDo7("gset {lastlocation(", upcase(name), ")} {", (info), "};"	\
	   "gset {lastlocation_timestamp(", upcase(name), ")}"	\
	   " [gset floaterclock]")
#define location_info(name) TclDo3("tryget {lastlocation(", upcase(name), ")}")
#define have_location_info(name) (strlen(location_info(name)) > 0)
#define user_exists(name) nameinuse(name)

void savenewpw(char *name, char *pw)
{
  TclDo5("permset {pw(", name, ")} {", pw, "}");
  /* TclDo5("uplevel #0 set {pwx(", upcase(name), ")} {", name, "}"); */
}

void savenewemail(char *name, char *x)
{
  TclDo5("permset {email(", name, ")} {", x, "}");
}
#endif /* LOGINSERVER */



/*****************************************************************************
Settings for server addresses.  They are stored in Tcl variables.
*****************************************************************************/

char *loginservername()
{
  return STRDUP(TclGet("loginservername"));
}

char *loginserveraddr()
{
    return STRDUP(TclGet("loginserveraddr"));
}

char *loginserverport()
{
  return STRDUP(TclGet("loginserverport"));
}


char *resultservername()
{
  return STRDUP(TclGet("resultservername"));
}

char *resultserveraddr()
{
    return STRDUP(TclGet("resultserveraddr"));
}

char *resultserverport()
{
  return STRDUP(TclGet("resultserverport"));
}

void setserver(const char *name) 
{
  TclSet("loginserveraddr", name);
  TclSet("resultserveraddr", name);
  comm_reset();
}





/*****************************************************************************
Code for keeping track of all the tables out there
*****************************************************************************/

/* 
   A tablelist will be implemented as a stringlist with alternating entries:
   a name, then a description of that table, then its IP address, then port.
*/
typedef stringlist tablelist;

static tablelist *tables = NULL;

#define tabletimestamp(l) (((stringlist *) l)->s)
#define tablename(l) (((stringlist *) l)->next->s)
#define tabledescription(l) (((stringlist *) l)->next->next->s)
#define tableIPaddr(l) (((stringlist *) l)->next->next->next->s)
#define tableport(l) (((stringlist *) l)->next->next->next->next->s)
#define nexttable(l) (((stringlist *) l)->next->next->next->next->next)
#define freetablelist(l) (freestringlist(freestringlist(freestringlist(freestringlist(freestringlist((stringlist *) (l)))))))

#ifdef LOGINSERVER
static void update_activity_webpage_with_table_info()
{
  int k;
  tablelist *l;
  char *s = salloc(15000);

  fputs("update_activity_webpage_with_table_info()\n", stderr);
  s[0] = '\0';
  for (l = tables; (k = strlen(s)) < 14500 && l != NULL; l = nexttable(l))
    sprintf(s + k, "<br><b>%s</b>: %s",
	    outputname(tablename(l)), tabledescription(l));
  
  TclDo5("notetables {", STRDUP(s), "} {", floatertime(), "}");
  sfree(s);
}
#else /* not loginserver */
#define update_activity_webpage_with_table_info()
#endif


static tablelist *updatetabletimestamp(tablelist *table)
{
  reset(tabletimestamp(table));
  tabletimestamp(table) = STRDUP(TclGet("floaterclock"));
  return table;
}

static tablelist *constable(char *name, char *description, char *IPaddr,
			    char *port, tablelist *tables)
{
  return updatetabletimestamp(consstringlist(STRDUP(""),
			       consstringlist(STRDUP(name),
				consstringlist(STRDUP(description),
				 consstringlist(STRDUP(IPaddr),
				  consstringlist(STRDUP(port), tables))))));
}

static int membertablelist(char *s)
{
  tablelist *l;

  for (l = tables; l != NULL; l = nexttable(l))
    if (streq(tablename(l), s)) return TRUE;

  return FALSE;
}

static int omembertablelist(char *os)
{
  tablelist *l;

  for (l = tables; l != NULL; l = nexttable(l))
    if (streq(outputname(tablename(l)), os)) return TRUE;

  return FALSE;
}

/* add name/description to the list of tables */
/* if a table of the given name already exists, just change the description */
/* also, update the timestamp */
static void addtable(char *name, char *description, char *IPaddr, char *port)
{
  tablelist *l;
  char *oname = outputname(name);

  if (strlen(oname) > FLOATER_MAXNAME)
    oname = markgarbage(STRNDUP(oname, FLOATER_MAXNAME));
    
  /*  printf("addtable(%s, %s)\n", name, description); */

  for (l = tables; l != NULL; l = nexttable(l))
    if (streq(outputname(tablename(l)), oname)) {
      if (description != NULL && strlen(description) > 0) {
	markgarbage(tabledescription(l));
	tabledescription(l) = STRDUP(description);
	if (hasWindows) {
	    char x[FLOATER_MAXNAME + 5];

	      sprintf(x, "%12s ", oname);
	        TclDo6("join_menu_add_table {", oname, "} "
		        "{", x, description, "}");
		}
      }
      updatetabletimestamp(l);
      return;
    }

  if (!streq(name, myname)) status2(outputname(name), " is now hosting a table.");
  tables = constable(name, description, IPaddr, port, tables);
  if (hasWindows) {
    char x[FLOATER_MAXNAME + 5];
    
    sprintf(x, "%12s ", oname);
    TclDo6("join_menu_add_table {", oname, "} "
	      "{", x, description, "}");
  }
}

static tablelist *removetable(char *name, tablelist *l)
{
/*  printf("removetable(%s)\n", name);*/
  if (l == NULL) return NULL;
  else if (streq(tablename(l), name)) {
    char *oname = outputname(name);

    status2(oname, " is no longer hosting a table.");
    TclDo3("FloaterCloseName {", name, "}");
    if (hasWindows) TclDo3("join_menu_remove_table {", oname, "}");
    return freetablelist(l);
  }
  else nexttable(l) = removetable(name, nexttable(l));
  return l;
}

void describetable(char *note)
{
  extern int scoremethod;
  char *s;

  s = markgarbage(salloc(sizeof(char) * (strlen(note) + 50)));
  sprintf(s, "%10s %3d  %s",
	  TEMPCAT((NONCOMPETITIVE() ? "nc" : "  "),
		  scoringmethods[scoremethod].scoringname),
	  length(who), note);
  if (mytabledesc != NULL && streq(mytabledesc, s)) return;
  if (mynote != note) {
    reset(mynote);
    mynote = STRDUP(note);
  }
  reset(mytabledesc);
  mytabledesc = STRDUP(s);
  addtable(myname, s, localIPaddr, localport);
  broadcast(makemsg4(ANNOUNCE_TABLE, myname, s, localIPaddr, localport));
}		     

/* used when something other than the note changes */
void redescribetable(void)
{
  assert(mynote != NULL);
  describetable(mynote);
}

/* Take t, the name of a table, and put the address and port for it into
   addr and port.  Requires exact match except case is ignored.
   Returns a fl_boolean: whether it succeeded. */
fl_bool tableaddr(char *t, char *addr, char *port)
{
  tablelist *l;

  for (l = tables; l != NULL; l = nexttable(l))
    if (strcaseeq(outputname(tablename(l)), t)) {
      strcpy(addr, tableIPaddr(l));
      strcpy(port, tableport(l));
      return TRUE;
    }
  return FALSE;
}

/***************************************************************************
What follows is for keeping track of connections to parents, children, 
who is at the table, etc.

Note that the connection to the loginserver, if any, is not maintained
here, except in the case of the tableroot which has the loginserver as
its parent.

These routines merely maintain our internal representation---they
usually do not issue Tcl commands to create or destroy connections.
***************************************************************************/

typedef struct conn {
  char *name, *IPaddr, *port, *connstr;
} conn;

typedef struct childlist {
  conn *c;
  struct childlist *next;
} childlist;

static conn *parent = NULL;
static conn *oldparent = NULL; /* used when we get a JOIN_ELSEWHERE request */


static conn *lho = NULL, *rho = NULL;

/* end of bridge specific things */

static childlist *kchildren = NULL;
static childlist *tchildren = NULL;

static void announce_disconn(char *s);

static int namemember(char *name, childlist *l)
{
  while (l != NULL)
    if (streq(name, l->c->name)) return TRUE;
    else l = l->next;
  return FALSE;
}

static int filemember(char *file, childlist *l)
{
  while (l != NULL)
    if (streq(file, l->c->connstr)) return TRUE;
    else l = l->next;
  return FALSE;
}

static conn *makeconn(char *name, char *IPaddr, char *port, char *connstr)
{
  conn *newc;

  newc = alloc(conn);
  newc->name = STRNDUP(name, FLOATER_MAXNAME);
  newc->IPaddr = STRNDUP(IPaddr, MAXADDRLEN);
  newc->port = STRNDUP(port, MAXPORTLEN);
  newc->connstr = STRDUP(connstr);
  return newc;
}

/* add a child to the list of children */
static void addchild(childlist **children, char *name, char *IPaddr, char *port, char *connstr)
{
  childlist *newlist;

#if DEBUGCONN
  DEBUG(talkmsg2("New child: ", name));
  DEBUG(talkmsg4("New child: ", IPaddr, ":", port));
  DEBUG(talkmsg2("New child: ", connstr));
#endif

  newlist = alloc(childlist);
  newlist->c = makeconn(name, IPaddr, port, connstr);
  newlist->next = *children;
  *children = newlist;

  /* do we have an old connection to this IPaddr:port?  If so, close it */
  /* (this should never happen) */
  for (newlist = (*children)->next; newlist != NULL; newlist = newlist->next)
    if (streq(newlist->c->IPaddr, IPaddr) && streq(newlist->c->port, port))
      TclDo3("catch {FloaterClose ", newlist->c->connstr, "}");
}

static void freeconn(conn *c)
{
  if (c == NULL) return;
#if DEBUGCONN
  DEBUG(talkmsg3(myname, "is removing connection to ", c->name));
  DEBUG(talkmsg4("Removing connection: ", c->IPaddr, ":", c->port));
  DEBUG(talkmsg2("Removing connection: ", c->connstr));
#endif
  markgarbage(c->name);
  markgarbage(c->IPaddr);
  markgarbage(c->port);
  markgarbage(c->connstr);
  markgarbage((char *) c);
}

/* frees memory used by a childlist node, and returns the contents of `next' */
static childlist *freechild(childlist *l)
{
  childlist *ret;

  if (l == NULL) return NULL; else ret = l->next;

  freeconn(l->c);
  markgarbage((char *) l);

  return ret;
}

#define removekchild(f, a) removechild(&kchildren, (f), (a))
#define removetchild(f, a) removechild(&tchildren, (f), (a))

static void removechild(childlist **children, char *connstr, fl_bool announce)
{
  childlist *l;

#if DEBUGCONN
  if (hasWindows)
    printf("%s: removechild %s\n", myname, connstr);
#endif
  if (*children == NULL) {
#if DEBUGCONN
    DEBUG(warningmsg3("Attempting to remove child (", connstr, ") that isn't here"));
#endif
    return;
  }

  if (streq((*children)->c->connstr, connstr)) {
    char *name = TEMPCOPY((*children)->c->name);
    *children = freechild(*children);
    if (announce) announce_disconn(name);
    return;
  }
  
  for (l = *children; l->next != NULL; l = l->next)
    if (streq(l->next->c->connstr, connstr)) {
      char *name = TEMPCOPY(l->next->c->name);
      l->next = freechild(l->next);
      if (announce) announce_disconn(name);
      return;
    }
  
#if DEBUGCONN
  DEBUG(warningmsg3("Attempted to remove kchild (", connstr, ") that isn't here"));
#endif
}    

void showparent(void)
{
  if (parent == NULL) 
    status("No parent");
  else
    status7("Parent: ", outputname(parent->name), " (", parent->IPaddr, ":",
	    parent->port, ")");
}
  
void showip(void)
{
  if (opennormalport() < 0) {
    errormsg("Unable to open a port locally; others may not connect to you.");
    return;
  }

  status4("Others may connect to you at ", localIPaddr, ":", localport);
}

void showkchildren(void)
{
  childlist *l;
  char s[1000];

  if (kchildren == NULL) {
    status("No kchildren");
    return;
  }
  strcpy(s, "Kchildren:");
  for (l = kchildren; l != NULL && strlen(s) < 999 - FLOATER_MAXNAME; l = l->next)
    sprintf(s + strlen(s), " %s", l->c->name);
  status(s);
}

void showtchildren(void)
{
  childlist *l;
  char s[1000];

  if (tchildren == NULL) {
    status("No tchildren");
    return;
  }
  strcpy(s, "Tchildren:");
  for (l = tchildren; l != NULL && strlen(s) < 999 - FLOATER_MAXNAME; l = l->next)
    sprintf(s + strlen(s), " %s", l->c->name);
  status(s);
}

void showchildren(void) { showkchildren(); showtchildren(); }

#define sendtokchildren(s, except) sendtochildren(kchildren, (s), (except))
#define sendtotchildren(s, except) sendtochildren(tchildren, (s), (except))

/* Send s to all members of l except for those that match no_send. */
static void sendtochildren(childlist *l, char *s, char *no_send)
{
  while (l != NULL) {
    if (no_send == NULL || !streq(no_send, l->c->connstr))
      sendstrmessage(l->c->connstr, s);
     l = l->next;
  }
}

void sendtoallchildren(message *m)
{
  char *s = msgtotext(m);
  
  sendtokchildren(s, NULL);
  sendtotchildren(s, NULL);
}


/* choose a kchild by name.  If N/S/E/W is specified, then choose based
 * on Bridge seating direction.  Otherwise, look for a name match */
childlist *findkchild(char *nom)
{
  childlist *l,*loser=NULL;
  int len=strlen(nom);

  /* check for bridge directions */
  if((len == 1) && (strchr("NSEWnsew", nom[0])))
    if(name_at_seat(toupper(nom[0]))) {
      nom = name_at_seat(toupper(nom[0]));
      len = strlen(nom);
    }
  


  /* look for exact matches */
  for (l = kchildren; l != NULL; l = l->next) {
    if (!strcasecmp(nom, l->c->name)) {
      loser=l; break;
    }
  }
  if (loser) return(loser);

  /* look for exact matches on the name part only */
  for (l = kchildren; l != NULL; l = l->next) {
    if (strlen(l->c->name) > len && l->c->name[len]=='!' &&
        !strncasecmp(nom, l->c->name, len)) {
      if (loser) {
        status4("Ambiguous match: ", loser->c->name, " or ", l->c->name);
        return(NULL);
      }
      loser=l;
    }
  }
  if (loser) return(loser);

  /* look for names for which nom is a prefix */
  for (l = kchildren; l != NULL; l = l->next) {
    if (strlen(l->c->name) >= len &&
        !strncasecmp(nom, l->c->name, len)) {
      if (loser) {
        status4("Ambiguous match: ", loser->c->name, " or ", l->c->name);
        return(NULL);
      }
      loser=l;
    }
  }
  return(loser);
}

void punt(char *bozo)
{
  childlist *loser;

  if (!tablehostmode) {
    errormsg("You must be host to punt others from your table");
    return;
  }
  if (kchildren == NULL) {
    errormsg("No one to punt");
    return;
  }

  loser=findkchild(bozo);
  if (!loser) {
    status3("No such user: '", bozo, "'");
    return;
  }
  sendmessage(loser->c->connstr, makemsg2(SAY, myoutputname, "Begone!"));
  removekchild(loser->c->connstr, TRUE);
}

void unseat(char *bozo)
{
  childlist *loser;

  if (!tablehostmode) {
    errormsg("You must be host to unseat a player");
    return;
  }
  if (kchildren == NULL) {
    errormsg("No one to unseat");
    return;
  }

  loser=findkchild(bozo);
  if (!loser) {
    status3("No such user: '", bozo, "'");
    return;
  }
  sendmessage(loser->c->connstr, makemsg2(SAY, myoutputname, "Stand up!"));
  selfmessage(makemsg4(SEATED, loser->c->name, "0",
                     loser->c->IPaddr, loser->c->port));
}

char *hhmmss()
{ 
  char mybuf[9];
  struct tm *mytm;
  time_t mytime = time(0);
  mytm=localtime(&mytime);
  sprintf(mybuf,"%2d:%02d:%02d",mytm->tm_hour,mytm->tm_min,mytm->tm_sec);
  return(STRDUP(mybuf));
}

/* add someone to the "who" list */
void addwho(char *s)
{
  int oldlen = length(who);
  struct tm *mytime;

  who = addstringlist(s, who);
  if (oldlen == length(who)) return;
  if (!streq(s, myname))
    status6(outputname(s), " has joined @ ", markgarbage(hhmmss()), " (", itoa(length(who)), ").");
#if DEBUGCONN
  else status6(outputname(s), " has joined @ (", markgarbage(hhmmss()), " (", itoa(length(who)), ").");
#endif
}

/* remove someone from the "who" list */
void removewho(char *s)
{
  int oldlen = length(who);
  struct tm *mytime;

  who = removestringlist(s, who);
  if (oldlen == length(who)) return;
  if (!streq(s, myname))
    status6(outputname(s), " has left @ ", markgarbage(hhmmss()), " (", itoa(length(who)), ").");
}

/****************************************************************************
Code for making messages.  Code for converting them to their string
representation and vice versa.
****************************************************************************/

message *makemsg10(char kind,
		   const char *a, const char *b, const char *c,
		   const char *d, const char *e, const char *f,
		   const char *g, const char *h, const char *i,
		   const char *j)
{
  message *new;
  static int messageno = 0;
  char messageID[20];

  sprintf(messageID, "%x", messageno++);

  new = alloc(message);
  markgarbage((char *) new); /* mark it to be freed later */
  new->kind = kind;
  new->args = (char **) salloc(MAXMSGARGS * sizeof(char *));
  markgarbage((char *) new->args); /* mark it to be freed later */
  new->args[0] = (a == NULL) ? NULL : TEMPCOPY(a);
  new->args[1] = (b == NULL) ? NULL : TEMPCOPY(b);
  new->args[2] = (c == NULL) ? NULL : TEMPCOPY(c);
  new->args[3] = (d == NULL) ? NULL : TEMPCOPY(d);
  new->args[4] = (e == NULL) ? NULL : TEMPCOPY(e);
  new->args[5] = (f == NULL) ? NULL : TEMPCOPY(f);
  new->args[6] = (g == NULL) ? NULL : TEMPCOPY(g);
  new->args[7] = (h == NULL) ? NULL : TEMPCOPY(h);
  new->args[8] = (i == NULL) ? NULL : TEMPCOPY(i);
  new->args[9] = (j == NULL) ? NULL : TEMPCOPY(j);
  new->from = TEMPCOPY(myname);
  new->ID = TEMPCOPY(messageID);
  return new;
}

static int countargs(message *m)
{
  int numargs;
  
  for (numargs = 0; numargs < MAXMSGARGS; numargs++)
    if (m->args[numargs] == NULL) break;

  return numargs;
}

#define MAXMSGTEXT 5000
static char *msgtotext(message *m)
{
  char text[MAXMSGTEXT];
  int i, numargs, totallength;

  numargs = countargs(m);

  sprintf(text, "%c%d\\%d\\%d\\%s%s", m->kind, numargs,
	  strlen(m->from), strlen(m->ID), m->from, m->ID);
  for (i=totallength=0; i<numargs; i++) {
    sprintf(text + strlen(text), "%d\\", strlen(m->args[i]));
    totallength += strlen(m->args[i]);
  }
  assert(totallength + strlen(text) + 3 < MAXMSGTEXT);
  for (i=0; i<numargs; i++)
    sprintf(text + strlen(text), "%s", m->args[i]);

#if DBG
  if (hasWindows)
    puts(text);
#endif
  if (text[strlen(text) - 1] == '\\') sprintf(text + strlen(text), "\\");
  return TEMPCOPY(text);
}


message *mysterymessage(char *s)
{
#if SERVER
  fprintf(stderr, "Mystery message: %s\n", s);
#else
#if DBG
  DEBUG(talkmsg2("Mystery message: ", s));
#endif
#endif

  return NULL;
}

/* the inverse of msgtotext */
/* marks its return value as garbage */
message *parsemessage(char *s)
{
  char *original = s;
  message *m;
  int numargs, i, arglen[MAXMSGARGS], fromlen, IDlen;
  
  if (s == NULL || *s == '\0') return NULL;

  m = alloc(message);
  markgarbage((char *) m);
  m->kind = *s++;
  if (!validmessagekind(m->kind)) return mysterymessage(original);
  assert(m->kind != '\0');
  m->args = (char **) salloc(sizeof(char **) * MAXMSGARGS);
  markgarbage((char *) m->args);
  if (!isdigit(*s)) return mysterymessage(original);
  numargs = parseint(&s);
  if (!isdigit(*s)) return mysterymessage(original);
  fromlen = parseint(&s);
  if (!isdigit(*s)) return mysterymessage(original);
  IDlen = parseint(&s);
  m->from = markgarbage(salloc((fromlen + 1) * sizeof(char)));
  m->ID = markgarbage(salloc((IDlen + 1) * sizeof(char)));
  strncpy(m->from, s, fromlen);
  m->from[fromlen] = m->ID[IDlen] = '\0';
  safety_check(m->from);
  s += fromlen;
  strncpy(m->ID, s, IDlen);
  s += IDlen;
  safety_check(m->ID);
  for (i=0; i<numargs; i++) {
    if (!isdigit(*s)) return mysterymessage(original);
    arglen[i] = parseint(&s);
  }
  for (i=0; i<numargs; i++) {
    m->args[i] = markgarbage(salloc((arglen[i] + 1) * sizeof(char)));
    strncpy(m->args[i], s, arglen[i]);
    m->args[i][arglen[i]] = '\0';
    if (strlen(m->args[i]) != arglen[i]) return mysterymessage(original);
    destructivebracketclean(m->args[i]);
    s += arglen[i];
  }
  if (*s != '\0') {
    char warn[100];
    sprintf(warn, "Extra cruft at end of %c message: `%s'", m->kind, s);
    warningmsg(warn);
  }
  while (i < MAXMSGARGS) m->args[i++] = NULL;
  
  return m;
}

/* take an array of n messages and output a multimessage of those messages */
/* limitation: n <= 9 */
/* should work but is currently unused */
message *multimessage(message *A[], int n)
{
  int i;
  char *B[MAXMSGARGS];

  assert(n <= 9);
  for (i = 0; i < 9; i++) B[i] = ((i < n) ? msgtotext(A[i]) : NULL);
  return makemsg10(MULTIMESSAGE, B[0], B[1], B[2], B[3], B[4],
		   B[5], B[6], B[7], B[8], NULL);
}

/*****************************************************************************/

#define BEEP_STR "_!BEEP!beep!BEEP!_"
#define saybeep_to_one(beepee) say_to_one((beepee), BEEP_STR)

static void say_to_one(char *who, char *what)
{
  extern fl_bool spec;
  selfmessage(makemsg3(SAY_TO_ONE, itoa(spec), who, what));
}

/* A perfect match except for case? */
static char *partial_namematch1(char *x, char **l, int n)
{
  if (n <= 0) return NULL;
  if (strcaseeq(x, *l)) return *l;
  else return partial_namematch1(x, ++l, --n);
}

/* Is x a perfect prefix for exactly one element of l? */
static char *partial_namematch2(char *x, char **l, int n)
{
  if (n <= 0) return NULL;
  if (strneq(x, *l, strlen(x)))
    return (partial_namematch2(x, l + 1, --n) == NULL ? *l : NULL);
  else return partial_namematch2(x, ++l, --n);
}

/* Ignoring case, is x a perfect prefix for exactly one element of l? */
static char *partial_namematch3(char *x, char **l, int n)
{
  if (n <= 0) return NULL;
  if (strncaseeq(x, *l, strlen(x)))
    return (partial_namematch3(x, l + 1, --n) == NULL ? *l : NULL);
  else return partial_namematch3(x, ++l, --n);
}

/* l is a vector of length n of strings.  If x is a decent partial
   match for one and only one of the things in l, return that element
   of l.  Otherwise NULL. */
static char *partial_namematch(char *x, char **l, int n)
{
  char *s;
  if ((s = partial_namematch1(x, l, n)) != NULL) return s;
  if ((s = partial_namematch2(x, l, n)) != NULL) return s;
  else return partial_namematch3(x, l, n);
}

/* l is a vector of length n of strings.  Return whether x exactly
   equals anything in l. */
static fl_bool perfect_namematch(char *x, char **l, int n)
{
  return ((n > 0) && (streq(x, *l) || perfect_namematch(x, ++l, --n)));
}

char *namematch(char *x, char **l, int n)
{
  return perfect_namematch(x, l, n) ? x : partial_namematch(x, l, n);
}

void commbeep(char *arg)
{
  int i, n;
  char **who = getwho(&n);
  char *beepee;

  if (n <= 1) { errormsg("There is no one else here to beep."); return; }

  arg = trim(arg);

  for (i = 0; i < n; i++) who[i] = outputname(who[i]);

  /* Default: beep everyone */
  if (!strexists(arg)) {
    while (--n >= 0) if (!streq(who[n], myoutputname)) commbeep(who[n]);
    return;
  }

  if ((beepee = namematch(arg, who, n)) != NULL)
    saybeep_to_one(beepee);
  else
    errormsg3("It is unclear whom to beep (`", arg, "'?)");
}

/*****************************************************************************/

#define recent_table_arrival() \
    atob(TclDo("uplevel #0 {expr $floaterclock - $table_arrival_time < 10}"))

void commresult(fl_bool competitive, message *m, char *handname)
{
  char *s;

  s = msgtotext(m);

  if (competitive && !recent_table_arrival()) {
    /* use Tcl variable sentresult() to keep track of the hands we've
     sent out results for */
    if (atob(TclDo3("global sentresult; info exists sentresult(", handname, ")")))
      if (streq(TclDo3("gset sentresult(", handname, ")"), s)) return;
    TclDo5("gset sentresult(", handname, ") {", s, "}");

    if (atob(TclDo3("emailresult {", s, "}"))) {
      errormsg("Unable to report result to server by email!");
      errormsg(TclGet("errorstring"));
    }
    else
      status2("Result successfully reported to server @ ", markgarbage(hhmmss()));
  }

  if (tablehostmode) {
    char *rs = connect_to_resultserver();

    if (rs != NULL) sendstrmessage(rs, s);
    else errormsg("Unable to connect to result server to see others' results");
  }      
}

/* delay is the number of seconds to wait before reporting (typically
   by email) what's been seen.  See mail.TCL for emailseens. */
void commseens(int delay)
{
  if (delay <= 0) {
    if (atob(TclDo("emailseens"))) {
      errormsg("Unable to report boards seen by email!");
      errormsg(TclGet("errorstring"));
    }
    else
      status("Boards seen successfully reported by email");
  } else TclDo3("after [expr 1000 * ", itoa(delay), "] emailseens");
}

/*****************************************************************************/


/* callback for when a connection times out */
int floatertimeout(ClientData clientData, Tcl_Interp *interp,
		   int argc, const char *argv[])
{
  assert(argc == 2); /* argv[0] is "floatertimeout"; argv[1] is the connstr */

    
#if LOGGING
  fprintf(logfile, "closing %s (timeout)\n", argv[1]);
#endif

 if (isparentfile(argv[1]) && !tablehostmode) disconnect_all();
 else {
   /* closing the connection automatically will invoke FloaterClose(), below */
   TclDo2("FloaterClose ", argv[1]);
 }
 return TCL_OK;
}

/* Called to close a conn */
int FloaterClose(ClientData clientData, Tcl_Interp *interp,
		 int argc, const char *argv[])
{
  char *sock;
  char *connstr = (char *) argv[1];

  assert(argc == 2); /* argv[0] is "FloaterClose"; argv[1] is the connstr */

  if (atoi(TclDo3("catch {uplevel #0 set conn_to_sock(", connstr, ")}")))
    /* we've already closed this one and forget all about it, so just return */
    return TCL_OK;

  sock = TclDo3("uplevel #0 set conn_to_sock(", connstr, ")");

#if DEBUGCONN
  if (hasWindows)
    printf("%s: FloaterClose %s (%s)\n", myname, connstr, sock);
#endif

  TclDo2("shouldnotsendiamalive ", connstr);
  TclDo2("shouldnotreceiveiamalive ", connstr);

#if DBG
  DEBUG(talkmsg5("Closing conn ", connstr, " (", sock, ")"));
#endif
#if LOGGING
  fprintf(logfile, "Closing conn %s (%s)\n", connstr, sock);
#endif

  TclDo3("catch {close ", sock, "}");
  TclDo3("catch {uplevel #0 unset sock_to_conn($conn_to_sock(", connstr,
	 "))}");
  TclDo3("catch {uplevel #0 unset conn_to_sock(", connstr, ")}");

#ifdef PSEUDOMAILER
  if (atob(TclDo2("remailnow ", connstr)))
    status2("remail failed: ", TclGet("errorstring"));
  else
    status("remail.");
#else
  if (iskchildfile(connstr)) removekchild(connstr, TRUE);
  else if (istchildfile(connstr)) {
    removetchild(connstr, TRUE);
#ifdef LOGINSERVER
    if (tchildren == NULL)
      tablerootname[0] = tablerootaddr[0] = tablerootport[0] = '\0';
#endif    
  } else if (isparentfile(connstr)) {
    char *parentname = TEMPCOPY(parent->name);

#if DEBUGCONN
    if (hasWindows)
      printf("%s: disconnected from parent\n", myname);
#endif
    freeconn(parent);
    parent = NULL;
    tablerootmode = FALSE;
    announce_disconn(parentname);
    if (state == DISCONNECTING) state = LIMBO;
    if (state == CONNECTED && !tablehostmode) disconnect_all();
    else if (state == CONNECTED && tablehostmode) {
      reset_time_to_rejoin();
      rejoinaddr[0] = rejoinport[0] = '\0'; /* don't ever rejoin at parent */
      rejoin();
    }
  }
  if (state == LIMBO) tablehostmode = FALSE;
  UIcommstate(state);
#endif
  if_not_connected_reset_tablehostname();
  return TCL_OK;
}

/* received message: argc and argv contain a message from someone */
/* argv[0] is "floaterreceive";
   argv[1] is the message;
   argv[2] is the conn of the sender */
int receivemessage(ClientData clientData, Tcl_Interp *interp,
		   int argc, const char *argv[])
{
  message *m;

  assert(argv[1] != NULL);

#ifdef PSEUDOMAILER
  status4("got ", argv[1], " from ", argv[2]);
  if (streq(argv[1], "ozzie_and_harriet")) {
    TclDo3("uplevel #0 {set ozzie(", argv[2], ") 1}");
  }
  else if (atob(TclDo3("global ozzie; info exists ozzie(", argv[2], ")"))) {
    TclDo5("remail {", argv[1], "} {", argv[2], "}");
  } else {
    status4("Don't know what to do with ", argv[1], " from ", argv[2]);
  }
  return TCL_OK;
#else

  /* parse the message; if empty, ignore it and return */
  if ((m = parsemessage((char *) argv[1])) == NULL) return TCL_OK;

  executionlog("receive: ", argv[1]); 

  reset(connstr);
  if (argc == 3) connstr = STRDUP(argv[2]);
  else assert(0);

#if LOGGING
  {
    int numargs = countargs(m);

    fprintf(logfile, "Received a %c message from %s with %d args:\n%s\n",
	    m->kind, m->from, numargs, argv[1]);
  }
#endif

  if (streq(m->from, myname)) {
    /* how droll, I'm receiving a message that I originally sent */
    return TCL_OK;
  }
  
#if DEBUGMSG0
  {
    int numargs, i;
    char s[100];

    numargs = countargs(m);

    sprintf(s, "Received a %c message from %s with %d args", m->kind,
	    m->from, numargs);
    if (hasWindows) puts(s);
    DEBUG(talkmsg(s));
    for (i=0; i<numargs; i++) {
      DEBUG(talkmsg(m->args[i]));
      if (hasWindows) puts(m->args[i]);
    }
  }
#endif

  /* take action depending on what the message is */
  handlemessage(m, STRDUP(argv[1]));
  return TCL_OK;
#endif
}

/* Handle a string of messages separated by tabs. */
void tabmulti(char *s)
{
  message *m;
  int i;
  fl_bool foundtab, foundend;

  s = TEMPCOPY(s);

  while (TRUE) {
    i = 0;
    do {
      foundtab = (s[i] == '\t');
      foundend = (s[i] == '\0');
      if (foundtab) s[i] = '\0';
      i++;
    } while (!foundtab && !foundend);
    if ((m = parsemessage(s)) != NULL) handlemessage(m, s);
    if (foundtab) s = s + i; else return;
  }
}

void sendmessage(char *to, message *m)
{
  char *s;

  assert(strexists(to));
  s = msgtotext(m);
  assert(strexists(s));
  TclDo5("FloaterSend ", to, " {", s, "}");
#if LOGGING  
  fprintf(logfile, "%s%s%s%s%s\n", "FloaterSend ", to, " {", s, "}");
#endif
}

static void sendstrmessage(char *to, char *s)
{
  assert(strexists(to));
  assert(strexists(s));
  TclDo5("FloaterSend ", to, " {", s, "}");
#if LOGGING
  fprintf(logfile, "%s%s%s%s%s\n", "FloaterSend ", to, " {", s, "}");
#endif
}

static void sendtoparent(char *s)
{
  if (parent != NULL) sendstrmessage(parent->connstr, s);
}

/*
   send a message to all parents and kchildren
   exception: if I'm the tablehost and its not a globally important message,
   send it only to kchildren
*/
void broadcast(message *m)
{
  char *s;

  s = msgtotext(m);

  sendtokchildren(s, NULL);
  if (!tablehostmode || globalbroadcast(m->kind)) sendtoparent(s);
  if (globalbroadcast(m->kind)) sendtotchildren(s, NULL);
}

/* send a message to myself (and automatically rebroadcast it if it is
   that kind of message) */
void selfmessage(message *m)
{
  char realconnstr;

  /* while handling a self-generated message, connstr should be empty */
  /* (having to do this hack shows why connstr should not be global) */
  if (connstr != NULL) {
    realconnstr = connstr[0];
    connstr[0] = '\0';
    handlemessage(m, NULL);
    connstr[0] = realconnstr;
  } else handlemessage(m, NULL);
}

/* broadcast, except don't send m back to whoever told us m */
static void rebroadcast(message *m)
{
  char *s;

  s = msgtotext(m);

  if (parent != NULL && strexists(connstr) && streq(connstr, parent->connstr))
    sendtokchildren(s, NULL);
  else {
    sendtokchildren(s, connstr);
    if ((tablehostmode && globalbroadcast(m->kind)) || !tablehostmode)
      sendtoparent(s);
  }
  if (globalbroadcast(m->kind)) sendtotchildren(s, connstr);
}

void commsetup()
{
  TclSet("loginserveraddr", DEFAULT_SERVER);
  TclSet("resultserveraddr", DEFAULT_SERVER);
  
  if (localIPaddr == NULL) {
    Floater_hostname = STRDUP(TclDo("info hostname"));
    localIPaddr = STRDUP(TclGet("localIPaddr"));
#if DEBUGCONN
    if (hasWindows)
      printf("localIPaddr=%s\n", localIPaddr);
#endif
  }
#ifndef SERVER
  opennormalport();
#endif
}

#define PORTLO 10100
#define PORTHI 10120
/* If some port is already open for listening, return its number.
   Otherwise, try to use ports in the range PORTLO to PORTHI inclusive, but if
   that doesn't work, try 1010, then any old port. Returns port that
   is open on success, or -1 on failure. */
static int opennormalport()
{
  int p = PORTLO, port = openport(itoa(p));
  while (port < 0 && p <= PORTHI)
    port = openport(itoa(++p));
  return ((port < 0) ? openport(NULL) : port);
}


static void resetmyname()
{
    char newname[100];
#if 0
    sprintf(newname, "Doe %s", localport); /* for now, use port # as name */
#endif
    sprintf(newname, "No Name");
    setmyname(newname, FALSE);
}

/* returns port opened on success, or -1 on failure */
/* if port is NULL, use any old port */
static int openport(char *port) 
{
  int ret;
  char *command;

  if (localport != NULL) return atoi(localport);

  command = markgarbage(salloc((((port == NULL) ? 0 : strlen(port)) + 100)
			       * sizeof(char)));

  sprintf(command,
	  "catch {FloaterListen %s} openport",
	  (port == NULL) ? "" : port);
  ret = 
    atoi(TclDo(command)) ?
      -1 : atoi(localport = STRDUP(TclDo("set openport")));

  if (localport != NULL && strlen(myname) == 0) {
    resetmyname();
  }

  return ret;
}

/* set up a name by concatenating the desired name and a bogus, unique tag */
/* the unique tag depends on the local IP address and port */
/* if loggedin is false, add an asterisk after the name to indicate it */
static void setmyname(char *newname, fl_bool loggedin)
{
  /* hack because changing your name will confuse others at the table */
  if (state == CONNECTED) sever_all_communication();

  assert(strlen(newname) <= MAX_OUTPUTNAME_LENGTH);
  strcpy(myname, newname);
  if (!loggedin) sprintf(myname + strlen(myname), "*");
  strcpy(myoutputname, myname);
  openport(NULL);
  setbogusnewname(myname + strlen(myname), localIPaddr, localport);

  /* stuff related to what hands have been seen */
  TclSet("seenname", loggedin ? myoutputname : "_everyone_");
  checknewweek();
  loadseen();
}

/* for human readable output, truncate name at the '!' */
/* do not modify input; instead, copy, and mark copy as dead */
char *outputname(char *name)
{
  int i;

  if (!strexists(name)) return "";
  i = 1;
  while ((name[i] != '\0') && (name[i] != '!')) i++;
  if (name[i] != '!') return name;
  else return markgarbage(STRNDUP(name, i));
}

static void disconnect(conn *c)
{
#if DEBUGCONN
  DEBUG(talkmsg4("Closing connection (", c->connstr, ") to ", c->name));
#endif
#if LOGGING
  fprintf(logfile, "Closing connection (%s) to %s\n", c->connstr, c->name);
#endif
  sendmessage(c->connstr, makemsg1(ANNOUNCE_DISCONNECT, myname));
  TclDo3("catch {FloaterClose ", c->connstr, "}");
}

static void disconnect_all(void)
{
  state = DISCONNECTING;

/*   UIpatientcursor();*/

#ifndef SERVER
  strcpy(location, "");
#endif
#if DEBUGCONN
    status("Severing all connections; clearing `who' list");
#endif

  while (kchildren != NULL) {
    disconnect(kchildren->c);
    kchildren = freechild(kchildren);
  }

  while (tchildren != NULL) {
    disconnect(tchildren->c);
    tchildren = freechild(tchildren);
  }

  if (parent != NULL) {
    disconnect(parent);
    freeconn(parent);
    parent = NULL;
  }

  if (lho != NULL) {
    disconnect(lho);
    freeconn(lho);
    lho = NULL;
  }

  if (rho != NULL) {
    disconnect(rho);
    freeconn(rho);
    rho = NULL;
  }

  while (who != NULL) who = freestringlist(who);

/*  UInormalcursor();*/
  tablehostmode = FALSE;
  state = LIMBO;
  nobridge();

  /* neither send nor expect iamalive messages */
  TclSet("sendiamalivelist", "");
  TclSet("receiveiamalivelist", "");
  UIcommstate(state);
  if_not_connected_reset_tablehostname();
}

void sever_all_communication(void)
{
  state = DISCONNECTING;
  announce_disconn(myname);
  disconnect_all();
  if (tablehostmode) {
    tablehostmode = FALSE;
    tables = removetable(myname, tables);
  }
  state = LIMBO;
}

void comm_reset()
{
  sever_all_communication();
  resetmyname();
}



#ifdef LOGINSERVER
void loginserver(char *p)
{
  int port;
  char s[100];
  extern char previousglobaldate[];

  /* setup(); */

  if ((port = openport(p)) <= 0) {
    errormsg("Attempt to become a login server failed (unable to open a port)");
    return;
  }
  
  disconnect_all();

  checknewweek();
  status4("Date: ", globaldate,
	  "; Previous date: ", previousglobaldate);
  TclDo4("seen_startup ", globaldate, " ", previousglobaldate);

  sprintf(s, "Listening on port %s:%d on local machine",
	  Floater_hostname, port);
  status(s);
  state = CONNECTED;
  status("You are now a login server");
}
#endif

#ifdef PSEUDOMAILER
void pseudomailer(char *p)
{
  int port;
  char s[100];

  /* setup(); */

  if ((port = openport(p)) <= 0) {
    errormsg("Attempt to become a pseudomailer failed (unable to open a port)");
    return;
  }
  
  disconnect_all();

  sprintf(s, "Listening on port %s:%d on local machine",
	  Floater_hostname, port);
  status(s);
  state = CONNECTED;
  status("You are now a pseudomailer");
}
#endif
  
#ifdef RESULTSERVER
void resultserver(char *p)
{
  int port;
  char s[100];

  /* setup(); */

  if ((port = openport(p)) <= 0) {
    errormsg("Attempt to become a result server failed (unable to open a port)");
    return;
  }
  
  disconnect_all();

  sprintf(s, "Listening on port %s:%d on local machine",
	  Floater_hostname, port);
  status(s);
  state = CONNECTED;
  checknewweek();
  tabmulti(TclDo2("loadresults ", globaldate));
  startingup = FALSE;
  status("You are now a result server");
}
#endif
  
#ifndef SERVER
/* serve a table */
void tablehost(char *addr, char *p)
{
  int port;
  char s[100];

  /* setup(); */

  if ((port = opennormalport()) <= 0) {
    errormsg("Attempt to serve a table failed (unable to open a port)");
    return;
  }
  
  if (!tablehostmode && omembertablelist(myoutputname)) {
    if (membertablelist(myname)) tables = removetable(myname, tables);
    else {
      errormsg3("There is already a table under the name `",
		myoutputname, "', so to prevent name conflicts you will "
		"not be able to host a table right now");
      return;
    }
  }

  sever_all_communication();

  sprintf(s, "Listening on port %s:%d on local machine",
	  Floater_hostname, port);
  status(s);
  state = CONNECTED;
  status("You are now hosting a table");
#ifndef SERVER
  strcpy(location, "hosting");
#endif
  tablehostmode = TRUE;
  addwho(myname);
  describetable(defaultnote);
  STRCPY(tablehostname, myname);
  STRCPY(tablehostaddr, localIPaddr);
  STRCPY(tablehostport, localport);

  /* try to join the table tree */
  if (addr == NULL || p == NULL) {
    if (strlen(tablerootaddr) > 0) {
      addr = tablerootaddr;
      p = tablerootport;
    } else {
      addr = loginserveraddr();
      p = loginserverport();
    }
  }
  if (addr != NULL && p != NULL)
    if (!join(addr, p, TABLE_SERVERs, 1) &&
	(!streq(p, loginserverport()) || !streq(addr, loginserveraddr()))) {
      /* didn't work; try loginserver */
      addr = loginserveraddr();
      p = loginserverport();
      if (addr != NULL && p != NULL)
	join(addr, p, TABLE_SERVERs, 1);
    }

  tablehostmode = TRUE;
  addwho(myname);
  UIcommstate(state);
}
#endif /* ifndef SERVER */
  
/*****************************************************************************/

/* Go through the table list and remove any tables we haven't heard
from lately.  If we haven't heard from our parent lately, broadcast
the demise of the parent (which will kick in the rejoin mechanism). */
void checktablelisttimeouts(void)
{
  tablelist *l;
  static fl_bool inside = FALSE; /* prevent recursive entry */
  static int lastcheck = -1;
  int time = floaterclock();
  int timeout = atoi(TclGet("tabletimeout"));

  /* checking once per second is plenty */
  if (lastcheck == time) return;
  if (inside) return; else inside = TRUE;
  lastcheck = time;

  for (l = tables; l != NULL; ) 
    if (time - atoi(tabletimestamp(l)) > timeout) {
      if (isparentname(tablename(l))) {
	status3("Haven't heard from ", outputname(tablename(l)), " for a while; assuming table is gone");
	selfmessage(makemsg1(TABLE_DEMISE, tablename(l)));
	/* don't bother processing the rest of the list right now */
	inside = FALSE;
	return;
      }
      l = tables = removetable(tablename(l), tables); /* inefficient */
    } else l = nexttable(l);

  inside = FALSE;
}

/* tablehosts should reannounce their existence so others know they're
   still alive */
void periodictablereannounce(void)
{
  static int lastreannounce = -1;
  int time = floaterclock();
  int timeout = atoi(TclGet("tablereannounce"));

  if (!tablehostmode || !strexists(mytabledesc)) return;
  if (lastreannounce != -1 && ((time - lastreannounce) < timeout)) return;

  DEBUG(talkmsg("Reannouncing my table"));
  selfmessage(makemsg4(ANNOUNCE_TABLE, myname, mytabledesc,
		       localIPaddr, localport));
  lastreannounce = time;
}

/*****************************************************************************/

void try_rejoin(void)
{
#if DEBUBCONN
  DEBUG(talkmsg("call to try_rejoin()"));
#endif

  if (tablehostmode && parent == NULL && time_to_rejoin()) rejoin();
}

/* Set *q to a copy of the part of addr before its first '!'; set *p to a
   copy of the rest.  Assumes '!' appears at least once in addr. */
static void addrsplit(char *addr, char **p, char **q)
{
  *p = TEMPCOPY(addr);
  for (*q = *p; **q != '!'; ++*q);
  **q = '\0';
  ++*q;
}

/* Open a socket.  join() is one of the two functions that calls
   FloaterConnect (the other is connect_to_loginserver()).  Return 1
   on success, 0 on failure. */
int join(char *addr, char *port, char *capacity, int numtries)
{
  int t, retry, lport;
  char s[100];

  if (numtries == 0) return 0;

  assert(numtries > 0);

  UIpatientcursor();

  if ((lport = opennormalport()) <= 0) {
    errormsg("Attempt to join failed (unable to open a port locally)");
    UInormalcursor();
    return 0;
  }

  if (streq(addr, localIPaddr) && streq(port, localport)) {
    UInormalcursor();
    return 0;
  }

  if (isin('!', addr)) {
    int ret;
    char *p, *q;
    addrsplit(addr, &p, &q);
#if DBG||DEBUGCONN
    TclDo5("catch {talkmsg {recursive join on ", p, " and ", q, "}}");
#endif
    ret = (join(p, port, capacity, numtries) ? 1 :
	   join(q, port, capacity, numtries));
    UInormalcursor();
    return ret;
  }

  if (tablehostmode && capacity[0] == KIBITZER)
    sever_all_communication();
  else if (!tablehostmode && state != DISCONNECTING && state != LIMBO)
    disconnect_all();

  assert(parent == NULL);

  if (localport != NULL && strlen(myname) == 0) {
    char newname[100];
    sprintf(newname, "Doe %s", localport); /* for now, use port # as name */
    setmyname(newname, FALSE);
  }

  for (retry = 0; retry < numtries; retry++) {
    if (retry == 1) UIpatientcursor();
    if (retry > 0) {
      sprintf(s, "for {set i 0} {$i < %d} {incr i} {update; after 125}",
	      retry * retry * 8);
      TclDo(s);
    }
    if (retry > 0)
      status4("Failed to connect to ", addr, ":", port);
    status4("Attempting to connect to ", addr, ":", port);
    t = atoi(TclDo5("uplevel #0 {catch {FloaterConnect ",
		    addr, " ", port, "} joinfile}"));
    if (t == 0) break; /* it worked */
  }
  if (retry >= 1 && numtries > 1) UInormalcursor();

#if LOGGING
  fprintf(logfile, "joined %s:%s with %s\n", addr, port, TclGet("joinfile"));
#endif
#if DBG
  DEBUG(talkmsg6("joined ", addr, ":", port, " with ", TclGet("joinfile")));
#endif

  if (t != 0) {
    /* connection didn't work */

    if (tablehostmode && parent == NULL)
      if (capacity[0] == KIBITZER) 
	{ assert(0); }
      else {
	/* The following line is commented out to avoid alarming the user. */
	/* status("You've been disconnected from the table tree."); */
	/* Note that attempts to rejoin the table tree will continue... */
	UInormalcursor();
	return 0;
      }
    else status4("Giving up attempt to connect to ", addr, ":", port);

    if (!tablehostmode) {
      state = LIMBO;
      disconnect_all();
    }
    UInormalcursor();
    return 0;
  }

  status("Communication established");
  
  /* set the parent or rho variable even though we may end up being rejected */
  if (capacity[0] == KIBITZER || capacity[0] == TABLE_SERVER) {
    parent = makeconn(BOGUS_NAME, addr, port,
		      TclDo("uplevel #0 {set parentfile $joinfile}"));
    expecting_join_elsewhere = TRUE;
  }
  else if (capacity[0] == LHO) {
    /* I'm trying to join as his LHO, i.e., he is my RHO */
    if (rho != NULL) {
      disconnect(rho);
      freeconn(rho);
    }
    rho = makeconn(BOGUS_NAME, addr, port,
		   TclDo("uplevel #0 {set rhofile $joinfile}"));
  }
  sendmessage(TclDo("uplevel #0 set joinfile"),
	      makemsg5(REQUEST_JOIN, FLOATER_VERSION,
		       myname, localIPaddr, itoa(lport), capacity));
  state = CONNECTED;
  UIcommstate(state);
  UInormalcursor();
  return 1;
}

static void reject_join(char *reason, char *caps)
{
  sendmessage(connstr, makemsg2(REJECTED_JOIN, reason, caps));
  TclDo3("catch {FloaterClose ", connstr, "}");
}

/* tell the guy connected on file to please connect to addr:port instead */
static void shunt2(char *connstr,
		   char *name, char *addr, char *port, char *cap)
{
#if DEBUGCONN
  char s[200];
  sprintf(s, "Asking %s to connect to %s at %s:%s", connstr, name, addr, port);
  DEBUG(talkmsg(s));
#endif

  sendmessage(connstr, makemsg4(JOIN_ELSEWHERE, name, addr, port, cap));
}

/* Called when some new guy wants to join.  If it decides to shunt, it randomly
   selects one of my children to tell the new guy to try, and returns TRUE.
   If it decides not to shunt, it returns FALSE and does nothing. */
/* Currently only used for the table tree. */
static fl_bool shunt(char *connstr, char *cap)
{
  int numtchildren;
  childlist *c;

  for (numtchildren = 0, c = tchildren; c != NULL; numtchildren++) c = c->next;
  if (numtchildren >= MAXTCHILDREN) {
    int skip =  unifmod(numtchildren);
    for (c = tchildren; skip > 0; skip--) c = c->next;
    shunt2(connstr, c->c->name, c->c->IPaddr, c->c->port, cap);
    return TRUE;
  }
  return FALSE;
}

static void accept_join(char *name, char *IPaddr, char *port, char capacity)
{
  char caps[2];
  printf("accept_join %s", name);
  caps[0] = capacity;
  caps[1] = '\0';
  sendmessage(connstr, makemsg3(ACCEPTED_JOIN, caps, localIPaddr, localport));
  TclDo3("if ![info exists connstr] {set connstr ", connstr, "}");
  TclDo("shouldsendiamalive $connstr");
  TclDo("shouldreceiveiamalive $connstr");
  TclDo4("uplevel #0 set \\{name_to_conn(", name, ")\\} ", connstr);

  switch (capacity) {
  case KIBITZER:
    addchild(&kchildren, name, IPaddr, port, connstr);
    addwho(name);
    tellhandstatus(outputname(name), connstr); /* bring newcomer up to date */
    announce_who(); /* tell the newcomer who is here */
    tellseatstatus(connstr); /* bring the newcomer up to date */
    tellcc(connstr); /* transmit convention cards to newcomer */
    announce_tables();
    break;
  case TABLE_SERVER:
    addchild(&tchildren, name, IPaddr, port, connstr);
    addtable(name, "", IPaddr, port);
    announce_tables();
    break;
  case LHO:
    if (lho != NULL) { 
     disconnect(lho);
      freeconn(lho);
    }
#if DBG
    DEBUG(status5(name, " is connected as LHO (to ", myname,
		  ") via ", connstr));
#endif
    lho = makeconn(name, IPaddr, port, connstr);
    break;
  }
}

/* try to connect to login server; return connstr on success, otherwise NULL */
static char *connect_to_loginserver(void)
{
  int t, retry;
  char *addr, *port;

  addr = loginserveraddr();
  port = loginserverport();

  if (isin('!', addr)) {
    char *p, *q;
    addrsplit(addr, &p, &q);
    addr = p;
  }

  if (opennormalport() < 0) {
    errormsg("Unable to open a port locally; unable to log in");
    return NULL;
  }

  for (retry = 0; retry < NUMTRIES; retry++) {
    if (retry == 1) UIpatientcursor();
    if (retry > 0) {
      char s[100];

      if (!hasWindows) textrefresh();
      /* wait for retry * retry seconds */
      sprintf(s, "for {set i 0} {$i < %d} {incr i} {update; after 125}",
	      retry * retry * 8);
      TclDo(s);
    }
    if (retry > 0)
      status4("Failed to connect to ", addr, ":", port);
    status4("Attempting to connect to login server at ", addr, ":", port);
    t = atoi(TclDo5("catch {FloaterConnect ",
		    addr, " ", port, "} serverfile"));
    if (t == 0) break; /* it worked */
  }

  if (retry >= 1 && NUMTRIES > 1) UInormalcursor();

  if(t == 0) {
       /* update the local address */
       if(localIPaddr)
	    free(localIPaddr);
       localIPaddr = NULL;

       localIPaddr = STRDUP(TclDo(
            "global conn_to_sock\n"
	    "lindex [fconfigure $conn_to_sock($serverfile) -sockname] 0"));
  }
  
  return ((t == 0) ? TclDo("set serverfile") : NULL);
}

/* try to connect to result server; return connstr on success, otherwise NULL */
static char *connect_to_resultserver(void)
{
  int t, retry;
  char *addr, *port;

  addr = resultserveraddr();
  port = resultserverport();

  if (isin('!', addr)) {
    char *p, *q;
    addrsplit(addr, &p, &q);
    addr = p;
  }

  for (retry = 0; retry < NUMRSTRIES; retry++) {
    if (retry == 1) UIpatientcursor();
    if (retry > 0) {
      char s[100];

      if (!hasWindows) textrefresh();
      /* wait for retry * retry seconds */
      sprintf(s, "for {set i 0} {$i < %d} {incr i} {update; after 125}",
	      retry * retry * 8);
      TclDo(s);
    }
    if (retry > 0)
      status4("Failed to connect to ", addr, ":", port);
    status4("Attempting to connect to result server at ", addr, ":", port);
    t = atoi(TclDo5("catch {FloaterConnect ",
		    addr, " ", port, "} serverfile"));
    if (t == 0) break; /* it worked */
  }

  if (retry >= 1 && NUMRSTRIES > 1) UInormalcursor();
  return ((t == 0) ? TclDo("set serverfile") : NULL);
}

static fl_bool validname(char *name)
{
  int len = (name == NULL ? -1 : strlen(name));
  int i;

  if (len < 1) return FALSE;
  if (len > MAX_OUTPUTNAME_LENGTH) {
    errormsg3("A valid name is at most ",
	      itoa(MAX_OUTPUTNAME_LENGTH), " characters long");
    return FALSE;
  }

  for (i = 0; i < len; i++)
    if (!(isalpha(name[i]) || isdigit(name[i]) || isin(name[i], ":'`^~./?-_#")
	  || (i > 0 && name[i] == ' '))) {
      errormsg("A valid name consists of letters, spaces, digits, and :'`^~./?-_#");
      return FALSE;
    }

  return TRUE;
}

/* new is a fl_boolean---whether this is a new user */
/* if new is true then password is actually an email address */
void login(char *name, char *password, fl_bool new)
{
  char *serverfile;

  loggedin = FALSE;

  /* neither name nor password may be the empty string */
  if (!strexists(name) || !strexists(password)) return;

  name = trim(name);

  if (!validname(name)) return;

  TclSet("test_connection_succeeded", "0");

  if ((serverfile = connect_to_loginserver()) == NULL) {
    status("Giving up attempt to connect to login server");
    if (new || !prioruse(name)) {
/*      setbogusnewname(myname, localIPaddr, localport);*/
      status3("For now, you will be allowed to play under the name ",
	      myoutputname, ".");
      status("Results you get under this name will not be recorded.");
    } else {
      setmyname(name, FALSE);
      status3("You may still play as ", myoutputname,
	      ", but you won't be logged in.");
      /*status("You may use the command `help login' for details on what this means");*/
      /*status("Check the documentation for details on what this means.");*/
    }
    triedtologin = TRUE;
    return;
  }

  /* hack because changing your name will confuse others at the table */
  if (state == CONNECTED) sever_all_communication();

  sendmessage(serverfile,
	      makemsg7(LOGIN, FLOATER_VERSION, name, password,
		       localIPaddr, localport, new ? "new" : "", platform));
  status("Server contacted: logging in...");
  triedtologin = TRUE;
}

void changepw(char *name, char *old, char *new)
{
  char *serverfile;

  if (!strexists(new)) {
    errormsg("New password must be at least one character long");
    return;
  }
  if ((serverfile = connect_to_loginserver()) != NULL) {
    sendmessage(serverfile, makemsg3(CHANGEPW, name, old, new));
    reset(newpw);
    newpw = STRDUP(new);
    status("Changing password...");
  }
}

void changeemail(char *name, char *new)
{
  char *serverfile;

  if (!strexists(new)) {
    errormsg("New email must be at least one character long");
    return;
  }
  if ((serverfile = connect_to_loginserver()) != NULL) {
    sendmessage(serverfile, makemsg2(CHANGEEMAIL, name, new));
    reset(newemail);
    newemail = STRDUP(new);
    status("Changing email...");
  }
}

#ifndef SERVER
void updatelocation(void)
{
  if (!isin('*', myoutputname))
    selfmessage(makemsg2(UPDATE_LOCATION, myoutputname, location));
}
#endif

static void announce_disconn(char *s)
{
#if DEBUGCONN
  if (hasWindows) printf("%s: announce_disconn(%s)\n", myname, s);
#endif
#if LOGGING
  fprintf(logfile, "%s: announce_disconn(%s)\n", myname, s);  
#endif

  if (member(s, who)) {
#if DEBUGCONN
    DEBUG(talkmsg3("broadcasting ANNOUNCE_DISCONNECT ( ", s, " )"));
#endif
    selfmessage(makemsg1(ANNOUNCE_DISCONNECT, s));
  }
  if (membertablelist(s)) {
#if DEBUGCONN
    DEBUG(talkmsg3("broadcasting TABLE_DEMISE ( ", s, " )"));
#endif
    selfmessage(makemsg1(TABLE_DEMISE, s));
  }    
}


void listwho(void)
{
  stringlist *l;
  char s[10000], t[10000];
  extern char *northname, *southname, *eastname, *westname, *curhandID;

  if (who == NULL) {
    talkmsg("Who: nobody's here at all");
    return;
  }

  s[0] = t[0] = '\0';
  for (l = who; l != NULL && strlen(s) < 9999 - FLOATER_MAXNAME; l = l->next)
    if (curhandID == NULL || !is_spec(l->s, curhandID)) {
      sprintf(s + strlen(s), ", %s", outputname(l->s));
      if (northname != NULL && streq(northname, l->s)) strcat(s, " (N)");
      else if (southname != NULL && streq(southname, l->s)) strcat(s, " (S)");
      else if (eastname != NULL && streq(eastname, l->s)) strcat(s, " (E)");
      else if (westname != NULL && streq(westname, l->s)) strcat(s, " (W)");
    }

  if (curhandID != NULL)
    for (l = who; l != NULL && strlen(t) < 9999 - FLOATER_MAXNAME; l = l->next)
      if (is_spec(l->s, curhandID)) {
	sprintf(t + strlen(t), ", %s", outputname(l->s));
	if (northname != NULL && streq(northname, l->s)) strcat(t, " (N)");
	else if (southname != NULL && streq(southname, l->s)) strcat(t, " (S)");
	else if (eastname != NULL && streq(eastname, l->s)) strcat(t, " (E)");
	else if (westname != NULL && streq(westname, l->s)) strcat(t, " (W)");
    }

  talkmsg2("Who:", s + 1); /* s + 1 to skip first comma */
  if (t[0] != '\0')
    talkmsg2("Who (spectators):", t + 1); /* t + 1 to skip first comma */
}

/* Set *n to the number elements in the who list, and return a vector
   containing the names on the who list.  Return value is marked as garbage. */
char **getwho(int *n)
{
  int i;
  char **r;
  stringlist *l;

  if (who == NULL) {
    *n = 0;
    return NULL;
  }
  *n = length(who);
  r = (char **) markgarbage(salloc(sizeof(char *) * (*n)));
  for (i = 0, l = who; i < *n; i++, l = l->next) r[i] = l->s;
  return r;
}

static void listtables(void)
{
  tablelist *l;
  int c;
  char s[80];

  if (tables == NULL) {
    talkmsg("Tables: none");
    return;
  }

  for (c = 0, l = tables; l != NULL; (l = nexttable(l)), c++) {
    strcpy(s, "               ");
    talkmsg3("Tables: ",
	     strncpy3(s, outputname(tablename(l))),
	     tabledescription(l));
  }
  sprintf(s, "Tables: a total of %s\n", maybe_plural(itoa(c), "table"));
  talkmsg(s);
}

#define expecting_goodbye(f) TclDo5("uplevel #0 {if [info exists {expecting_goodbye(", f, ")}] {set {expecting_goodbye(", f, ")}}}")
#define set_expecting_goodbye(f, s) \
	TclDo4("uplevel #0 set {expecting_goodbye(", f, ")} ", s)
#define unset_expecting_goodbye(f) \
	TclDo3("uplevel #0 unset {expecting_goodbye(", f, ")}")

#ifdef LOGINSERVER
void gettables(void) { listtables(); }
#else
void gettables(void)
{
  char *serverfile;

  if (state == CONNECTED) {
    listtables();
    return;
  }

  if ((serverfile = connect_to_loginserver()) == NULL) {
    status("Unable to connect to login server to get fresh list of tables");
    listtables();
  } else {
    sendmessage(serverfile, makemsg0(TABLES_REQUEST));
    set_expecting_goodbye(serverfile, "listtables");
  }
}
#endif

void getfind(char *s)
{
  char *serverfile;

  if ((serverfile = connect_to_loginserver()) == NULL) {
    status("Unable to connect to login server to do `find'");
  } else {
    sendmessage(serverfile, makemsg1(FIND, s));
    /* set_expecting_goodbye(serverfile, "find"); */
  }
}

#ifdef LOGINSERVER

static void handlefindname(char *name)
{
  char s[1000];
  fl_bool hli, hei = FALSE;

  fprintf(stderr, "handlefindname(%s)\n", name);
  if (hli = have_location_info(name)) {
    sprintf(s, "%s last seen %s", name, location_info(name));
    sendmessage(connstr, makemsg1(FOUND, s));
  }
#if 0
  if (hei = have_email_info(name)) {
    sprintf(s, "email for %s is %s", name, email_info(name));
    sendmessage(connstr, makemsg1(FOUND, s));
  }
#endif
  if (!(hei || hli)) {
    if (user_exists(name)) 
      sprintf(s, "Sorry, no recent information on %s", name);
    else
      sprintf(s, "There is no user named `%s'", name);
    sendmessage(connstr, makemsg1(FOUND, s));
  }    
}

/* Go through comma separated list of names and call handlefindname()
   on each. */
static void handlefind(char *s)
{
  char *curname = TEMPCOPY(s);
  fl_bool done;
  int i;

  do {
    /* stop at next comma or end of string */
    for (i = 0; curname[i] != '\0' && curname[i] != ','; i++);
    done = (curname[i] == '\0');
    curname[i] = '\0'; /* truncate at comma */
    handlefindname(trim(curname));
    if (done) break;
    /* advance past name we just handled */
    while (*curname != '\0') curname++;
    curname++;
  } while (TRUE);
}

#endif /* LOGINSERVER */


/* announce who's here, who is N, who is S, who is tablehost, ... to new guy */
static void announce_who(void)
{
  extern char *curhandID;
  stringlist *l;

  sendmessage(connstr, makemsg0(FORGET_WHO));
  sendmessage(connstr, makemsg4(ANNOUNCE_TABLEHOST, myname, tablehostname,
				tablehostaddr, tablehostport));
  for (l = who; l != NULL; l = l->next)
    sendmessage(connstr, makemsg3(ANNOUNCE_PRESENCE, l->s,
				  (curhandID == NULL) ? "baloney" : curhandID,
				  (is_spec(l->s, curhandID) ? "1" : "0")));
}

/* announce all the tables that we know of to the new guy */
static void announce_tables(void)
{
  tablelist *l;

  /* update globaldate and globalseed if it is a new week */
  checknewweek();

/*  sendmessage(connstr, makemsg0(FORGET_TABLES));*/
  if (strlen(loginserveraddr()) > 0)
    sendmessage(connstr,
		makemsg4(ANNOUNCE_LOGINSERVER, myname, loginservername(),
			 loginserveraddr(), loginserverport()));
  if (strlen(tablerootaddr) > 0)
    sendmessage(connstr, makemsg4(ANNOUNCE_TABLEROOT, myname, tablerootname,
				  tablerootaddr, tablerootport));
#ifndef LOGINSERVER
  if (strlen(globaldate) > 0)
    sendmessage(connstr,
		makemsg2(ANNOUNCE_GLOBALDATE, globaldate, globalseed
			 /*, seconds_to_next_week()*/
			 ));
#endif
  for (l = tables; l != NULL; l = nexttable(l))
    sendmessage(connstr, makemsg4(ANNOUNCE_TABLE,
				  tablename(l), tabledescription(l),
				  tableIPaddr(l), tableport(l)));
}

static void rejoin(void)
{
  assert(tablehostmode);
  if (!strexists(rejoinaddr)) {
#if DEBUGCONN    
    DEBUG(talkmsg("Rejoining at tableroot"));
#endif
    /* try the tableroot, but only once---reset tablerootaddr after this */
    strcpy(rejoinaddr, tablerootaddr);
    strcpy(rejoinport, tablerootport);
    strcpy(rejoincap, TABLE_SERVERs);
    rejoinnumtries = NUMTRIES;
    tablerootaddr[0] = tablerootport[0] = '\0';
  }
  if (!strexists(rejoinaddr)) {
#if DEBUGCONN
    DEBUG(talkmsg("Rejoining at loginserver"));
#endif
    strcpy(rejoinaddr, loginserveraddr());
    strcpy(rejoinport, loginserverport());
    strcpy(rejoincap, TABLE_SERVERs);
    rejoinnumtries = NUMTRIES;
  }

#if DEBUGCONN
  {
    char s[2000];
    sprintf(s, "Rejoining at %s:%s as %s", rejoinaddr, rejoinport, rejoincap);
    DEBUG(talkmsg(s));
  }
#endif

  status("Attempting to rejoin the table tree (do not be alarmed).");
  if (!join(rejoinaddr, rejoinport, rejoincap, rejoinnumtries)) {
#if DEBUGCONN
    DEBUG(talkmsg("Will probably next try rejoining at loginserver"));
#endif
    strcpy(rejoinaddr, loginserveraddr());
    strcpy(rejoinport, loginserverport());
    strcpy(rejoincap, TABLE_SERVERs);
    rejoinnumtries = NUMTRIES;
  }

#if DEBUGCONN
  DEBUG(talkmsg("Done with rejoin attempt"));
#endif
}

void requestseat(int which)
{
    printf("request seat\n");
  extern fl_bool spec;
  extern fl_bool kibitzing1; /* from UI.c */
  extern int kibitzingseat; /* from UI.c; to stop hopping */
  extern char *curhandID;

  if (parent == NULL && !tablehostmode) {
    errormsg("You can't sit down when you aren't at a table");
    return;
  }
  if (which != Noseat && !unoccupied(seattochar[which])) {
    if (myseat == which) {
      char s[100];
      sprintf(s, "You are already sitting %c", seattochar[which]);
      status(s);
    } else errormsg("That seat is taken");
  } else if (seated && spec && which != Noseat) {
    errormsg("You are a spectator and you are seated at the table; therefore you aren't allowed to change seats");
  } else if (!seated && spec && which != Noseat) {
    errormsg("Spectators are not allowed to take a seat in the middle of hand");
  } else if (curhandID != NULL && which != Noseat && kibitzing1 && which != kibitzingseat) {
    errormsg("You aren't allowed to take this seat while kibitzing another one");
  } else if (curhandID != NULL && which != Noseat && kibitzingseat != Noseat && 
             which != kibitzingseat) {
    errormsg("You aren't allowed to take this seat this hand after kibitzing another one");
  } else {
    if (which != Noseat) {
      status("Requesting seat...");
      selfmessage(makemsg4(REQUEST_SEAT, myname, seattostring[which],
			   localIPaddr, localport));
    } else
      if (myseat != Noseat) selfmessage(makemsg4(REQUEST_SEAT, myname, "0",
						 localIPaddr, localport));
      else status("You already are a kibitzer");
  }
}

/* Close the connstr that sent us this message.  Used by the
   loginserver to disconnect you after it has answered you, for example. */
static void connclose(void)
{
  TclDo3("catch {FloaterClose ", connstr, "}");
}

static void handlemessage(message *m, char *msg)
{
  int numargs = countargs(m);
  char **argv = m->args;
#if DBG
  static int msgcount = 0;
#endif
  if (m->kind == '\0') executionlog("handlemessage: ", "null kind");
  else {
    char s[2];
    s[0] = m->kind;
    s[1] = '\0';
    executionlog("handlemessage: ", s);
  }

  if (tablebroadcast(m->kind) || globalbroadcast(m->kind)) rebroadcast(m);

#if DBG
  if (hasWindows)
    printf("%s(%d): start with %c msg\n", myoutputname, msgcount, m->kind);
#endif
  assert(m->kind != '\0');

  if (!hasWindows) needrefresh = TRUE;
  switch (m->kind) {

    /* Note: bridge specific stuff follows */

#ifdef RESULTSERVER
  case REQUEST_RESULT:
    if (numargs != 2) goto wrongargs;
    resultrequest(argv[1], connstr);
    if (argv[0][0] == 'Y') connclose();
    break;
  case RESULT:
    if (numargs != 9) goto wrongargs;
    if (isCompetitiveHandname(argv[0]))
      result(argv[0], atoi(argv[1]), argv[2], argv[3], argv[4], argv[5],
	     argv[6], argv[7], argv[8], msg);
    if (!startingup) {
      resultrequest(argv[0], connstr);
      connclose();
    }
    break;
#else
  case RESULT:
    if (numargs != 9) goto wrongargs;
    result(argv[0], atoi(argv[1]), argv[2], argv[3], argv[4], argv[5],
	   argv[6], argv[7], argv[8], msg);
    break;
#endif
  case RESULT_DUMP:
    if (numargs != 4) goto wrongargs;
    if (streq(argv[1], "done")) {
      if (atoi(argv[2]) > 1 || haveseen(atoi(argv[2]), atoi(argv[3])))
	dumpresult(argv[0]);
    } else tabmulti(argv[1]);
    break;

  case PASS_STATUS:
    if (numargs != 2) goto wrongargs;
    if (filehandinfo(m)) broadcast_curpassstate();
    break;

  case NEW_HAND:
    if (numargs != 8) goto wrongargs; else goto fileinfo;
  case AUCTION_STATUS:
    if (numargs != 2) goto wrongargs; else goto fileinfo;
  case PLAY_STATUS:
    if (numargs != 2) goto wrongargs;
    /* Only tablehost can end the hand */
    if (numcards_(argv[1]) == 52 && !streq(m->from, tablehostname)) {
      if (streq(myname, tablehostname))
	selfmessage(makemsg2(PLAY_STATUS, argv[0], argv[1]));
      else
	sendtoparent(msgtotext(m));
      break;
    }
    /* fall through */
  fileinfo:
    if (filehandinfo(m)) {
      if (seated) UIupdate();
      rebroadcast(m);
    }
    break;

  case ANNOUNCE_GLOBALDATE:
    /* this message is used both to announce the date and seed, and to
       request a SEEN message from everyone who is seated */
    if (numargs != 2) goto wrongargs;
    if (!streq(argv[0], globaldate)) {
      setglobaldate(argv[0]);
      setglobalseed(argv[1]);
      /*    set_seconds_to_next_week(argv[2]);*/
      resetseen();
    }
    if (seated) {
      selfmessage(makemsg3(SEEN, seattostring[myseat], "0",
			   TclDo3("seen ", globaldate, "IMP")));
      selfmessage(makemsg3(SEEN, seattostring[myseat], "1",
			   TclDo3("seen ", globaldate, "MP")));
    }
    break;

  case YOU_HAVE_SEEN:
    /* This message type is sent by the loginserver to a player who logs in.
       First arg is set name; second arg is compressed bit vector indicating
       which boards have been seen. */
    if (numargs != 2) goto wrongargs;
    parse_you_have_seen(argv[0], argv[1]);
    break;

  case SEEN:
    if (numargs != 3) goto wrongargs;
    if (tablehostmode) {
      parseseen(argv[0][0], atoi(argv[1]), argv[2]);
    } else if (parent != NULL) sendmessage(parent->connstr, m);
    break;
    
    /* CC_TO_HOST message type just goes up to the table host, who then
       broadcasts a CC message.  This keeps everyone's view coherent. */
  case CC_TO_HOST:
    if (tablehostmode) {
      if (numargs != 2) goto wrongargs;
      if (!streq(argv[0], "EW") && !streq(argv[0], "NS")) goto undecipherable;
      selfmessage(makemsg2(CC, argv[0], argv[1]));
    } else if (parent != NULL) sendmessage(parent->connstr, m);
    break;
    
  case CC:
    if (numargs != 2) goto wrongargs;
    if (!streq(argv[0], "EW") && !streq(argv[0], "NS")) goto undecipherable;
    ccupdate(argv[0], argv[1]);
    break;

  case REQUEST_SEAT:
    if (tablehostmode) {
      if (numargs != 4) goto wrongargs;
      if (!isin(argv[1][0], "NSEW0")) goto undecipherable;
      if (argv[1][0] == '0') {
	/* someone getting up from a seat */
	extern char *northname, *southname, *eastname, *westname;
	char ch;

	if (safestreq(m->from, northname)) ch = 'N';
	else if (safestreq(m->from, southname)) ch = 'S';
	else if (safestreq(m->from, eastname)) ch = 'E';
	else if (safestreq(m->from, westname)) ch = 'W';
	else goto undecipherable;
	parseseen(ch, -1, "?");
	selfmessage(makemsg4(SEATED, argv[0], argv[1], argv[2], argv[3]));
      } else if (unoccupied(argv[1][0])) {
	/* we don't know what new guy has seen */
	parseseen(argv[1][0], -1, "?");
	selfmessage(makemsg4(SEATED, argv[0], argv[1], argv[2], argv[3]));
      }
    } else if (parent != NULL) sendmessage(parent->connstr, m);
    break;
    
  case SEATED:
    if (numargs != 4) goto wrongargs;
    printf("seated myname %s, %s\n",myname, argv[0]);
    if (streq(myname, argv[0])) {
      if (isin(argv[1][0], "NSEW")) {
          printf("comm handlemessage seated\n");
	extern fl_bool duplicate; /* from br.c */
	if (tablehostmode) checknewweek();
	selfmessage(makemsg3(SEEN, argv[1], "0",
			     TclDo3("seen ", globaldate, "IMP")));
	selfmessage(makemsg3(SEEN, argv[1], "1",
			     TclDo3("seen ", globaldate, "MP")));
      }
    }
    sit(argv[0], argv[1][0], argv[2], argv[3]);
    break;
    
    /* Note: end of bridge specific section */
    
  case SAY:
    if (numargs != 2) goto wrongargs;
    talkmsg3(argv[0], ": ", argv[1]);
    break;
    
  case SAY_TO_ONE:
    if (numargs != 3) goto wrongargs;
    {
      fl_bool senderspec = atob(argv[0]);
      char *who = argv[1], *what = argv[2], *owho = outputname(who);

      extern fl_bool spec;
      
      if (streq(who, myoutputname)) {
	if (streq(what, BEEP_STR)) {
	  UIbeep();
	  status4("BEEP! (from ", outputname(m->from), ") @ ", markgarbage(hhmmss()));
	} else if (spec == senderspec || !senderspec)
	  talkmsg3(owho, "- ", what);
	else
	  warningmsg2(owho,
		     ", a spectator, is trying to send you a private message");
      } else if (streq(what, BEEP_STR))
	status4("Beep sent from ", outputname(m->from), " to ", owho);
    }
    break;
    
  case SAY_TO_SPEC:
    if (numargs != 2) goto wrongargs;
    {
      extern fl_bool spec; /* from UI.c */
      if (spec) talkmsg3(argv[0], "% ", argv[1]);
    }
    break;
    
  case SAY_EXCEPT_PARD:
    if (numargs != 3) goto wrongargs;
    if (!(seated && myseat == (atoi(argv[2]) ^ 2)))
      talkmsg3(argv[0], "! ", argv[1]);
    break;
    
  case SAY_TO_OPP:
    if (numargs != 4 && numargs != 3) goto wrongargs;
    if (seated && (myseat == atoi(argv[2]) ||
		   (numargs == 4 && myseat == atoi(argv[3]))))
      talkmsg3(argv[0], numargs == 3 ? "- " : "= ", argv[1]);
    break;
    
  case JOIN_ELSEWHERE:
    /* request that we join somewhere else */
    if (numargs != 4) goto wrongargs;
    if (parent == NULL) goto spoof;  /* spoofing?? */
    if (!streq(m->from, parent->name))
      if (!isparentname(BOGUS_NAME) ||
	  !expecting_join_elsewhere)
	goto spoof;    /* spoofing?? */
    oldparent = parent;
    parent = NULL;
/*    puts("setting parent to NULL at 1418"); */
    status("Shunted...");
    if (join(argv[1], argv[2], argv[3], NUMTRIES)) {
      /* communication established with new parent */
      TclDo2("FloaterClose ", oldparent->connstr);
      freeconn(oldparent);
    } else {
      parent = oldparent;
      oldparent = NULL;
    }
    break;
    
  case REJECTED_JOIN:
    /* rejected outright --- compare with JOIN_ELSEWHERE */
    if (numargs != 2) goto wrongargs;
    if (isupper(argv[0][0])) status(argv[0]);
    else status3(outputname(m->from), " ", argv[0]);
    if (argv[1][0] == KIBITZER || argv[1][0] == TABLE_SERVER) {
      freeconn(parent);
      parent = NULL;
/*      puts("setting parent to NULL at 1438");*/
      if (!tablehostmode) {
	state = LIMBO;
	if_not_connected_reset_tablehostname();
	UIcommstate(state);
      }
    }
    else if (rho != NULL && streq(m->from, rho->name)) {
      freeconn(rho);
      rho = NULL;
    }
    expecting_join_elsewhere = FALSE;
    rejoinaddr[0] = rejoinport[0] = '\0';
    break;
    
  case ACCEPTED_JOIN:
    TclDo2("shouldsendiamalive ", connstr);
    TclDo2("shouldreceiveiamalive ", connstr);
    switch (argv[0][0]) {
    case KIBITZER:
#ifndef SERVER
      sprintf(location, "kibitzing at %s's table", outputname(m->from));
      TclDo("uplevel #0 {set table_arrival_time $floaterclock}");
#endif
      broadcast(makemsg1(ANNOUNCE_CONNECT, myname));
      /* fall through */
    case TABLE_SERVER:
      addwho(myname);
      state = CONNECTED;
      UIcommstate(state);

      /* Copy info for possible later use if we are unexpectly disconnected;
	 currently, however, this information is never used---we always try
	 to rejoin the table tree at the top, not at our parent who left us. */
      STRCPY(rejoinaddr, argv[1] /* parent->IPaddr */);
      STRCPY(rejoinport, argv[2] /* parent->port */);
      STRCPY(rejoincap, argv[0]);

      if (argv[0][0] == TABLE_SERVER) 
	selfmessage(makemsg4(ANNOUNCE_TABLE, myname, mytabledesc,
			     localIPaddr, localport));
      expecting_join_elsewhere = FALSE;
      break;
    case LHO:
      assert(LHO_RHO_CONNECTIONS);
      if (rho != NULL) freeconn(rho);
      rho = makeconn(m->from, argv[1], argv[2], connstr);
    }
    break;
    
  case REQUEST_JOIN:
    if (numargs != 5) goto wrongargs;
    {
      char capacity = argv[4][0], *version = argv[0], *name = argv[1];
      char *IPaddr = argv[2], *port = argv[3], *caps = argv[4];

      /* would check version number for compatibility here */

      switch (capacity) {
#ifndef SERVER
      case KIBITZER:
	if (accepting_kibitzers) accept_join(name, IPaddr, port, capacity);
	else reject_join(tablehostmode ?
			 (nokibitzers ?
			  "The table is not accepting new connections" :
			  "is too busy") :
			 "is not hosting a table", caps);
	break;
      case TABLE_SERVER:
	if (accepting_tableservers) {
	  if (!shunt(connstr, caps))
	    accept_join(name, IPaddr, port, capacity);
	} else reject_join(tablehostmode ? "is too busy" :
			   "is not a table server", caps);
	break;
	/* bridge specific */
      case LHO:
	if (lho == NULL || streq(name, lho->name))
	  accept_join(name, IPaddr, port, capacity);
	else reject_join("You aren't the LHO!", caps);
	break;
	/* end of bridge specific */
#endif
#ifdef LOGINSERVER
      case TABLE_SERVER:
#if 0  /* is it necessary to change this stuff here?! */
	STRCPY(loginservername, myname);
	STRCPY(loginserveraddr, localIPaddr);
	STRCPY(loginserverport, localport);
	/* status4(localIPaddr, ":", localport, " is loginserver location"); */
#endif

	if (version[0] != '8' && version[0] != '9') {
	  sendmessage(connstr,
		      makemsg1(INFO,
			       "Please upgrade to version 1.2 or higher!"));
	  reject_join("You must upgrade to host a table", caps);
	  break;
	}

	if (strlen(tablerootaddr) == 0) {
	  status2(name, " is tableroot");
	  STRCPY(tablerootname, name);
	  STRCPY(tablerootaddr, IPaddr);
	  STRCPY(tablerootport, port);
	  accept_join(name, IPaddr, port, capacity);
	} else assert((tchildren != NULL) && shunt(connstr, caps));
	break;
      case KIBITZER:
      case LHO:
#endif
      default:
	reject_join("doesn't understand in what capacity you're trying to join", caps);
      }
    }
    break;
    
  case TABLE_DEMISE:
    if (numargs != 1) goto wrongargs;
    tables = removetable(argv[0], tables);
    update_activity_webpage_with_table_info();
    if (streq(argv[0], tablerootname))
      tablerootname[0] = tablerootaddr[0] = tablerootport[0] = '\0';
    if (streq(tablehostname, argv[0])) {
      /* the table I'm at is closing, so don't try to rejoin */
      status3("The table (hosted by ", outputname(argv[0]), ") has closed.");
      sever_all_communication();
      state = LIMBO;
      if_not_connected_reset_tablehostname();
      UIcommstate(state);
    }
    break;
    
  case ANNOUNCE_DISCONNECT:
    if (numargs != 1) goto wrongargs;
    TclDo3("FloaterCloseName {", argv[0], "}");
    persongone(argv[0]); /* handles bookkeeping if it was a player who left */
    if (streq(argv[0], tablerootname)) /* shouldn't happen, I don't think */
      tablerootname[0] = tablerootaddr[0] = tablerootport[0] = '\0';
    removewho(argv[0]);
    if (tablehostmode) redescribetable();
    break;
    
  case ANNOUNCE_CONNECT:
    if (numargs != 1) goto wrongargs;
    addwho(argv[0]);
#ifdef BEEPONJOIN
    UIbeep();
#endif
    if (tablehostmode) redescribetable();
    break;
    
  case FORGET_WHO:
    if (numargs != 0) goto wrongargs;
#if DEBUGCONN
    DEBUG(status("Clearing `who' list"));
#endif
    while (who != NULL) who = freestringlist(who);
    addwho(myname);
    break;
    
/*
  case FORGET_TABLES:
    if (numargs != 0) goto wrongargs;
    while (tables != NULL) tables = freetablelist(tables);
    if (tablehostmode) addtable(myname, mytabledesc, localIPaddr, localport);
    break;
*/    
  case ANNOUNCE_PRESENCE:
    if (numargs != 3) goto wrongargs;
    addwho(argv[0]);
    if (argv[2][0] == '1') goto makespec;
    break;
    

  case KIB1:
    /* args are kibber, seat being kibbed (should be playername?) and handID */
    if (numargs != 3) goto wrongargs;
    { /* awkward construct savagely cloned */
      extern char *curhandID;
      if (curhandID == NULL || !streq(curhandID, argv[2]))
	status4(outputname(argv[0]), " is a kibitzer (for ",
		TclDo3("global handname; if [catch {set handname(",
		       argv[2], ")} n] "
		       "{return {next hand}} {return \"hand $n\"}"),
		")");
      else
	status4(outputname(argv[0]), " is now kibitzing ",argv[1], ".");
    }
    break;

    /* SPEC is somewhat bridge specific (might apply in similar games too) */
  case SPEC:
    /* args are name of person going spec, and handID */
    if (numargs != 2) goto wrongargs;
  makespec:
    {
      extern char *curhandID;
      make_spec(argv[0], argv[1]);
      if (curhandID == NULL || !streq(curhandID, argv[1]))
	status4(outputname(argv[0]), " is a spectator (for ",
		TclDo3("global handname; if [catch {set handname(",
		       argv[1], ")} n] "
		       "{return {next hand}} {return \"hand $n\"}"),
		")");
      else
	status2(outputname(argv[0]), " is a spectator");
    }
    break;

  case ANNOUNCE_TABLE:
    if (numargs != 4) goto wrongargs;
    addtable(argv[0], argv[1], argv[2], argv[3]);
    update_activity_webpage_with_table_info();
    break;
    
  case ANNOUNCE_LOGINSERVER:
    if (numargs != 4) goto wrongargs;
    if (tablerootmode && streq(m->from, argv[0])) {
      if (parent == NULL)
	parent = makeconn(argv[0], argv[1], argv[2], connstr);
      parent->name = STRDUP(argv[0]);
    }
    
    argv[0] = TEMPCOPY(myname); /* use my name for broadcast to my children */
    TclSet("loginservername", argv[1]);
    TclSet("loginserveraddr", argv[2]);
    TclSet("loginserverport", argv[3]);
    rebroadcast(m);
    break;
    
  case ANNOUNCE_TABLEHOST:
    if (numargs != 4) goto wrongargs;
    if (isparentname(BOGUS_NAME)) strncpy2(parent->name, argv[0],
					   FLOATER_MAXNAME - 1);
    argv[0] = TEMPCOPY(myname); /* use my name for broadcast to my children */
    STRCPY(tablehostname, argv[1]);
    STRCPY(tablehostaddr, argv[2]);
    STRCPY(tablehostport, argv[3]);
    rebroadcast(m);
    break;
    
  case ANNOUNCE_TABLEROOT:
    if (numargs != 4) goto wrongargs;
    if (isparentname(BOGUS_NAME)) strncpy2(parent->name, argv[0],
					   FLOATER_MAXNAME - 1);
    argv[0] = TEMPCOPY(myname);
    STRCPY(tablerootname, argv[1]);
    STRCPY(tablerootaddr, argv[2]);
    STRCPY(tablerootport, argv[3]);
    tablerootmode = tablehostmode && streq(myname, tablerootname) &&
      streq(localport, tablerootport) && streq(localIPaddr, tablerootaddr);
    rebroadcast(m);
    break;
    
  case UPDATE_LOCATION:
    if (numargs != 2) goto wrongargs;
#ifdef LOGINSERVER
    set_location_info(argv[0], argv[1]);
#else
    if (parent != NULL) sendmessage(parent->connstr, m);
#endif
    break;

#ifdef LOGINSERVER
  case CHANGEPW:
    if (numargs != 3) goto wrongargs;
    if (strexists(argv[1]) && streq(argv[1], password(argv[0]))) {
      savenewpw(argv[0], argv[2]);
      sendmessage(connstr, makemsg3(CHANGEDPW, argv[0], argv[1], argv[2]));
    } else
      sendmessage(connstr, makemsg0(WRONG_PASSWORD));
    break;

  case CHANGEEMAIL:
    if (numargs != 2) goto wrongargs;
    if (!streq(outputname(m->from), argv[0])) goto spoof;
    savenewemail(argv[0], argv[1]);
    sendmessage(connstr, makemsg2(CHANGEDEMAIL, argv[0], argv[1]));
    break;

  case LOGIN:
    if (numargs != 7) goto wrongargs;
    {
      char *IPaddr = argv[3], *port = argv[4];
      if (!streq(argv[0], FLOATER_VERSION)) {
	sendmessage(connstr, makemsg1(INFO, TEMPCAT("You are using an outdated version!  Please consider upgrading to ", TclGet("floater_version"))));
	sendmessage(connstr, makemsg1(INFO, "The latest news is always at http://www.floater.org/"));
	if (argv[0][0] != FLOATER_VERSION[0]) {
	  sendmessage(connstr,
		      makemsg1(INFO, "Log in request rejected!  Sorry."));
	  connclose();
	  goto done;
	}
      }

      fprintf(stderr, "login: %s on %s using %s\n", argv[1], argv[6], argv[0]);

      if (streq(argv[5], "new")) {
	if (nameinuse(argv[1])) sendmessage(connstr, makemsg0(NAME_IN_USE));
	else {
	  sendmessage(connstr, makemsg2(LOGGED_IN, argv[1],
					testConn2(argv[0], IPaddr,
						  port, argv[5])));
	  TclDo5("notelogin {", argv[1], "} {", floatertime(), "}");
	  announce_tables();
	  newaccount(argv[1], argv[2]);
	}
      } else {
	if (strexists(argv[2]) && streq(argv[2], password(argv[1]))) {
	  /* won't work---need to kill the existing table!gCai... not argv[1]
	     if (omembertablelist(argv[1]))
	     selfmessage(makemsg1(TABLE_DEMISE, argv[1]));
	   */
	  sendmessage(connstr, makemsg1(INFO, "Welcome to Floater."));
	  sendmessage(connstr, makemsg1(INFO, "You may join a table if you like, or else host a new one."));
	     
	  announce_seen(connstr, argv[1]);
	  announce_tables();
	  sendmessage(connstr, makemsg2(LOGGED_IN, argv[1],
					testConn2(argv[0], IPaddr,
						  port, argv[5])));
	  TclDo5("notelogin {", argv[1], "} {", floatertime(), "}");
	  update_activity_webpage_with_table_info();
	} else
	  sendmessage(connstr, makemsg0(WRONG_PASSWORD));
      }
    }
    break;
#else
  case CHANGEDPW:
    if (numargs != 3) goto wrongargs;
    if (newpw != NULL && streq(newpw, argv[2])) {
      status2("Password changed for ", argv[0]);
      reset(newpw);
    }
    connclose();
    break;

  case CHANGEDEMAIL:
    if (numargs != 2) goto wrongargs;
    if (newemail != NULL && streq(newemail, argv[1])) {
      status2("Email address changed for ", argv[0]);
      reset(newemail);
    }
    connclose();
    break;

  case LOGGED_IN:
    /* First arg is my name; Second is two things concatenated:
       "new" or "" followed by "y" or "n".  The first is whether I'm
       new and should expect my password to arrive by email.
       The second is whether I am allowed to host a table. */
    if (numargs != 2) goto wrongargs;
    if (loggedin) goto spoof;
    triedtologin = loggedin = TRUE;
    setmyname(argv[0], TRUE);
    status2("You are now logged in as ", myoutputname);
    TclDo3("catch {wm title . {Floater: ", myoutputname, "}}");
    TclDo3("catch {wm iconname . {Floater: ", myoutputname, "}}");
    if (strneq(argv[1], "new", 3))
      status("You will receive your password by email");
    setprioruse(myname);
    connclose();
    sever_all_communication();
    break;
    
  case WRONG_PASSWORD:
    triedtologin = TRUE;
    if (numargs != 0) goto wrongargs;
    UIwrongpassword();
    connclose();
    break;
    
  case NAME_IN_USE:
    if (numargs != 0) goto wrongargs;
    if (!triedtologin || loggedin) goto spoof;
    UInameinuse();
    connclose();
    break;
    
  case GOODBYE:
    if (numargs != 0) goto wrongargs;
    {
      char *s = expecting_goodbye(connstr);

      if (!strexists(s)) goto spoof;
      if (streq(s, "listtables")) listtables();
      connclose();
      unset_expecting_goodbye(connstr);
    }
    break;

    /* claiming is bridge specific */
  case CLAIM:
  case REJECT_CLAIM:
  case ACCEPT_CLAIM:
  case RETRACT_CLAIM:
    if (numargs != 2) goto wrongargs;
    handleclaimmessage(m->kind, argv[0], atoi(argv[1]), m->from);
    break;
#endif
    
#ifdef LOGINSERVER
  case FIND:
    if (numargs != 1) goto wrongargs;
    handlefind(argv[0]);
    connclose();
    break;
#endif

  case FOUND:
    if (numargs != 1) goto wrongargs;
    foundmsg(argv[0]);
    break;

  case TABLES_REQUEST:
    if (numargs != 0) goto wrongargs;
    announce_tables();
    sendmessage(connstr, makemsg0(GOODBYE));
    break;

  case INFO:
    if (numargs != 1) goto wrongargs;
    talkmsg2("Info: ", argv[0]);
    break;

  case MULTIMESSAGE:
    {
      int i;
      for (i = 0; i < numargs; i++)
	handlemessage(parsemessage(argv[i]), argv[i]);
    }
    break;

  case '\0':
    /* Shouldn't happen, but ignore it unless we're debugging. */
#if !DBG
    return;
#endif
    /* fall through */

  undecipherable:
  spoof:
  wrongargs:
  default:
    {
      int i;
      char s[100];

      sprintf(s, "Received a bogus %c message from %s with %d args",
	      m->kind, connstr, numargs);
#if DBG
      DEBUG(talkmsg(s));
      for (i=0; i<numargs; i++) DEBUG(talkmsg(argv[i]));
#else
      warningmsg(s);
#endif
    }

#ifdef LOGINSERVER
    connclose();
#endif    
  }
done:
#if DBG
  if (hasWindows)
    printf("%s(%d): done with %c msg\n", myoutputname, msgcount++, m->kind);
#endif
  ;
}


