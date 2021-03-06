/****************************************************************************
 * Multi-Automated Auction Code                                             *
 * Markanth : dlmud@dlmud.com                                               *
 * Devil's Lament : dlmud.com port 3778                                     * 
 * Web Page : http://www.dlmud.com                                          *
 *                                                                          *
 * Provides automated support for multiple auctioned items.                 *
 * Includes advanced number parser by                                       *
 * Erwin S. Andreasen, erwin@andreasen.org                                  *
 *                                                                          *
 * All I ask in return is that you give me credit on your mud somewhere     *
 * or email me if you use it.                                               *
 ***************************************************************************/


>>> DB.C <<<

1) add to boot_db just before weather stuff:

   auction_list = NULL;

>>> ACT_INFO.C <<<

1) Add to format_obj_to_char()

   if(IS_OBJ_STAT(obj, ITEM_AUCTIONED))
       strcat(buf, "{Y(Auctioned){x");

>>> ACT_COMM.C <<<

1) Add to do_quit():

      if(has_auction(ch))
      {
          send_to_char("The auctioneer will not let you leave while you have an auction running.\n\r", ch);
          return;
      }

2) rename old do_auction() to

    void do_auction_talk();

>>> MAGIC.C <<<

1) In spell_locate_object() change the begining for loop to look like this

   for (obj = object_list; obj != NULL; obj = obj->next)
   {
      if (!can_see_obj (ch, obj) || !is_name (target_name, obj->name) ||
      /* check for AUCTIONED item */
	  IS_OBJ_STAT (obj, ITEM_NOLOCATE | ITEM_AUCTIONED) ||
      ...
      ...

>>> UPDATE.C <<<

1) In update_hander():

        a) add at top:

           static int   pulse_auction;
        
        b) add after pulse music:

           if (--pulse_auction <= 0)
           {
              pulse_auction = PULSE_AUCTION;
              auction_update ();
           }

>>> RECYCLE.C <<<

1) add at bottem (wherever)

AUCTION_DATA *auction_free;

AUCTION_DATA *new_auction (void)
{
   static AUCTION_DATA auc_zero;
   AUCTION_DATA *auction;

   if (auction_free == NULL)
      auction = alloc_perm (sizeof (*auction));
   else
   {
      auction = auction_free;
      auction_free = auction_free->next;
   }

   *auction = auc_zero;
   VALIDATE (auction);
   return auction;
}

void free_auction (AUCTION_DATA * auction)
{
   if (!IS_VALID (auction))
      return;

   auction->high_bidder = NULL;
   auction->item = NULL;
   auction->bid = 0;
   auction->owner = NULL;
   auction->status = 0;
   auction->number = 0;
   INVALIDATE (auction);

   auction->next = auction_free;
   auction_free = auction;
}


>>> TABLES.C <<<

1) Add to extra_flags[]:

   {"auctioned", ITEM_AUCTIONED, FALSE},
   {"no_auction", ITEM_NO_AUCTION, TRUE},

>>> SAVE.C <<<

1) Add to fread_obj():

	    if (!new_format)
	    {
	       obj->next = object_list;
	       object_list = obj;
	       obj->pIndexData->count++;
	    }

+	    if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
+       {
+           bug("char loaded with auctioned item.", 0);
+	       REMOVE_BIT (obj->extra_flags, ITEM_AUCTIONED);
+       }

>>> HANDLER.C <<<

1) Add to obj_to_room()

   if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
      return;

2) Add to obj_to_obj()

   if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
      return;

3) Add to extract_char()

    a) change the first fPull statement to this:

   if (fPull)
   {
      if(has_auction(ch))
          return;

      die_follower (ch);
   }

    b) change the first obj for loop to this

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
	obj_next = obj->next_content;

    /* prevent losing auctioned items on death */
    if(!fPull && !IS_OBJ_STAT(obj, ITEM_AUCTIONED))
	extract_obj( obj );
    }

4) Add to can_drop()

   if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
      return FALSE;

>>> ACT_WIZ.C <<<

1) Add to do_reboot(), do_shutdown(), and do_copyover() if you have it

   while (auction_list != NULL)
      reset_auc (auction_list, TRUE);

