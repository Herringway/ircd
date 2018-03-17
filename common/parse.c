/************************************************************************
 *   IRC - Internet Relay Chat, common/parse.c
 *   Copyright (C) 1990 Jarkko Oikarinen and
 *                      University of Oulu, Computing Center
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * $Id: parse.c,v 6.1 1991/07/04 21:03:58 gruner stable gruner $
 *
 * $Log: parse.c,v $
 * Revision 6.1  1991/07/04  21:03:58  gruner
 * Revision 2.6.1 [released]
 *
 * Revision 6.0  1991/07/04  18:04:51  gruner
 * frozen beta revision 2.6.1
 *
 */

/* -- Jto -- 03 Jun 1990
 * Changed the order of defines...
 */

char parse_id[] = "parse.c v2.0 (c) 1988 University of Oulu, Computing Center and Jarkko Oikarinen";

#include <ctype.h>
#include "struct.h"
#include "common.h"
#define MSGTAB
#include "msg.h"
#undef MSGTAB
#include "sys.h"
#include "numeric.h"

extern aClient *client;
extern aClient *hash_find_client(), *hash_find_server();

static char sender[HOSTLEN+1];

int myncmp(str1, str2, n)
char *str1, *str2;
int n;
    {
	while ((islower(*str1) ? toupper(*str1) : *str1) ==
	       (islower(*str2) ? toupper(*str2) : *str2))
	    {
		str1++; str2++; n--;
		if (n == 0 || (*str1 == '\0' && *str2 == '\0'))
			return(0);
	    }
	return(1);
    }

/*
**  Case insensitive comparison of two NULL terminated strings.
**
**	return	0, if equal
**		1, if not equal
*/
int mycmp(str1, str2)
Reg1 char *str1;
Reg2 char *str2;
    {
	while ((islower(*str1) ? toupper(*str1) : *str1) ==
	       (islower(*str2) ? toupper(*str2) : *str2))
	    {
		if (*str1 == '\0')
			return(0);
		str1++;
		str2++;
	    }
	return 1;
    }

/*
**  Find a client (server or user) by name.
**
**  *Note*
**	Semantics of this function has been changed from
**	the old. 'name' is now assumed to be a null terminated
**	string and the search is the for server and user.
*/
#ifndef CLIENT_COMPILE
aClient *find_client(name, cptr)
char *name;
aClient *cptr;
    {
	Reg1 aClient *c2ptr;

	if (name) {
		c2ptr = hash_find_client(name, cptr);
		return c2ptr;
	}
	return cptr;
    }
#else
aClient *find_client(name, cptr)
char *name;
aClient *cptr;
    {
	Reg1 aClient *c2ptr;

	if (name)
		for (c2ptr = client; c2ptr; c2ptr = c2ptr->next) 
			if (mycmp(name, c2ptr->name) == 0)
				return c2ptr;
	return cptr;
    }
#endif

/*
**  Find a user@host (server or user).
**
**  *Note*
**	Semantics of this function has been changed from
**	the old. 'name' is now assumed to be a null terminated
**	string and the search is the for server and user.
*/
aClient *find_userhost(user, host, cptr, count)
char *user, *host;
aClient *cptr;
int *count;
    {
	Reg1 aClient *c2ptr;
	Reg2 aClient *res = (aClient *) 0;

	*count = 0;
	if (user && host)
	  for (c2ptr = client; c2ptr; c2ptr = c2ptr->next) 
	    if (c2ptr->user)
	      if (matches(host, c2ptr->user->host) == 0 &&
		  mycmp(user, c2ptr->user->username) == 0) {
		(*count)++;
		res = c2ptr;
	      }
	if (res)
	  return res;
	else
	  return cptr;
    }

