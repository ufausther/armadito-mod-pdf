/*  
	<ARMADITO PDF ANALYZER is a tool to parse and analyze PDF files in order to detect potentially dangerous contents.>
    Copyright (C) 2015 by Teclib' 
	<ufausther@teclib.com>
    
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





// spot Javascript :: returns 1 if found and 0 if not.
int getJavaScript(struct pdfDocument * pdf, struct pdfObject* obj){

	char * js = NULL;
	char * js_obj_ref = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;
	struct pdfObject * js_obj = NULL;


	if( obj->dico == NULL){
		//printf("No dictionary in object %s\n",obj->reference);
		return -1;
	}

	start = searchPattern(obj->dico, "/JS" , 3 ,  strlen(obj->dico));

	if(start == NULL){
		//printf("No Javascript detected in object %s\n",obj->reference);
		return 0;
	}

	//printf("JavaScript Entry in dictionary detected in object %s\n", obj->reference);
	//printf("dictionary = %s\n", obj->dico);


	start += 3;

	// If there is a space
	//printf("start[0] = %c\n",start[0]);
	if(start[0] == ' '){
		start ++;
	}


	//printf("start[0] = %c\n",start[0]);

	len = strlen(obj->dico) - (int)(start - obj->dico);
	//printf("len = %d\n",len);

	// get an indirect reference
	js_obj_ref = getIndirectRef(start,len);


	//printf("js_obj_ref = %s\n", js_obj_ref);

	// Get javascript
	if(js_obj_ref != NULL){


		js_obj = getPDFObjectByRef(pdf,js_obj_ref);

		if(js_obj == NULL){
			#ifdef DEBUG
				printf("Error :: getJavaScript :: Object %s not found\n",js_obj_ref);
			#endif
			return -1;
		}


		// Decode object stream
		if(js_obj->filters != NULL){		
			decodeObjectStream(js_obj);			
		}


		if( js_obj->decoded_stream != NULL){
			js = js_obj->decoded_stream;
		}else{
			js = js_obj->stream;
		}


		if(js != NULL){
			#ifdef DEBUG
				printf("Found JS content in object %s\n",js_obj_ref);
			#endif
			//printf("Javascript content = %s\n",js);	
			// TODO Launch analysis on content

			//printf("js = %s\n",js);

			
			pdf->testObjAnalysis->active_content ++;
			pdf->testObjAnalysis->js ++;
			unknownPatternRepetition(js, strlen(js),pdf,js_obj);
			findDangerousKeywords(js,pdf,js_obj);
			

		}else{
			#ifdef DEBUG
				printf("Debug :: getJavaScript :: Empty js content in object %s\n",js_obj_ref);
			#endif
		}

		free(js_obj_ref);

	}else{

		// TODO process js script directly
		//printf("Warning :: getJavaScript :: JS object reference is null\n");

		js = getDelimitedStringContent(start,"(",")",len);
		//printf("JavaScript content = %s :: len = %d\n",js,len);

		if(js != NULL){
			#ifdef DEBUG
				printf("Debug :: getJavaScript :: Found JS content in object %s\n",obj->reference);
			#endif

			//printf("js = %s\n",js);

			pdf->testObjAnalysis->active_content ++;
			pdf->testObjAnalysis->js ++;
			unknownPatternRepetition(js, strlen(js),pdf,obj);
			findDangerousKeywords(js,pdf,obj);

		}



	}

	#ifdef DEBUG
		printf("\n\n");
	#endif

	return 1;

}

// This function get the JavaScript content in XFA form description (xml). // and analyze it
// return NULL if there is no JS content 
int getJSContentInXFA(char * stream, int size, struct pdfObject * obj, struct pdfDocument * pdf){
	//TODO

	char * start = NULL;
	char * end = NULL;
	char * js_content = NULL;
	int len = 0;
	int rest = 0;
	//int rest_size = 0;
	char * script_balise = NULL;
	//char * script_balise_end = NULL;
	char * tmp = NULL;
	int found  = 0;



	// get the 

	end = stream;
	rest = size;

	while((start = searchPattern(end,"<script",7,rest)) != NULL && rest > 0){

		#ifdef DEBUG
			printf("Debug :: getJSContentInXFA :: javascript content found in %s\n",obj->reference);
		#endif

		rest = (int)(start-stream);
		rest = size - rest;
		end = start;
		len = 0;
		while(end[0] != '>' && len<=rest ){
			end ++;		
			len ++;
		}

		script_balise = (char*)calloc(len+1,sizeof(char));
		script_balise[len]= '\0';
		memcpy(script_balise,start,len);
		//printf("script_balise = %s\n",script_balise);

		//TODO :: Check the keyword javascript 

		// save the script start ptr
		tmp = start;

		// search the </script> balise
		rest = (int)(end-stream);
		rest = size - rest;

		start = searchPattern(end,"</script",8,rest);

		if(start == NULL){
			return -1;
		}

		rest = (int)(start-stream);
		rest = size - rest;

		end = start;
		len = 0;
		while(end[0] != '>' && len<=rest ){
			end ++;		
			len ++;
		}

		//printf("end0 = %c\n",end[0] );


		len = (int)(end - tmp);
		len ++;


		js_content = (char*)calloc(len+1, sizeof(char));
		js_content[len]='\0';

		memcpy(js_content,tmp,len);

		//printf("Debug :: getJSContentInXFA :: js_content = %s\n",js_content);
		found = 1;

		// Analyze the js content
		unknownPatternRepetition(js_content,len,pdf,obj);
		findDangerousKeywords(js_content,pdf,obj);


	
		rest = (int)(start-stream);
		rest = size - rest;


		free(script_balise);
		free(js_content);
		script_balise = NULL;
		js_content  = NULL;

		//printf("\n\n");


	}

	
	/*
	if(start == NULL){
		//printf("Debug :: getJSContentInXFA No js content script found in xfa xml :: %s\n",obj->reference );
		//printf("Debug :: getJSContentInXFA :: stream = %s\n",stream);

		return NULL;
	}*/

	if(found == 1){
		pdf->testObjAnalysis->js ++;
	}

	

	//printf("start = %s\n",start);

	


	return found;
}


