// Filename: dictionary.c
// Descriptions: Together with interface dictionary.h, this dictionary.c
//               provides a library (general purpose) for dictionary-like data structure
//               based on a hash table.
//               Each elements will be assigned with a key, and stored in DNODE. These functions
//               provide a solution for adding/removing/searching elements inside a dictionary
//               with optimized speed.

/* Change to NOT free the data stored in each DNODE.  That is mamanged by ZombieLisp */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "header.h"
#include "hash.h"
#include "dictionary.h"

//! \brief Generate the hash slot number for each string.
int make_hash(char* c) {
  return (hash1(c)%(MAX_HASH_SLOT - 1));
}

/*pseudocode*/
// 1 malloc a DICTIONARY to d
// 2 initialize start end to null
// 3 initialize each hash slot to null
dictionary_t* new_dictionary() {
  dictionary_t* d = (dictionary_t*)malloc(sizeof(dictionary_t));
  MALLOC_CHECK(d);
  d->start = d->end = NULL;
  int i;
  for (i = 0; i < MAX_HASH_SLOT; i++) {
    d->hash[i] = NULL;
  }
  return d;
}

/*pseudocode*/
// 1 while (go through each DNODE)
// 2  free the data of DNODE
// 3  free this DNODE
// 4 end while
void clean_dictionary(dictionary_t* dict) {
  DNODE* d = dict->start;
  while (d != NULL) {
    DNODE* dd = d;
    /* if (dd->data != NULL) */
    /*   free(dd->data); */
    d = d->next;
    free(dd);
  }
}

/*pseudocode*/
// 1 h = hash value of the key
// 2 if (dict->start is null, implies dict is empty)
// 3   add the new DNODE to the dict,
// 4   updating start and end pointer
// 5 else (there is already some DNODE in dict)
// 6   if (the h slot is occupied)
// 7     for each DNODE with hash value h
// 8        find is there a DNODE with the same key
// 9        if find, do nothing
// 10    end
// 11    if (not find a DNODE with the same key)
// 12      add the DNODE after the last DNODE with hash value h
// 13  else (the h slot is not occupied)
// 14    add the new DNODE to the end, update end
// 15    update hashtable
// 16 Done
void dictionary_put(dictionary_t* dict, char* key, void* data) {
  MYASSERT(strlen(key) > 0);
  int h = make_hash(key);
  if (dict->start == NULL) {
    //! The first element of our dictionary
    //! I assume \a dict->end is also NULL
    MYASSERT(dict->end == NULL);
    dict->start = dict->end = (DNODE*)malloc(sizeof(DNODE));
    MALLOC_CHECK(dict->start);
    dict->start->prev = dict->start->next = NULL; 
    dict->hash[h] = dict->start;
    dict->start->data = data;
    BZERO(dict->start->key, KEY_LENGTH);
    strncpy(dict->start->key, key, KEY_LENGTH);
  } else {
    DNODE* d;
    //! Not the first element.
    if (dict->hash[h]) {
      //! Same hash.
      int flag = 0; //!< zero the key is not in our dictionary, one if does
      //! First, we want to see is this key already in our dictionary
      for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
           d = d->next) {
        if (!strncmp(d->key, key, KEY_LENGTH)) {
          //! We find the same key.
          /* if (d->data != NULL) */
          /*   free(d->data); */
          d->data = data;
          flag = 1;
          break;
        }        
      }
      //! not in.
      if (flag == 0) {
        //! We didin't find the same key.
        if (d == NULL)
          d = dict->end;
        else
          d = d->prev;
        DNODE* dd = (DNODE*)malloc(sizeof(DNODE));
        MALLOC_CHECK(dd);
        dd->next = d->next;
        dd->prev = d;
        if (dd->next == NULL)
          dict->end = dd;
        d->next = dd;
        BZERO(dd->key, KEY_LENGTH);
        strncpy(dd->key, key, KEY_LENGTH);
        dd->data = data;
      }
     } else {
      //! No same hash, create at the end of the list.
      DNODE* d = (DNODE*)malloc(sizeof(DNODE));
      d->next = NULL;
      d->prev = dict->end;
      dict->end->next = d;
      //! add to our cash slot
      dict->hash[h] = d;
      d->data = data;
      BZERO(d->key, KEY_LENGTH);
      strncpy(d->key, key, KEY_LENGTH);
      //! change the end of our list
      dict->end = d;
    }
  }
}


/*Pseudocode*/
// 1 
void dictionary_remove(dictionary_t* dict, char* key) {
  DNODE* d;  
  int h = make_hash(key);
  if (dict->hash[h] == NULL)
    return;
  for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
       d = d->next) {
    //! OK, we find our node.
    if (!strncmp(d->key, key, KEY_LENGTH)) {
      //! is it the last node.
      if (d->next == NULL) 
        dict->end = d->prev;
      else
         d->next->prev = d->prev;
      //! or is it the first node.
      if (d->prev == NULL)
        dict->start = d->next;
      else
        d->prev->next = d->next;
      //! update hash slot, 
      if (dict->hash[h] == d) {
        if ((d->next != NULL)&&(make_hash(d->next->key)==h)) {
          dict->hash[h] = d->next;
        } else {
          dict->hash[h] = NULL;
        }
      }
      /* if (d->data != NULL) */
      /*   free(d->data); */
      free(d);
    }
  }
}

/*Pseudocode*/
// 1 get the hash value of key, save to h
// 2 get the pointer of hash slot h
// 3   for (each DNODE d with same hash value)
// 4     if (d is the same key)
// 5       return data of d
// 6   end
// 7 return null
void* dictionary_get(dictionary_t* dict, char* key) {
  DNODE* d;  
  int h = make_hash(key);
  //! This speed up the process.
  if (dict->hash[h] == NULL)
    return NULL;
  //! ok, we have the hash, so we find the actual key.
  for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
       d = d->next) 
    if (!strncmp(d->key, key, KEY_LENGTH)) 
      return d->data;
  return NULL;
}

/*Pseudocode*/
// 1 get the hash value of key, save to h
// 2 get the pointer of hash slot h
// 3   for (each DNODE d with same hash value)
// 4     if (d is the same key)
// 5       return d
// 6   end
// 7 return null
DNODE* GetDNODEWithKey(dictionary_t* dict, char* key) {
  DNODE* d;  
  int h = make_hash(key);
  //! This speed up the process.
  if (dict->hash[h] == NULL)
    return NULL;
  //! ok, we have the hash, so we find the actual key.
  for (d = dict->hash[h]; (d!=NULL) && (make_hash(d->key) == h);
       d = d->next) 
    if (!strncmp(d->key, key, KEY_LENGTH)) 
      return d;
  return NULL;
}

void with_each_value_do(dictionary_t* dict, processing_function_t func)
{
  for (int i = 0; i < MAX_HASH_SLOT; i++) {
    for (DNODE *d = dict->hash[i]; (d != NULL); d = d->next) {
      func(d->data);
    }
  }
 
}