/*
**  Find server by name.
**
**	This implementation assumes that server and user names
**	are unique, no user can have a server name and vice versa.
**	One should maintain separate lists for users and servers,
**	if this restriction is removed.
**
**  *Note*
**	Semantics of this function has been changed from
**	the old. 'name' is now assumed to be a null terminated
**	string.
*/
#ifndef CLIENT_COMPILE
aClient *find_server(name, cptr)
char *name;
aClient *cptr;
{
  Reg1 aClient *c2ptr = (aClient *) 0;

  if (name) {
    if (c2ptr = hash_find_server(name, (aClient *)NULL))
      return (c2ptr);
    if (!index(name, '*'))
      return (aClient *)NULL;
    for (c2ptr = client; c2ptr; c2ptr = c2ptr->next) {
      if (!IsServer(c2ptr) && !IsMe(c2ptr))
	continue;
      if (matches(name, c2ptr->name) == 0)
	break;
      if (index(c2ptr->name, '*'))
        if (matches(c2ptr->name, name) == 0)
          break;
    }
  }
  return c2ptr;
}
#else
aClient *find_server(name, cptr)
char *name;
aClient *cptr;
{
  Reg1 aClient *c2ptr = (aClient *) 0;

  if (name) {
    for (c2ptr = client; c2ptr; c2ptr = c2ptr->next) {
      if (!IsServer(c2ptr) && !IsMe(c2ptr))
	continue;
      if (matches(c2ptr->name, name) == 0 ||
	  matches(name, c2ptr->name) == 0)
	break;
    }
  }
  return c2ptr;
}
#endif

/*
**  Find person by (nick)name.
*/
aClient *find_person(name, cptr)
char *name;
aClient *cptr;
    {
	aClient *c2ptr = find_client(name, (aClient *)NULL);

	if (c2ptr != NULL && !IsServer(c2ptr) && !IsMe(c2ptr))
		return c2ptr;
	else
		return cptr;
    }