// get Object
int getXFA(struct pdfDocument * pdf, struct pdfObject* obj){

	char * xfa = NULL;
	char * xfa_obj_ref = NULL;
	char * start = NULL;
	char * end = NULL;
	char * obj_list = NULL;
	//char * stream = NULL;
	int len = 0;
	int len2 = 0;
	struct pdfObject * xfa_obj = NULL;


	//printf("Analysing object :: %s\n",obj->reference);

	if( obj->dico == NULL ){
		//printf("No dictionary in object %s\n",obj->reference);
		return -1;
	}

	start = searchPattern(obj->dico, "/XFA" , 4 , strlen(obj->dico));

	/*if( strncmp(obj->reference,"5 0 obj",7) == 0 && strncmp(obj->reference,"5 0 obj",strlen(obj->reference)) == 0 ){
		printf("dico = %s\n",obj->dico);
	}*/

	if(start == NULL){
		//printf("No XFA entry detected in object dictionary %s\n",obj->reference);
		return 0;
	}

	//printf("XFA Entry in dictionary detected in object %s\n", obj->reference);
	//printf("dictionary = %s\n", obj->dico);

	start += 4;

	// If there is a space // todo put a while
	//printf("start[0] = %c\n",start[0]);
	if(start[0] == ' '){
		start ++;
	}

	//printf("start[0] = %c\n",start[0]);

	len = strlen(obj->dico) - (int)(start - obj->dico);
	//printf("len = %d\n",len);


	// Get xfa object references

	// If its a list get the content
	if(start[0] == '['){

		obj_list =  getDelimitedStringContent(start,"[", "]", len); 
		//printf("obj_list = %s\n",obj_list);


		end = obj_list;
		len2 = strlen(obj_list);


		// get XFA object reference in array ::
		while( (xfa_obj_ref = getIndirectRefInString(end, len2)) ){

			//printf("xfa_obj_ref = %s\n",xfa_obj_ref);

			end = searchPattern(end, xfa_obj_ref , 4 , len2); // change value 4
			end += strlen(xfa_obj_ref) - 2;

			len2 = (int)(end - obj_list);
			len2 = strlen(obj_list) - len2;

			// get xfa object 
			xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);

			if(xfa_obj == NULL){
				#ifdef DEBUG
					printf("Error :: getXFA :: Object %s not found\n",xfa_obj_ref);
				#endif
				continue;
			}

			// Decode object stream			
			if(xfa_obj->filters != NULL){		
				decodeObjectStream(xfa_obj);			
			}
		

			// get xfa content
			if(xfa_obj->decoded_stream != NULL ){
				xfa = xfa_obj->decoded_stream;
			}else{
				xfa = xfa_obj->stream;
			}

			if(xfa != NULL){
				//printf("XFA content = %s\n",xfa);

				getJSContentInXFA(xfa,strlen(xfa),xfa_obj,pdf);

				#ifdef DEBUG
					printf("Found XFA content in object %s\n",xfa_obj_ref);
				#endif
				// Analyze xfa content
				//unknownPatternRepetition(xfa, strlen(xfa),pdf, xfa_obj);
				//findDangerousKeywords(xfa,pdf,xfa_obj);
				pdf->testObjAnalysis->active_content ++;
				pdf->testObjAnalysis->xfa ++;
			}else{
				#ifdef DEBUG
					printf("Warning :: Empty XFA content in object %s\n",xfa_obj_ref);
				#endif
			}


		}

	}else{
				
		len2 = (int)(start - obj->dico);
		len2 = strlen(obj->dico) - len2;


		xfa_obj_ref = getIndirectRefInString(start, len2);

		if(xfa_obj_ref == NULL){
			#ifdef DEBUG
				printf("Error :: getXFA :: get xfa object indirect reference failed\n");
			#endif
			return -1;
		}

		//printf("xfa_obj_ref = %s\n",xfa_obj_ref);

		xfa_obj =  getPDFObjectByRef(pdf, xfa_obj_ref);

		if(xfa_obj == NULL){
			#ifdef DEBUG
				printf("Error :: getXFA :: Object %s not found\n",xfa_obj_ref);
			#endif
			return -1;
		}

		// Decode object stream			
		if(xfa_obj->filters != NULL){		
			decodeObjectStream(xfa_obj);			
		}

		// get xfa content
		if(xfa_obj->decoded_stream != NULL ){
			xfa = xfa_obj->decoded_stream;
		}else{
			xfa = xfa_obj->stream;
		}

		if(xfa != NULL){
			#ifdef DEBUG
				printf("Found XFA content in object %s\n",xfa_obj_ref);
			#endif
			//printf("XFA content = %s\n",xfa);	

			getJSContentInXFA(xfa,strlen(xfa),xfa_obj,pdf);
			// TODO Analyze xfa content
			//unknownPatternRepetition(xfa, strlen(xfa), pdf, xfa_obj);
			//findDangerousKeywords(xfa,pdf,xfa_obj);
			pdf->testObjAnalysis->active_content ++;
			pdf->testObjAnalysis->xfa ++;
		}else{
			#ifdef DEBUG
				printf("Warning :: Empty XFA content in object %s\n",xfa_obj_ref);
			#endif
		}

		free(xfa_obj_ref);

	}

	
	#ifdef DEBUG
		printf("\n\n");
	#endif
	
	return 1;

}



