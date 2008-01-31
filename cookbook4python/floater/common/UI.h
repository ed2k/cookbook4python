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
 *    Lex Spoon <lex@cc.gatech.edu>
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

/* UI.h -- high-level user interface calls */
   
#ifndef __ui_H__
#define __ui_H__

extern char *vulnames[];

#define fulltable()
#define notfulltable()

void UIcommstate(int state);
void UIwrongpassword(void);
void UInameinuse(void);
void UIupdate(void);
void UIplayergone(int which);
void UIsit(char *name, char ch, int myseat);
void UIupdatestatus(void);
void UIspec(void);
void UItablehost(void);
void UIheartshandover(int ns, int es, int ss);
void UIpassset(char *t);
void UItogglepassedcard(char *p);

void setstatusline(const char *s);
void setinfoline(const char *s);

void UIbeep(void);

void UIinsertTalk(const char *s);
void UIinsertCmd(const char *s);

void UIpatientcursor(void);
void UInormalcursor(void);


#endif /* __ui_H__ */