parse(cptr, buffer, length, mptr)
aClient *cptr;
char *buffer;
int length;
struct Message *mptr;
    {
	aClient *from = cptr;
	Reg2 char *ch;
	char *ch2, *para[MAXPARA+1];
	int len, i, numeric, paramcount;

	debug(DEBUG_DEBUG,"Parsing: %s",buffer);
	*sender = '\0';
	for (ch = buffer; *ch == ' '; ch++);
	para[0] = cptr->name;
	if (*ch == ':')
	    {
		/*
		** Copy the prefix to 'sender' assuming it terminates
		** with SPACE (or NULL, which is an error, though).
		*/
		for (++ch, i = 0; *ch && *ch != ' '; ++ch )
			if (i < sizeof(sender)-1) /* Leave room for NULL */
				sender[i++] = *ch;
		sender[i] = '\0';
		/*
		** Actually, only messages coming from servers can have
		** the prefix--prefix silently ignored, if coming from
		** a user client...
		**
		** ...sigh, the current release "v2.2PL1" generates also
		** null prefixes, at least to NOTIFY messages (e.g. it
		** puts "sptr->nickname" as prefix from server structures
		** where it's null--the following will handle this case
		** as "no prefix" at all --msa  (": NOTICE nick ...")
		*/
		if (sender[0] != '\0' && IsServer(cptr))
		    {
		      from = find_server(sender, (aClient *) NULL);
		      if (!from || matches(from->name, sender)) {
			from = find_client(sender, (aClient *)NULL);
			}

		      para[0] = sender;

		      /* Hmm! If the client corresponding to the
			 prefix is not found--what is the correct
			 action??? Now, I will ignore the message
			 (old IRC just let it through as if the
			 prefix just wasn't there...) --msa
			 */
		      if (from == NULL)
			{
			  debug(DEBUG_NOTICE,
				"Unknown prefix (%s) from (%s)",
				buffer, cptr->name);
			  return (-1);
			}
		      if (from->from != cptr)
			{
			  debug(DEBUG_FATAL,
				"Message (%s) coming from (%s)",
				buffer, cptr->name);
			  return (-1);
			}
		    }
		while (*ch == ' ')
			ch++;
	    }
	if (*ch == '\0')
	    {
		debug(DEBUG_NOTICE, "Empty message from host %s:%s",
		      cptr->name,from->name);
		return(-1);
	    }
	/*
	** Extract the command code from the packet
	*/
	ch2 = index(ch, ' '); /* ch2 -> End of the command code */
	len = (ch2) ? (ch2 - ch) : strlen(ch);
	if (len == 3 &&
	    isdigit(*ch) && isdigit(*(ch + 1)) && isdigit(*(ch + 2)))
	    {
		mptr = NULL;
		numeric = (*ch - '0') * 100 + (*(ch + 1) - '0') * 10
			+ (*(ch + 2) - '0');
		paramcount = MAXPARA;
	    }
	else
	    {
		for (; mptr->cmd; mptr++) 
			if (myncmp(mptr->cmd, ch, len) == 0 &&
			    strlen(mptr->cmd) == len)
				break;

		if (!mptr->cmd)
		    {
			/*
			** Note: Give error message *only* to recognized
			** persons. It's a nightmare situation to have
			** two programs sending "Unknown command"'s or
			** equivalent to each other at full blast....
			** If it has got to person state, it at least
			** seems to be well behaving. Perhaps this message
			** should never be generated, though...  --msa
			** Hm, when is the buffer empty -- if a command
			** code has been found ?? -Armin
			*/
			if (buffer[0] != '\0') {
			  if (IsPerson(from))
			    sendto_one(from,
				       ":%s %d %s %s :Unknown command",
				       me.name, ERR_UNKNOWNCOMMAND,
				       from->name, ch);
			  /*
                          ** This concerns only client ... --Armin
			  */
			  if (me.user)
			    debug(DEBUG_ERROR,
				  "*** Error: %s %s from server",
				  "Unknown command", ch);
			}
			return(-1);
		    }
		paramcount = mptr->parameters;
		if (mptr->flags & 1 && !IsServer(cptr) && !IsService(cptr) && !IsOper(cptr))
		  cptr->since += 1;  /* Allow only 1 msg per 2 seconds
				      * (on average) to prevent dumping.
				      * to keep the response rate up,
				      * bursts of up to 5 msgs are allowed--SRB
				      */
	    }
	/*
	** Must the following loop really be so devious? On
	** surface it splits the message to parameters from
	** blank spaces. But, if paramcount has been reached,
	** the rest of the message goes into this last parameter
	** (about same effect as ":" has...) Prime example is
	** the SERVER message (2 parameters). --msa
	*/

	/* Note initially true: ch2==NULL || *ch2 == ' ' !! */

	if (me.user)
		para[0] = sender;
	i = 0;
	if (ch2 != NULL)
	    {
		if (paramcount > MAXPARA)
			paramcount = MAXPARA;
		for (;;)
		    {
			while (*ch2 == ' ')
				/*
				** Never "FRANCE " again!! ;-) Clean
				** out *all* blanks.. --msa
				*/
				*ch2++ = '\0';
			if (*ch2 == '\0')
				break;
			if (*ch2 == ':')
			    {
				/*
				** The rest is single parameter--can
				** include blanks also.
				*/
				para[++i] = ch2 + 1;
				break;
			    }
			para[++i] = ch2;
			if (i >= paramcount)
				break;
			for (; *ch2 != ' ' && *ch2; ch2++);
		    }
	    }
	para[++i] = NULL;
	if (mptr == NULL)
		return (DoNumeric(numeric, cptr, from, i, para));
	else
	    {
		mptr->count++;
		if (IsRegisteredUser(cptr) &&
		    mptr->func != m_ping && mptr->func != m_pong)
		  from->user->last = time(NULL);
		return (*mptr->func)(cptr, from, i, para);
	    }
    }


char *getfield(newline)
char *newline;
    {
	static char *line = NULL;
	char *end, *field;
	
	if (newline)
		line = newline;
	if (line == NULL)
		return(NULL);

	field = line;
	if ((end = index(line,':')) == NULL)
	    {
		line = NULL;
		if ((end = index(field,'\n')) == NULL)
			end = field + strlen(field);
	    }
	else
		line = end + 1;
	*end = '\0';
	return(field);
    }
