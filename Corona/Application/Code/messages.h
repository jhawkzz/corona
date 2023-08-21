
#ifndef MESSAGES_H_
#define MESSAGES_H_

// messages
enum MESSAGE
{
   CM_NULL,
   
   CM_LBUTTONDOWN,
   CM_RBUTTONDOWN,
      
   CM_PAUSE_ENGINE,
   CM_RESET_ENGINE,
   
   CM_TOGGLE_ALWAYS_ON_TOP,
   CM_TOGGLE_REDRAW_BACKBUFFER,
   CM_RESOLUTION_CHANGED,
   
   CM_COUNT
};

typedef struct Message_Struct
{
   MESSAGE  message;
   char     user_param[ MAX_PATH ];
   LPARAM   lParam;
}
Message;

class Message_Stack : public Stack
{
   public:
           ~Message_Stack( );
      void Add_Message   ( char * user_param, LPARAM lParam, MESSAGE message );
      void Remove_Message( Message * message );
};

#endif
