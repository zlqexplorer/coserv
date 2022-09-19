#ifndef COSERV_COROUTINE_COTHREAD_H
#define COSERV_COROUTINE_COTHREAD_H


typedef void (*threadFunc)(void*);

int cothread(threadFunc func,void* arg);