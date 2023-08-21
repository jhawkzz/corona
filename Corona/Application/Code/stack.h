
#ifndef STACK_H_
#define STACK_H_

#define STACK_EMPTY     (0xFFFFFFFF)
#define NODE_CHUNK_SIZE (5)

typedef struct Stack_Node_Struct
{
   uint32 data;
   Stack_Node_Struct * prev;
   Stack_Node_Struct * next;
   
   BOOL start_of_allocation;
}
Stack_Node;

class Stack
{

   public:

                          Stack             ( void );
                          ~Stack            ( );

             void         Initialize        ( void );
             void         Release_Everything( void );

             void         Reset             ( void );

             void         Push              ( uint32 data );
             uint32       Pop               ( void );
             uint32       Peek              ( void );

   private:
            Stack_Node *  Allocate_Nodes    ( void );

            Stack_Node *  m_head_node;
            Stack_Node *  m_top_node;

            uint32        m_chunks_allocated;
};

#endif