>>> FIGHT.C <<<

1) in make_corpse() add to the ch->carrying for loop:

    /* auctioned items stay with character */
      if (IS_OBJ_STAT(obj, ITEM_AUCTIONED))
	 continue;

>>> INTERP.C <<<

1) add command entries:

   {"auction", do_auction, POS_SLEEPING, 2, LOG_NORMAL, 1},
   {"bid", do_bid, POS_SLEEPING, 2, LOG_NORMAL, 1},

>>> MERC.H <<<

1) define structure:

typedef struct auction_data AUCTION_DATA;

2) define new game pulse:

#define	PULSE_AUCTION             (30 * PULSE_PER_SECOND)

3) add:

#define	AUCTION_LENGTH  5

4) Add structure:

struct auction_data {
   AUCTION_DATA *next;
   OBJ_DATA    *item;
   CHAR_DATA   *owner;
   CHAR_DATA   *high_bidder;
   sh_int       status;
   sh_int       number;
   long         bid;
   bool         valid;
};

5) Add bits under item flags:

#define	ITEM_NO_AUCTION         (##)  /*whatevers free */
#define	ITEM_AUCTIONED          (##)

6) Add macro if you don't already have something simaler

#define GET_NAME(Ch)    (IS_NPC(Ch) ? (Ch)->short_descr : (Ch)->name)

7) Add external:

extern AUCTION_DATA *auction_list;

8) Add declarations:

void reset_auc args ((AUCTION_DATA * auc, bool forced));
int count_auc args ((CHAR_DATA * ch));
void auction_update args ((void));
/* advatoi and parsebet by Erwin Andreason THE snippet man */
long advatoi args ((const char *s));
long parsebet args ((const long currentbet, const char *argument));
AUCTION_DATA *auction_lookup args((sh_int num));
int get_auc_id args((void));
bool has_auction args((CHAR_DATA *ch));
void auction_channel args((CHAR_DATA *ch, char *message));
void do_auction_talk args((CHAR_DATA *ch, char *argument));


>>> RECYCLE.H <<<

1) Add declarations:

AUCTION_DATA *new_auction args ((void));
void free_auction args ((AUCTION_DATA * auction));

>>> INTERP.H <<<

1) Add command declarations:

DECLARE_DO_FUN (do_auction);
DECLARE_DO_FUN (do_bid);


>>> MAKEFILE <<<

1) Add to O_FILES:

        auction.o

>>> BEGIN AUCTION.C <<<

