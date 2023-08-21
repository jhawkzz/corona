
#include "corona_headers.h"

Message_Stack::~Message_Stack( )
{
   // remove all messages that are left over.
   Message * message = (Message *) Pop( );

   while( (int) message != STACK_EMPTY )
   {
      Remove_Message( message );

      message = (Message *) Pop( );
   }
}

void Message_Stack::Add_Message( char * user_param, LPARAM lParam, MESSAGE message )
{
   Message * p_message = new Message;

   // copy all the parameters
   p_message->lParam  = lParam;
   p_message->message = message;
   p_message->user_param[ 0 ] = 0;

   if ( user_param )
   {
      strcpy( p_message->user_param, user_param );
   }

   Push( (uint32) p_message );
}

void Message_Stack::Remove_Message( Message * p_message )
{
   delete p_message;
}
