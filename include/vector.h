/*
Modefied by MobiRuby developers.

Original is
libds - Simple, Memory-Safe Data Structures in C
========================================================================

This is a collection of C implementations of common data structures that
I use in many of my C projects. It includes the following procedures and 
data structures.

Hashmap
Vector (auto-expanding array)
Linked List
String Buffer (auto-expanding, length-recording string)

Installation Instructions:

Running make with no arguments will generate a shared library named libds.a.
You can then copy that library, as well as all header files, into your 
project directory. If you do not wish to use separate header files, you may 
generate a single header file called ds.h by running make ds.h.

You can link your applications to the static library like so. 

gcc -static main.c -L. -lds -o main

Assuming that main.c is your program and libds.a has been copied into the 
same directory.

Documentation on how to use the various functions and datastructures can be 
found in the header files.

License:

libds is distributed under the MIT License, the full text of which is 
reproduced in LICENSE.

Copyright (c) 2011 Zhehao Mao

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __LIBDS_VECTOR_H__
#define __LIBDS_VECTOR_H__

/* A C implementation of a vector, or dynamically expanding array. */

#include <stdlib.h>

#define BASE_CAP 10
#define EXPAND_RATIO 1.5

struct vector {
	void** data;
	size_t length;
	size_t capacity;
	void (*destructor)(void*);
};

typedef struct vector * vector_p;

/* Create a vector object. It must be eventually destroyed by a call to 
   destroy_vector to avoid memory leaks. */
vector_p create_vector();
/* Create a new vector that is composed of the items in the old vector with
   indices in the range of [start,end) */
vector_p subvector(vector_p vec, int start, int end);
/* Add an item to the end of the vector */
void vector_add(vector_p vec, void* data);
/* Get the item at index i of the vector */
void* vector_get(vector_p vec, size_t i);
/* Set the item at index i of the vector to the data provided. */
int vector_set(vector_p vec, size_t i, void* data);
/* Insert the data at index i of the vector and shift the other 
   items to the right. */
int vector_insert(vector_p vec, size_t i, void* data);
/* Get the index of the item in the vector that is equal to the data. 
   Equality is defined as having the same bytes in memory. */
int vector_index(vector_p vec, void* data);
/* Remove the item at index i of the vector and free its memory */
void vector_remove(vector_p vec, size_t i);
/* Check to make sure there is still room in the vector and expand it if 
   necessary. This function is not meant to be called directly. */
void check_length(vector_p vec);
/* Destroy the vector and free all the memory associated with it. */
void destroy_vector(vector_p vec);
/* Swaps the pointers at indices i and j in the vector */
void vector_swap(vector_p vec, size_t i, size_t j);


void vector_enqueue(vector_p vec, void* data);
void* vector_dequeue(vector_p vec);

#endif