// This function get Embedded file content and analyze it ; returns 0 ther is no embedded file
int getEmbeddedFile(struct pdfDocument * pdf , struct pdfObject* obj){

	char * ef = NULL;
	struct pdfObject * ef_obj = NULL;
	char * ef_obj_ref = NULL;
	char * start = NULL;
	//char * isDico = NULL;
	//char * end = NULL;
	int len = 0;
	//int ef_len 

	//printf("Analysing object :: %s\n",obj->reference);

	// verif params
	if(obj->dico == NULL || obj->type == NULL){
		return 0;
	}

	// Get by Type or by Filespec (EF entry)

	if( strncmp(obj->type,"/EmbeddedFile",13) == 0){


		// Decode object stream			
		if(obj->filters != NULL){		
			decodeObjectStream(obj);			
		}

		if(obj->decoded_stream != NULL ){
			ef = obj->decoded_stream;
		}else{
			ef = obj->stream;
		}

		if(ef != NULL){
			#ifdef DEBUG
				printf("Found EmbeddedFile object %s\n",obj->reference);
			#endif
			//printf("ef content = %s\n",ef);
			// TODO Process to ef stream content analysis.
			//unknownPatternRepetition(ef, strlen(xfa), pdf, xfa_obj);
			//findDangerousKeywords(xfa,pdf,obj);
			
		}else{
			#ifdef DEBUG
				printf("Warning :: Empty EF stream content in object %s\n",obj->reference);
			#endif
		}


	}


	
	if( strncmp(obj->type,"/Filespec",9) == 0){

		// Get EF entry in dico
		start = searchPattern(obj->dico, "/EF" , 3 , strlen(obj->dico));

		if(start == NULL){
			//printf("No EF detected in object %s\n",obj->reference);
			return 0;
		}

		#ifdef DEBUG
			printf("Found EmbeddedFile in file specification %s\n",obj->reference);
		#endif

		start += 3;

		// For white spaces
		while(start[0] == ' '){
			start ++;
		}


		// The case <</EF <</F 3 0 R>>
		//isDico = searchPattern
		#ifdef DEBUG
			printf("start[0] = %c\n",start[0]);
		#endif

		if(start[0] == '<' && start[1] == '<'){

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) - len;

			ef_obj_ref = obj->reference;
			ef_obj = obj;



		}else{

			len = (int)(start - obj->dico);
			len = strlen(obj->dico) - len;
			// get indirect ref of the 
			ef_obj_ref = getIndirectRef(start,len);
			//printf("ef_obj_ref = %s\n",ef_obj_ref);
			ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);

		}



		

		if(ef_obj != NULL){


			if(ef_obj->dico == NULL){
				#ifdef DEBUG
					printf("Warning :: No dictionary found in object %s\n",ef_obj_ref);
				#endif
				return -1;
			}
			// Get the /F entry in the dico
			start = searchPattern(ef_obj->dico, "/F" , 2 , strlen(ef_obj->dico));

			if(start == NULL){
				//printf("No EF detected in object %s\n",obj->reference);
				return 0;
			}

			start += 2;

			// For white spaces
			while(start[0] == ' '){
				start ++;
			}

			len = (int)(start - ef_obj->dico);
			len = strlen(ef_obj->dico) - len;

			ef_obj_ref = getIndirectRef(start,len);
			#ifdef DEBUG
				printf("EF_obj_ref = %s\n",ef_obj_ref);
			#endif


			ef_obj = getPDFObjectByRef(pdf,ef_obj_ref);

			if(ef_obj != NULL){

				// Decode object stream			
				if(ef_obj->filters != NULL){		
					decodeObjectStream(ef_obj);			
				}

				if(ef_obj->decoded_stream != NULL ){
					ef = ef_obj->decoded_stream;
				}else{
					ef = ef_obj->stream;
				}

				if( ef != NULL){
					#ifdef DEBUG
						printf("Found EmbeddedFile object %s\n",ef_obj_ref);
					#endif
					//printf("ef content = %s\n",ef);
					// TODO Process to ef stream content analysis.
				}else{
					#ifdef DEBUG
						printf("Warning :: Empty EF stream content in object %s\n",obj->reference);
					#endif
				}

			}else{
				#ifdef DEBUG
					printf("Warning :: getEmbeddedFile :: object not found %s\n",ef_obj_ref);
				#endif
			}



		}else{
			#ifdef DEBUG
				printf("Warning :: getEmbeddedFile :: object not found %s\n",ef_obj_ref);
			#endif
		}
		

	}

	/*if(ef != NULL){
		ptr2_len > pattern_size
	}*/

	if(ef != NULL){

		pdf->testObjAnalysis->active_content ++;
		pdf->testObjAnalysis->ef ++;
		// Analysi
	}



	return 1;
}


