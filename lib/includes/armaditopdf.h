/***

Copyright (C) 2015, 2016 Teclib'

This file is part of Armadito module PDF.

Armadito module PDF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Armadito module PDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Armadito module PDF.  If not, see <http://www.gnu.org/licenses/>.

***/



#ifndef _ARMADITO_PDF_H_
#define _ARMADITO_PDF_H_

#include "armaditopdf/structs.h"
#include "armaditopdf/errors.h"
#include "armaditopdf/log.h"
#include "armaditopdf/osdeps.h"
#include "armaditopdf/utils.h"
#include "armaditopdf/parsing.h"


#define a6o_pdf_ver "0.12.6"


// Tests Coefficients
#define EMPTY_PAGE_CONTENT 99
#define OBJECT_COLLISION 10
#define OBJECT_COLLISION_AND_BAD_XREF 60
#define BAD_TRAILER 40
#define BAD_XREF_OFFSET 30
#define BAD_OBJ_OFFSET 20
#define OBFUSCATED_OBJECT 50 
#define MULTIPLE_HEADERS 50
#define MALICIOUS_COMMENTS 50

#define ACTIVE_CONTENT 40
#define SHELLCODE 40
#define PATTERN_HIGH_REPETITION 40
#define DANGEROUS_KEYWORD_HIGH 90
#define DANGEROUS_KEYWORD_MEDIUM 40
#define DANGEROUS_KEYWORD_LOW 20
#define TIME_EXCEEDED 20

//#define LARGE_FILE_SIZE 1500000

/* Functions */
char * getVersion();
int analyzePDF(char * filename);
int analyzePDF_fd(int fd, char * filename);
int analyzePDF_ex(int fd, char * filename);
int calc_suspicious_coef(struct pdfDocument * pdf);
void pdf_print_report(struct pdfDocument * pdf);

// C API functions
int pdf_initialize();
struct pdfDocument * pdf_load_fd(int fd, char * filename, int * retcode);
struct pdfDocument * pdf_load_file(char * filename, int * retcode);
struct pdfDocument * pdf_load_data(char * data, unsigned int data_size,  int * retcode);
void pdf_unload(struct pdfDocument * pdf);
void pdf_finalize();


#endif
