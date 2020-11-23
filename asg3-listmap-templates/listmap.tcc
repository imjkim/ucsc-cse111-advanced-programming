// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t,mapped_t,less_t>::~listmap() {
   if(empty()){
      iterator i = begin();
      while(i != end()){
         erase(i);
         ++i;
      }
   }
   DEBUGF ('l', reinterpret_cast<const void*> (this));
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::insert (const value_type& pair) {
   DEBUGF ('l', &pair << "->" << pair);
   if(empty()){
      node* n = new node(anchor(), anchor(), pair);
      n->next = n->prev = anchor();
      return iterator(n);
   }
   node* temp = anchor();
   for(auto i = begin(); i != end(); ++i){
      if(not less(i->first, pair.first) && not less(pair.first, i->first)){
         i->second = pair.second;
         return iterator();
	  } else if(less (pair.first, i->first)){
         node* n = new node(temp, temp->next, pair);
         temp->next = n;
         return iterator(n);
	  }
      temp = temp->next;
   }
   node* n = new node(temp, temp->next, pair);
   temp->next = n;
   return iterator(n);
}
 
//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::find (const key_type& that) {
   DEBUGF ('l', that);
   for(auto i = begin(); i!= end(); ++i){
      if(not less(i->first, that) && not less(that, i->first)){
         return i;
	  }
   }
   return end();
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::erase (iterator position) {
   DEBUGF ('l', &*position);
   node* temp = anchor();
   for(auto i = begin(); i != end(); ++i){
      temp = temp->next;
      if(i == position){
         node* next = temp->next;
         temp->prev->next = next;
         next->prev = temp->prev;
         delete temp;
		 return iterator(next);
      }
   }
   return iterator();
}
//
// iterator listmap::printAll ()
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t,mapped_t,less_t>::iterator
listmap<key_t,mapped_t,less_t>::printAll () {
   for(auto i = begin(); i!= end(); ++i){
      cout << i->first << " = " << i->second << endl;
   }
   return end();
}