// This function get the Info obejct
int getInfoObject(struct pdfDocument * pdf){

	char * info = NULL;
	char * info_ref = 0;
	struct pdfObject * info_obj = NULL;
	char * start = NULL;
	//char * end = NULL;
	int len = 0;
	struct pdfTrailer* trailer = NULL;
	int res = 0;


	// Get the trailer
	if(pdf->trailers == NULL){
		#ifdef DEBUG
			printf("Warning :: getInfoObject :: No trailer found !");
		#endif
		return -1;
	}

	trailer = pdf->trailers;

	while(trailer != NULL){


		//printf("trailer content = %s\n",trailer->content );
		//printf("trailer size = %d\n",strlen(trailer->content));

		if(trailer->content == NULL){
			#ifdef DEBUG
				printf("Error :: getInfoObject :: Empty trailer content\n");
			#endif
			trailer = trailer->next;
			continue;
		}

		start = searchPattern(trailer->content, "/Info" , 5 , strlen(trailer->content));

		if(start == NULL){
			#ifdef DEBUG
				printf("No /Info entry found in the trailer dictionary\n");
			#endif
			return 0;
		}


		start += 5; // skip "/Info"

		len = (int)(start - trailer->content);
		len = strlen(trailer->content) - len;

		info_ref = getIndirectRefInString(start,len);

		#ifdef DEBUG
			printf("info_ref =  %s\n", info_ref);
		#endif

		info_obj = getPDFObjectByRef(pdf, info_ref);

		if(info_obj == NULL){
			#ifdef DEBUG
				printf("Warning :: getInfoObject :: Info object not found %s\n", info_ref);
			#endif
			return 0;
		}

		if(info_obj->dico != NULL){
			info = info_obj->dico;
			#ifdef DEBUG
				printf("info = %s\n",info);
			#endif

			// TODO analyze the content
			res = unknownPatternRepetition(info, strlen(info), pdf, info_obj);
			if(res > 0){
				#ifdef DEBUG
					printf("Warning :: getInfoObject :: found potentially malicious /Info object %s\n",info_ref);
				#endif
				pdf->testObjAnalysis->dangerous_keyword_high ++; // TODO find another variable for this test :: MALICIOUS_INFO_OBJ
			}

			res = findDangerousKeywords(info,pdf,info_obj);
			if(res > 0){
				#ifdef DEBUG
					printf("Warning :: getInfoObject :: found potentially malicious /Info object %s\n",info_ref);
				#endif
				pdf->testObjAnalysis->dangerous_keyword_high ++;
			}

		}else{
			#ifdef DEBUG
				printf("Warning :: getInfoObject :: Empty dictionary in info object :: %s\n",info_ref);
			#endif
		}


		trailer = trailer->next;
		free(info_ref);

		#ifdef DEBUG
		printf("\n\n");
		#endif

	}

	return 1;
}



