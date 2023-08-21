
#include "corona_headers.h"

Stack::Stack( void )
{
   m_head_node        = NULL;
   m_top_node         = NULL;
   m_chunks_allocated = 0;
}

Stack::~Stack( )
{
   Release_Everything( );
}

void Stack::Initialize( void )
{
   Release_Everything( );
}

void Stack::Release_Everything( void )
{
   Stack_Node ** pp_node_list = (Stack_Node **) new Stack_Node *[ m_chunks_allocated ];

   // find all the nodes that were used to allocate
   Stack_Node * p_node = m_head_node;
   uint32 index              = 0;

   while( p_node )
   {
      if ( p_node->start_of_allocation )
      {
         pp_node_list[ index++ ] = p_node;
      }
      
      p_node = p_node->next;
   }

   // delete all the flagged pointers
   uint32 i;
   for ( i = 0; i < m_chunks_allocated; i++ )
   {
      delete [] pp_node_list[ i ];
   }

   // delete the pointer list
   delete [] pp_node_list;

   m_head_node        = NULL;
   m_top_node         = NULL;
   m_chunks_allocated = 0;
}

void Stack::Reset( void )
{
   m_top_node = NULL;
}

void Stack::Push( uint32 data )
{
   //no nodes have been allocated
   if ( !m_head_node )
   {
      m_head_node = Allocate_Nodes( );
      m_top_node  = m_head_node;
   }
   else if ( !m_top_node )
   {
      // if top node is null they popped everything including the head
      m_top_node = m_head_node;
   }
   else
   {
      // just append it to the end, but allocate nodes if needed
      if ( !m_top_node->next )
      {
         m_top_node->next       = Allocate_Nodes( );
         m_top_node->next->prev = m_top_node;   
      }

      m_top_node = m_top_node->next;
   }

   m_top_node->data = data;
}

uint32 Stack::Pop( void )
{
   uint32 stack_data = STACK_EMPTY;

   if ( m_top_node )
   {
      // we need to move the top node back
      stack_data = m_top_node->data;
      m_top_node = m_top_node->prev;
   }

   return stack_data;
}

uint32 Stack::Peek( void )
{
   return m_top_node ? m_top_node->data : STACK_EMPTY;
}

Stack_Node * Stack::Allocate_Nodes( void )
{
   // allocate a new chunk of nodes
   Stack_Node * stack_chunk = new Stack_Node[ NODE_CHUNK_SIZE ];

   memset( stack_chunk, 0, sizeof( Stack_Node ) * NODE_CHUNK_SIZE );

   stack_chunk->start_of_allocation = TRUE;

   // connect all the pointers
   Stack_Node * last_node = NULL;

   uint32 i;
   for ( i = 0; i < NODE_CHUNK_SIZE - 1; i++ )
   {
      stack_chunk[ i ].next = &stack_chunk[ i + 1 ];
      stack_chunk[ i ].prev = last_node;

      last_node = &stack_chunk[ i ];
   }

   stack_chunk[ i ].prev = last_node;

   m_chunks_allocated++;

   return stack_chunk;
}
