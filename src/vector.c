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

#include "vector.h"
#include <string.h>

vector_p create_vector(){
	vector_p vec = (vector_p)malloc(sizeof(struct vector));
	vec->data = (void**)malloc(sizeof(void*)*BASE_CAP);
	vec->capacity = BASE_CAP;
	vec->length = 0;
	return vec;
}

vector_p subvector(vector_p vec, int start, int end){
	vector_p subvec = create_vector();
	int i;
	for(i=start; i<end; i++){
		vector_add(subvec, vec->data[i]);
	}
	return subvec;
}

void check_length(vector_p vec){
	if(vec->length >= vec->capacity){
		vec->capacity*=EXPAND_RATIO;
		vec->data = (void**)realloc((void*)vec->data, 
									vec->capacity*sizeof(void*));
		//vec->sizes = (int*)realloc((void*)vec->sizes, 
		//							vec->capacity*sizeof(int));
	}
}

void vector_add(vector_p vec, void* data){
	check_length(vec);
	vec->data[vec->length] = data;
	vec->length++;
}

void* vector_get(vector_p vec, size_t i){
	if(i >= vec->length)
		return NULL;
	return vec->data[i];
}

int vector_set(vector_p vec, size_t i, void* data){
	if(i >= vec->length)
		return -1;
	vec->data[i] = data;
	return 0;
}

int vector_insert(vector_p vec, size_t i, void* data){
	int x;
	
	if(i > vec->length)
		return -1;
	
	check_length(vec);
	for(x=vec->length;x>=i;x--){
		vec->data[x+1] = vec->data[x];
	}
	vec->data[i] = data;
	vec->length++;
	return 0;
}

void vector_remove(vector_p vec, size_t i){
	int x;
	if(i >= vec->length)
		return;
	vec->length--;
	for(x=i;x<vec->length;++x){
		vec->data[x] = vec->data[x+1];
	}
}

int vector_index(vector_p vec, void* data){
	int x;
	for(x=0;x<vec->length;++x){
		if(vec->data[x] == data){
			return x;
		}
	}
	return -1;
}

void destroy_vector(vector_p vec){
	free(vec->data);
	free(vec);
}


void vector_swap(vector_p vec, int i, int j){
	void * temp;
	
	if(i >= vec->length || j >= vec->length)
		return;
		
	temp = vec->data[i];
	vec->data[i] = vec->data[j];
	vec->data[j] = temp;	
}


void vector_enqueue(vector_p vec, void* data)
{
	vector_add(vec, data);
}


void* vector_dequeue(vector_p vec)
{
	void * data = vector_get(vec, 0);
	if(data) {
		vector_remove(vec, 0);
	}
	return data;
}