// This function detects potentially malicious URI.
int analyzeURI(char * uri, struct pdfDocument * pdf, struct pdfObject * obj){

	#ifdef DEBUG
		printf("\tTODO... URI analysis :: %s\n", uri);
	#endif
	
	// Path traveral detection.
	// Malicious uri detection

	if(pdf == NULL || obj == NULL)
		return -1;


	return 0;
}



//This function get the URI defined in the object and analyze it
int getURI(struct pdfDocument * pdf, struct pdfObject * obj){


	char * start = NULL;
	char * end = NULL;
	char * uri = NULL;
	int len = 0;




	if(obj == NULL || pdf == NULL){
		#ifdef DEBUG
			printf("Error :: getURI :: Bad (null) parameters\n");
		#endif
		return -1;
	}

	if(obj->dico == NULL){
		return 0;
	}

	// get the URI entry in the dico
	//start = searchPattern(obj->dico,"/URI",4,strlen(obj->dico));

	end= obj->dico;
	len = strlen(obj->dico);

	while( (start = searchPattern(end,"/URI",4,len)) != NULL ){

		start += 4;

		// skip white spaces
		while(start[0] == ' '){
			start ++;
		}

		//printf("start0 = %c\n",start[0]);
		end = start;

		if(start[0] != '('){
			continue;
		}


		len = (int)(start - obj->dico);
		len = strlen(obj->dico) - len;

		uri = getDelimitedStringContent(start,"(",")", len);
		//printf("uri = %s\n",uri);

		// Analyze uri
		if (uri != NULL) {
			analyzeURI(uri, pdf, obj);
			free(uri);
			uri = NULL;
		}
		

	}

	return 1;
}