/****************************************************************************
 * Multi-Automated Auction Code                                             *
 * Markanth : dlmud@dlmud.com                                               *
 * Devil's Lament : dlmud.com port 3778                                     *
 * Web Page : http://www.dlmud.com                                          *
 *                                                                          *
 * Provides automated support for multiple auctioned items.                 *
 * Includes advanced number parser by                                       *
 * Erwin S. Andreasen, erwin@andreasen.org                                  *
 *                                                                          *
 * All I ask in return is that you give me credit on your mud somewhere     *
 * or email me if you use it.                                               *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "magic.h"

AUCTION_DATA *auction_list;

AUCTION_DATA *auction_lookup(sh_int num)
{
    AUCTION_DATA *pAuc;

    for(pAuc = auction_list; pAuc != NULL; pAuc = pAuc->next)
    {
        if(pAuc->number == num)
            return pAuc;
    }
    return NULL;
}

int last_auc_id;

int get_auc_id (void)
{
    last_auc_id++;

   if( last_auc_id > 999)
       last_auc_id = 1;

   return last_auc_id;
}

void do_auction(CHAR_DATA *ch, char *argument)
{
   AUCTION_DATA *auc;
   AUCTION_DATA *p;
   OBJ_DATA    *obj = NULL;
   long         minbid = 1;
   char         arg1[MAX_INPUT_LENGTH];
   char         arg2[MAX_INPUT_LENGTH];
   char         buf[MAX_STRING_LENGTH];

   argument = one_argument (argument, arg1);
   argument = one_argument (argument, arg2);

   if (ch == NULL)
      return;

   if(arg1[0] == '\0')
    {
       if(IS_SET(ch->comm, COMM_NOAUCTION))
        {
           REMOVE_BIT(ch->comm, COMM_NOAUCTION);
           send_to_char("AUCTION channel is now ON.\n\r", ch);
        }
        else
        {
            SET_BIT(ch->comm, COMM_NOAUCTION);
            send_to_char("AUCTION channel is now OFF.\n\r", ch);
        }
        return;
    }
  else if (!str_cmp (arg1, "talk"))
   {

       if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOAUCTION))
       {
          send_to_char ("Turn on the auction info channel first.\n\r",ch);
          return;
       }
       /* nice hack here */
      strcat(arg2, " ");
      strcat(arg2, argument);

       do_auction_talk(ch, argument);
      return;
   }

   else if (!str_cmp (arg1, "stop") && IS_IMMORTAL (ch))
   {

        if(arg2[0] == '\0' || !is_number(arg2))
       {
            send_to_char("Stop which auction?\n\r", ch);
            return;
       }
        if((auc = auction_lookup(atoi(arg2))) == NULL)
       {
            send_to_char("No such auction.\n\r", ch);
            return;
       }
       sprintf(buf, "$n has stopped the auction and confiscated %s!", auc->item->short_descr);
      auction_channel (ch, buf);
      sprintf(buf, "AUCTION: You stop the auction and confiscate %s.\n\r", auc->item->short_descr);
      send_to_char(buf, ch);
      obj_from_char(auc->item);
      obj_to_char(auc->item, ch);
      reset_auc (auc, TRUE);
      return;
   }

   if ((obj = get_obj_carry (ch, arg1, ch)) == NULL)
   {
      send_to_char ("You aren't carrying that item.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT (obj, ITEM_AUCTIONED))
   {
      send_to_char ("That items is already being auctioned.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT (obj, ITEM_NODROP))
   {
      send_to_char ("You can't let go of that item.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT (obj, ITEM_NO_AUCTION))
   {
      send_to_char ("That item cannot be auctioned.\n\r", ch);
      return;
   }

   if (obj->item_type == ITEM_CORPSE_PC
       || obj->item_type == ITEM_CORPSE_NPC)
   {
      send_to_char ("Not a good idea....\n\r", ch);
      return;
   }

   if (count_auc (ch) >= 3)
   {
      send_to_char ("You are only allowed to auction 3 items a time!\n\r", ch);
      return;
   }

   if (arg2[0] != '\0')
      minbid = atol (arg2);

   if (minbid > 500000)
   {
      send_to_char ("Minumum bids can't be higher than 500000.\n\r", ch);
      return;
   }

   if (auction_list == NULL)
   {
      auc = new_auction ();
      auction_list = auc;
      auction_list->next = NULL;
   }
   else
   {
      auc = new_auction ();

      for (p = auction_list; p; p = p->next)
      {
	 if (p->next == NULL)
	    p->next = auc;
	 auc->next = NULL;
      }
   }

   SET_BIT (obj->extra_flags, ITEM_AUCTIONED);
   auc->owner = ch;
   auc->item = obj;
   auc->bid = minbid;
   auc->number = get_auc_id();
   auc->status = 0;

	sprintf(buf, "$n is auctioning %s (Level %d, Num %d). Current bid is %ld.",
     auc->item->short_descr, auc->item->level, auc->number, auc->bid);
    auction_channel(ch, buf);
	sprintf(buf, "AUCTION: You are auctioning %s (Level %d, Num %d). Current bid is %ld.",
     auc->item->short_descr, auc->item->level, auc->number, auc->bid);
    send_to_char(buf, ch);
   return;
}

void auction_update (void)
{
   AUCTION_DATA *auc;
   char buf[MAX_STRING_LENGTH];

   for (auc = auction_list; auc != NULL; auc = auc->next)
   {
      auc->status++;

     if(!auc->item)
     {
         reset_auc(auc, TRUE);
     }
	 else if (auc->status == AUCTION_LENGTH)
	 {
          if (auc->high_bidder == NULL)
          {
                reset_auc (auc, TRUE);
          }
          else if (auc->high_bidder->gold < auc->bid)
          {
               send_to_char("You can't cover your bid in the auction anymore!\n\r", auc->high_bidder);
               sprintf(buf, "%s can't cover the bid anymore, sale stopped.\n\r", GET_NAME( auc->high_bidder));
               send_to_char(buf, auc->owner);
               reset_auc (auc, TRUE);
          }
          else
          {
               sprintf(buf, "%s SOLD to %s for %ld gold.", auc->item->short_descr,
                 GET_NAME (auc->high_bidder), auc->bid);
               auction_channel(NULL, buf);
               reset_auc (auc, FALSE);
          }
	 }
   }
   return;
}

void reset_auc (AUCTION_DATA * auc, bool forced)
{
    char buf[MAX_STRING_LENGTH];

   if (!IS_VALID (auc))
      return;

   if (auc->item != NULL)
   {
      if (IS_OBJ_STAT (auc->item, ITEM_AUCTIONED))
         REMOVE_BIT (auc->item->extra_flags, ITEM_AUCTIONED);
      else
         bug ("Reset_auction: item not flagged auction item", 0);

      if (!forced && auc->high_bidder != NULL && auc->bid > 0)
      {
         auc->owner->gold += (auc->bid * 9) / 10;
         auc->high_bidder->gold -= auc->bid;

         sprintf (buf, "You recieve %ld gold for the sale of %s.\n\r", (auc->bid * 9) / 10, auc->item->short_descr);
         send_to_char(buf, auc->owner);
         obj_from_char (auc->item);
         obj_to_char (auc->item, auc->high_bidder);

         sprintf (buf, "You are sold %s for %ld gold.\n\r", auc->item->short_descr, auc->bid);
         send_to_char(buf, auc->high_bidder);
      }
      else if (auc->owner != NULL)
      {
         sprintf (buf, "Sale of %s stopped.\n\r", auc->item->short_descr);
         send_to_char(buf, auc->owner);
      }
   }
   auc->bid = 0;
   auc->high_bidder = NULL;
   auc->item = NULL;
   auc->owner = NULL;
   auc->status = 0;
   auc->number = 0;

   if (auc == auction_list)
   {
      if (auc->next != NULL)
         auction_list = auc->next;
      else
         auction_list = NULL;

      free_auction (auc);
      return;
   }

   free_auction (auc);
   return;
}

int count_auc (CHAR_DATA * ch)
{
   AUCTION_DATA *q;
   int          count;

   q = auction_list;

   if (!q)
      return 0;

   for (count = 0; q; q = q->next)
   {
      if (q->owner == ch)
	 count++;
   }

   return count;
}

long advatoi (const char *s)
{
   char         string[MAX_INPUT_LENGTH];
   char        *stringptr = string;
   char         tempstring[2];
   long         number = 0;
   long         multiplier = 0;

   strcpy (string, s);

   while (isdigit (*stringptr))
   {
      strncpy (tempstring, stringptr, 1);
      number = (number * 10) + atol (tempstring);
      stringptr++;
   }

   switch (UPPER (*stringptr))
   {
   case 'K':
      multiplier = 1000;
      number *= multiplier;
      stringptr++;
      break;
   case 'M':
      multiplier = 1000000;
      number *= multiplier;
      stringptr++;
      break;
   case '\0':
      break;
   default:
      return 0;

   }

   while (isdigit (*stringptr) && (multiplier > 1))
   {
      strncpy (tempstring, stringptr, 1);

      multiplier = multiplier / 10;
      number = number + (atol (tempstring) * multiplier);
      stringptr++;
   }

   if (*stringptr != '\0' && !isdigit (*stringptr))

      return 0;

   return (number);
}

long parsebet (const long currentbet, const char *argument)
{
   long         newbet = 0;
   char         string[MAX_INPUT_LENGTH];
   char        *stringptr = string;

   strcpy (string, argument);

   if (*stringptr)
   {
      if (isdigit (*stringptr))
	 newbet = advatoi (stringptr);
      else if (*stringptr == '+')
      {
	 if (strlen (stringptr) == 1)
	    newbet = (currentbet * 125) / 100;
	 else
	    newbet = (currentbet * (100 + atoi (++stringptr))) / 100;
      }
      else
      {
	 printf ("considering: * x \n\r");

	 if ((*stringptr == '*') || (*stringptr == 'x'))
	 {

	    if (strlen (stringptr) == 1)
	       newbet = currentbet * 2;
	    else
	       newbet = currentbet * atoi (++stringptr);
	 }
      }
   }

   return newbet;

}


void do_bid(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AUCTION_DATA *auc;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

      if (auction_list == NULL)
      {
	 send_to_char ("There's nothing up for auction right now.\n\r", ch);
	 return;
      }

    if(arg1[0] == '\0')
    {
      send_to_char("Num   Seller       Item Description                    Lvl    Last Bid   Time\n\r", ch);
      send_to_char("---   ------------ ----------------------------------- --- ------------- ----\n\r", ch);
      for (auc = auction_list; auc; auc = auc->next)
      {
	    if (!IS_OBJ_STAT (auc->item, ITEM_AUCTIONED))
	       bug ("Auctioned item is not flaged Auctioned.", 0);

        sprintf(buf, "%3d - %-12s %35s %3d %13ld %4d\n\r", auc->number, GET_NAME(auc->owner), auc->item->short_descr, auc->item->level, auc->bid, auc->status);
        send_to_char(buf, ch);
      }
      send_to_char("------------------------------------------------------------------------------\n\r", ch);
        send_to_char("Type: 'Bid <num>' to see stats and 'Bid <num> <amount>' to bid on an item.\n\r", ch);
      return;
    }
   else if ((auc = auction_lookup(atoi(arg1))) != NULL)
   {
        if(arg2[0] == '\0')
        {
          if (ch == auc->owner && !IS_IMMORTAL (ch))
          {
         sprintf(buf, "You're auctioning %s.\n\r", auc->item->short_descr);
         sprintf(buf, "Current bid is %ld gold.\n\r", auc->bid);
         return;
          }
          spell_identify (0, ch->level, ch, auc->item, TAR_OBJ_INV);
         sprintf(buf, "Current bid is %ld gold.\n\r", auc->bid);
         send_to_char(buf, ch);
          return;
       }
       else
       {
          long         bid = 0;

          if (ch == auc->high_bidder)
          {
         send_to_char ("You already have the highest bid!\n\r", ch);
         return;
          }

          if (ch == auc->owner)
          {
         send_to_char ("You cannot bid on your own items!\n\r", ch);
         return;
          }

          bid = parsebet (auction_list->bid, arg2);

          if (ch->gold < bid)
          {
                send_to_char ("You can't cover that bid.\n\r", ch);
                return;
          }

          if (bid < auc->bid)
          {
         sprintf(buf, "The minimum bid is %ld gold.\n\r", auc->bid);
         send_to_char(buf, ch);
         return;
          }

          if (bid < (auc->bid + 10))
          {
                sprintf(buf, "You must outbid %ld gold by at least 10.", auc->bid);
                send_to_char(buf, ch);
                return;
          }

          sprintf(buf, "%ld gold has been offered for %s.",
            bid, auc->item->short_descr);
          auction_channel (NULL, buf);

          auc->high_bidder = ch;
          auc->bid = bid;
          auc->status = 0;
          return;
       }
   }
}

bool has_auction(CHAR_DATA *ch)
{
   AUCTION_DATA *auc;

   for(auc = auction_list; auc != NULL; auc = auc->next)
    {
       if(auc->owner == ch || auc->high_bidder == ch)
           return TRUE;
    }

    return FALSE;
}

void auction_channel(CHAR_DATA *ch, char *message)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *dch;
    char buf[MAX_INPUT_LENGTH];

    for(d = descriptor_list; d != NULL; d = d->next)
    {
        if(d->connected != CON_PLAYING)
            continue;

        if((dch = d->character) == NULL || dch == ch)
            continue;

        sprintf(buf, "AUCTION: %s", message);

        if(ch) /* don't use $N only $n in message */
            act_new(buf, ch, NULL, dch, TO_VICT, POS_DEAD);
        else
            send_to_char(buf, dch);
    }
}


>>>> END AUCTION.C <<<

Markanth : dlmud@dlmud.com     
Devil's Lament : dlmud.com port 3778     
Web Page : http://dlmud.akbearsden.com
