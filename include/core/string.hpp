#pragma once

#include "core/types.hpp"

void *memcpy(void *dst, const void *src, size_t len);
void *memset(void *dst, int value, size_t num);
void swap(char &t1, char &t2);
void reverse(char str[], int length);
char *itoa(int32_t num, char *str, int base);
char *utoa(unsigned int num, char *str, int base);
size_t strlen(char* str);
int strncmp(const char* str1, const char* str2, size_t length);
int strcmp(char* str1, char* str2);