// Get Suspicious actions 
int getActions(struct pdfDocument * pdf, struct pdfObject * obj){


	char * start = NULL;
	//char * end = NULL;
	//char * openAction_dico = NULL;
	//int len = 0;
	int dico_len = 0;


	// Check parameters
	if(pdf == NULL || obj == NULL){
		return -1;
	}

	if(obj->dico == NULL){
		return -1;
	}

	dico_len = strlen(obj->dico);


	// get OpenAction dictionary
	//start = searchPattern(obj->dico,);


	// get Launch actions
	start = searchPattern(obj->dico,"/Launch",7,dico_len);
	if(start != NULL){

		#ifdef DEBUG
			printf("Warning :: getActions :: Found /Launch action in object %s\n",obj->reference);
		#endif

		pdf->testObjAnalysis->dangerous_keyword_high ++;
	}


	// get Actions :: Launch - GoToE - GoToR - 

	return 0;
}



// This function remove whites space in a given stream
char * removeWhiteSpace(char * stream, int size){

	char * out = NULL;
	char * start = NULL;
	char * end = NULL;
	int len = 0;
	int len_saved = 0;
	int len2 = 0;
	int count = 0;
	int i = 0;
	// count white spaces

	

	for(i = 0; i<size; i++){
		if(stream[i] == '\n' || stream[i] == '\r' || stream[i] == '\n' || stream[i] == ' ' ){
			count ++;
		}

	}

	//printf("There are %d white space in this stream\n",count);

	// calc the new len
	len = size - count;
	len_saved = len;
	out = (char*)calloc(len+1,sizeof(char));
	out[len] = '\0';

	

	start = stream;
	end = start;
	len = 0;

	//printf("end0 = %c\n",end[0]);

	
	
	while( len < (size - count) ){

		
		len2 = len;
		while(end[0] != '\n' && end[0] != '\r' && end[0] != '\n' && end[0] != ' ' && len2 < (size-count)){
			end ++;
			len2 ++;
		}

		len2 = (int)(end-start);
		//memcpy(out,start,len2);		
		os_strncat(out,len_saved+1,start, len2);
		len += len2;

		// skip white spaces
		start = end;
		while(start[0] == '\n' || start[0] == '\r' || start[0] == '\n' || start[0] == ' ' ){
			start ++;
		}

		end = start;
	}

	

	//printf("out = %s\n",out);
	return out;
}




