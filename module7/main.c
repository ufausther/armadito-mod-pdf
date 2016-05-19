/*  
	< ARMADITO PDF ANALYZER is a tool to parses and analyze PDF files in order to detect potentially dangerous contents.>
    Copyright (C) 2015 by Ulrich FAUSTHER <ufausther@teclib.com>
    

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
    
#include "pdfAnalyzer.h"


void Helper(){
	
	printf("ARMADITO PDF ANALYZER :: No file in parameter\n");
	printf("Command : ./pdfAnalyzer [filename]\n\n");

	return;
}

void Banner(){
	
	printf("-------------------------\n");
	printf("-- ARMADITO PDF ANALYZER  --\n");
	printf("-------------------------\n\n");

	return;
}


int main (int argc, char ** argv){

	int ret;
	//FILE * f = NULL;
	//int fd = -1;
	
	#ifdef DEBUG
	Banner();
	#endif

	if(argc < 2){
		Helper();
		return (-1);
	}
	
#if 0

	// Code for testing analysis with file descriptor.
	//printf ("Analyzing file : %s\n",argv[1]);
	if(!(f = os_fopen(argv[1],"rb"))){
		printf("[-] Error :: main :: Error while opening file %s\n",argv[1]);
		return -1;
	}
	
	fd = os_fileno(f);
	//printf("[+] Debug :: fd = %d\n",fd);
	ret = analyzePDF_fd(fd, argv[1]);

	fclose(f);

#else

	ret = analyzePDF(argv[1]);
	
#endif	

	//system("pause");
	
	return ret;
}