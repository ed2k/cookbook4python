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
#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/* the possible kinds of message follow */

#define MULTIMESSAGE 'M'

/* first, non-broadcast message types */
#define TABLES_REQUEST 't'
#define GOODBYE 'g'
#define LOGIN 'L'
#define LOGGED_IN 'i'
#define WRONG_PASSWORD 'w'
#define CHANGEPW '_'
#define CHANGEDPW '='
#define CHANGEEMAIL 'G'
#define CHANGEDEMAIL 'h'
#define NAME_IN_USE 'u'
#define REQUEST_JOIN 'J'
#define REJECTED_JOIN 'N'
#define ACCEPTED_JOIN 'Y'
#define JOIN_ELSEWHERE 'E'
#define REQUEST_SERVE '['
#define SERVE ']'
#define RESULT 'I'
#define INFO 'y'
#define REQUEST_RESULT 'R'
#define YOU_HAVE_SEEN 'U'
#define FIND 'b'
#define FOUND 'B'
#define nonbroadcast(c) (isin((c), "MtgLiw_=GhuJNYE[]IyRUbB"))

/* messages which are manually rebroadcast (sometimes) */
#define UPDATE_LOCATION 'k'
#define ANNOUNCE_LOGINSERVER 'l'
#define ANNOUNCE_TABLEHOST 'H'
#define ANNOUNCE_TABLEROOT '^'
#define PLAY_STATUS 'p'
#define PASS_STATUS 'j'
#define AUCTION_STATUS 'a'
#define NEW_HAND '*'
#define REQUEST_SEAT 'S'
#define SEEN 'e'
#define CC_TO_HOST 'v'
#define manualrebroadcast(c) (isin((c), "klH^pja*Sev"))

/* table-broadcast message types */
#define RESULT_DUMP 'z'
#define SAY '"'
#define SAY_EXCEPT_PARD '\''
#define SAY_TO_ONE '/'
#define SAY_TO_OPP 'O'
#define SAY_TO_SPEC '%'
#define ANNOUNCE_CONNECT 'C'
#define ANNOUNCE_DISCONNECT 'D'
#define ANNOUNCE_PRESENCE 'P'
#define FORGET_WHO 'F'
#define SEATED 's'
#define CLAIM 'c'
#define REJECT_CLAIM 'r'
#define ACCEPT_CLAIM 'A'
#define RETRACT_CLAIM 'o'
#define ANNOUNCE_GLOBALDATE 'd'
#define KIB1 'K'
#define SPEC 'W'
#define CC 'V'
#define tablebroadcast(c) (isin(c, "z\"'/O%CDPFscrAodKWV"))

/* global-broadcast message types */
#define FORGET_TABLES 'f'
#define ANNOUNCE_TABLE 'T'
#define TABLE_DEMISE '-'
#define globalbroadcast(c) (isin(c, "fT-"))

#define validmessagekind(c)						\
(globalbroadcast(c) || tablebroadcast(c) || manualrebroadcast(c)	\
 || nonbroadcast(c))

#define MAXMSGARGS 10
typedef struct {
  char kind;
  char **args;
  char *from, *ID;
} message;

message *makemsg10(const char kind,
		   const char *a, const char *b, const char *c,
		   const char *d, const char *e, const char *f,
		   const char *g, const char *h, const char *i,
		   const char *j);

#define makemsg9(kind, a, b, c, d, e, f, g, h, i) makemsg10(kind, a, b, c, d, e, f, g, h, i, NULL)
#define makemsg8(kind, a, b, c, d, e, f, g, h) makemsg9(kind, a, b, c, d, e, f, g, h, NULL)
#define makemsg7(kind, a, b, c, d, e, f, g) makemsg8(kind, a, b, c, d, e, f, g, NULL)
#define makemsg6(kind, a, b, c, d, e, f) makemsg7(kind, a, b, c, d, e, f, NULL)
#define makemsg5(kind, a, b, c, d, e) makemsg6(kind, a, b, c, d, e, NULL)
#define makemsg4(kind, a, b, c, d) makemsg5(kind, a, b, c, d, NULL)
#define makemsg3(kind, a, b, c) makemsg4(kind, a, b, c, NULL)
#define makemsg2(kind, a, b) makemsg3(kind, a, b, NULL)
#define makemsg1(kind, a) makemsg2(kind, a, NULL)
#define makemsg0(kind) makemsg1(kind, NULL)

/* the different capacities in which one may wish to join */
#define KIBITZER 'K'
#define TABLE_SERVER 'T'
#define LHO 'L'
#define KIBITZERs "K"
#define TABLE_SERVERs "T"
#define LHOs "L"

#endif