// This function detects when a string (unknown) is repeted in the stream with a high frequency.
// TODO remove white spaces to improve results.
int unknownPatternRepetition(char * stream, int size, struct pdfDocument * pdf, struct pdfObject * obj){

	//int ret = 0;
	char * pattern = NULL;
	int pattern_size = 5;
	//char * start = NULL;
	//char * end = NULL;
	//char * len = 0;
	int rep = 0;
	int lim_rep = 150;
	//int off = 0;
	char * ptr = NULL;
	char * ptr_bis = NULL;
	int ptr_len = 0;
	int ptr2_len = 0;
	char * tmp = NULL;
	char * whithout_space = NULL;
	time_t start_time, end_time;
	double time_elapsed = 0;
	int time_exceeded = 6;

	/*
	if(pdf != NULL){
		return 0;
	}*/

	//printf("Debug :: Stream = %s\n",stream);

	time(&start_time);
	//char * test = "Bonjour je suis une stream de test";


	//printf("\n\nDebug :: unknownPatternRepetition \n");

	// remove white space in stream ? 
	
	//whithout_space =  removeWhiteSpace(test,strlen(test));

	//printf("whithout_space = %s\n",whithout_space);

	
	whithout_space =  removeWhiteSpace(stream,size);
	

	//printf("whithout_space = %s\n",ptr);
	
	ptr = whithout_space;
	ptr_len = strlen(whithout_space);

	ptr_bis = whithout_space;
	ptr2_len = strlen(whithout_space);


	/*
	ptr = stream;
	ptr_len = strlen(stream);

	ptr_bis = stream;
	ptr2_len = strlen(stream);
	*/


	/*
	printf("ptr[0] = %c\n", ptr[0] );
	printf("ptr[1] = %c\n", ptr[1] );
	printf("ptr[2] = %c\n", ptr[2] );
	printf("ptr[3] = %c\n", ptr[3] );
	printf("ptr[4] = %c\n", ptr[4] );
	*/



	// get pattern
	while( ptr_len > pattern_size && (pattern = getPattern(ptr,pattern_size,ptr_len)) != NULL ){

		rep = 0;
		//printf("pattern = %s :: ptr_len = %d:: ptr = %d\n",pattern,ptr_len, ptr);
		ptr ++;
		ptr_len --;

		// search occurrences
		ptr_bis = ptr+5;
		ptr2_len = ptr_len-5;



		while( ptr2_len > pattern_size && (tmp = getPattern(ptr_bis,pattern_size,ptr2_len)) != NULL){

			//printf("tmp = %s\n",tmp);

			if(strncmp(pattern,tmp,pattern_size) == 0){
				rep ++;
			}

			
			if(rep > lim_rep){
				#ifdef DEBUG
					printf("Warning :: unknownPatternRepetition :: Found pattern repetition in object %s\n",obj->reference);
				#endif
				#ifdef DEBUG
					printf("pattern = %s :: rep = %d--\n\n",pattern,rep);
				#endif
				pdf->testObjAnalysis->pattern_high_repetition ++;
				free(pattern);
				free(tmp);
				return rep;
			}

			ptr_bis ++;
			ptr2_len --;

			time(&end_time);

			time_elapsed = difftime(end_time,start_time);
			
			//printf("\ntmp = %s, %.2lf sec \n",tmp,time_elapsed);

			if(time_elapsed > time_exceeded){
				#ifdef DEBUG
					printf("Warning :: unknownPatternRepetition :: Time Exceeded while analyzing object %s content\n",obj->reference );
				#endif
				pdf->testObjAnalysis->time_exceeded++;
				free(whithout_space);
				free(tmp);
				free(pattern);
				return 0;
			}


			free(tmp);


			
		}

		

		//printf("heyhey :: %d :: %d :: %d :: \n",ptr2_len,pattern_size,rep, );

		/*
		if(rep > 100){
			return 100;
		}
		*/

		time(&end_time);

		time_elapsed = difftime(end_time,start_time);

		//printf("heyhoo :: %d\n",time_elapsed);
		//printf("\nExecution time : %.2lf sec \n",time_elapsed);

		if(time_elapsed > 5){
			#ifdef DEBUG
				printf("Warning :: unknownPatternRepetition :: Time Exceeded while analyzing object %s content\n",obj->reference );
			#endif
			pdf->testObjAnalysis->time_exceeded++;
			free(whithout_space);
			free(pattern);
			return 0;
		}

		free(pattern);

	}

	free(whithout_space);


	return 0 ;

}




// Find a potentially dangerous pattern in the given stream; return High = 3 ; Medium = 2 ; Low = 1 ; none = 0
int findDangerousKeywords(char * stream , struct pdfDocument * pdf, struct pdfObject * obj){

	int i = 0;
	char * high_keywords[] = {"HeapSpray","heap","spray","hack","shellcode", "shell", "pointers", "byteToChar", "system32", "payload", "console"};
	int num_high = 10;
	int num_medium = 9;
	int num_low = 1;
	char * medium_keywords[] = {"substring", "split", "eval", "addToolButton", "String.replace", "unescape", "exportDataObject", "StringFromChar", "util.print"};
	char * low_keywords[] = {"toString"};
	char * start = NULL;
	int len = 0;
	int unicode_count = 0;
	char* unicode = NULL;
	int ret = 0;


	//printf(":: Searching Dangerous Keywords in %s\n",obj->reference);

	//printf("\n\n content to analyze = %s\n",stream);

	//stream = "hackbonjour";

	//printf("Test :: %s\n",high_keywords[i]);

	for(i = 0; i< num_high ; i++){

		//printf("Test :: %s\n",high_keywords[i]);
		//if(strnstr(stream,high_keywords[i],strlen(high_keywords[i])) != NULL ){
		
		if(searchPattern(stream,high_keywords[i],strlen(high_keywords[i]),strlen(stream)) != NULL ){
			#ifdef DEBUG
				printf("Warning :: findDangerousKeywords :: High dangerous keyword (%s) found in object %s\n",high_keywords[i], obj->reference);
			#endif
			pdf->testObjAnalysis->dangerous_keyword_high ++;
			ret = 3;
		}

	}

	// find unicode strings
	//stream = "%ufadeqsdqdqsdqsdqsdqsdqsd";
	start = stream ;
	len = strlen(stream);

	unicode = (char*)calloc(6,sizeof(char));

	while( len >= 6 && (start = getUnicodeInString(start,len)) != NULL && unicode_count < 50 ){


		memcpy(unicode, start, 6);
		#ifdef DEBUG
			printf("Warning :: findDangerousKeywords :: Found unicode string %s in object %s\n",unicode,obj->reference);
		#endif

		unicode_count ++ ;
		start ++;

		len = (int)(start - stream);
		len = strlen(stream) - len;


		//printf("len = %d\n",len);



	}

	if(unicode_count > 10){
		#ifdef DEBUG
			printf("Warning :: findDangerousKeywords :: Unicode string found in object %s\n", obj->reference);
		#endif
		pdf->testObjAnalysis->dangerous_keyword_high ++;
		ret = 3;
	}
	

	//printf("stream = %s\n",stream);

	for(i = 0; i< num_medium ; i++){

		//printf("Test :: %s\n",medium_keywords[i]);
		//if(strnstr(stream,medium_keywords[i],strlen(medium_keywords[i])) != NULL ){
		if(  searchPattern(stream,medium_keywords[i],strlen(medium_keywords[i]),strlen(stream)) != NULL ){
			#ifdef DEBUG
				printf("Warning :: findDangerousKeywords :: Medium dangerous keyword (%s) found in object %s\n",medium_keywords[i], obj->reference);
			#endif
			pdf->testObjAnalysis->dangerous_keyword_medium ++;			
			ret=  2;
		}

	}


	for(i = 0; i< num_low ; i++){

		//printf("Test :: %s\n",medium_keywords[i]);
		//if(strnstr(stream,medium_keywords[i],strlen(medium_keywords[i])) != NULL ){
		if(  searchPattern(stream,low_keywords[i],strlen(low_keywords[i]),strlen(stream)) != NULL ){
			#ifdef DEBUG
				printf("Warning :: findDangerousKeywords :: Low dangerous keyword (%s) found in object %s\n",low_keywords[i], obj->reference);
			#endif
			pdf->testObjAnalysis->dangerous_keyword_low ++;			
			ret = 1;
		}

	}

	free(unicode);
	return ret;


}

// This function get all potentially dangerous content (js, embedded files, dangerous pattern) form. and return 1 if any of 0 if not
int getDangerousContent(struct pdfDocument* pdf){

	int res = 0;
	struct pdfObject * obj = NULL;

	if( pdf == NULL || pdf->objects == NULL ){
		#ifdef DEBUG
		printf("Error :: getDangerousContent :: Null parameters\n");
		#endif
		return -1;
	}

	#ifdef DEBUG
	printf("\n-------------------------\n");
	printf("---  OBJECT ANALYSIS  ---\n");
	printf("-------------------------\n\n");
	#endif

	//printf("pdfobjects %d\n",pdf->objects);
	obj = pdf->objects;


	while(obj != NULL){

		//printf("Analysing object :: %s\n",obj->reference);

		getActions(pdf,obj);		

		getJavaScript(pdf,obj);
		
		getXFA(pdf,obj);
		
		getEmbeddedFile(pdf,obj);
		
		getURI(pdf,obj);

		// next object
		obj = obj->next;
		
	}
	
	getInfoObject(pdf);

	return res;

}