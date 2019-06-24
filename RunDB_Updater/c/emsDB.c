#include <fcgi_stdio.h>
//#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <uriparser/Uri.h>

int main(void) {
	char* quoteFormatComma[2];
	quoteFormatComma[0] = "\"%s\",";
	quoteFormatComma[1] = "%s,";
	char* quoteFormat[2];
	quoteFormat[0] = "\"%s\"]";
	quoteFormat[1] = "%s]";
	MYSQL mysql;
	mysql_init(&mysql);
	my_bool reconnect = 1;
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, &reconnect);
	if (!mysql_real_connect(&mysql,"localhost","ucn","slmmmtqom","EmsNov2016",0,NULL,0)) {
		//printf("Cannot Connect!\n");
		return 1;
	}
	//char query[1024];

	while(FCGI_Accept() >= 0) {
//			int k = 0; while(k == 0) { k+= 1;
		int i = 0;
		int found_query = 0;
		if(mysql_ping(&mysql)) {
			return 2;
			//printf("Bad Ping!\n");
		}

		int query_len = strlen(getenv("QUERY_STRING"));
		char* query = malloc(sizeof(char) * (query_len+5));
		snprintf(query, query_len+4, "?%s", getenv("QUERY_STRING"));
		/*char* q = "query=SELECT+name%2C+datatable%2C+instrumentID%2C+channel%2C+field%2C+calibrationtable%2C+calibrationtype+FROM+Channels+ORDER+BY+instrumentID+ASC%2C+channel+ASC";
		int query_len = strlen(q);
		char* query = malloc(sizeof(char) * (query_len+5));
		snprintf(query, query_len+4, "?%s", q);*/

		UriParserStateA state;
		UriUriA uri;
		state.uri = &uri;
		if (uriParseUriA(&state, query) != URI_SUCCESS) {
			printf("Content-type: application/json\r\n\r\n{}\n");
			uriFreeUriMembersA(&uri);
			continue;
			/* Failure */
		}
		UriQueryListA * queryList;
		int itemCount;
		if (uriDissectQueryMallocA(&queryList, &itemCount, uri.query.first,
					uri.query.afterLast) != URI_SUCCESS) {
			printf("Content-type: application/json\r\n\r\n{}\n");
			uriFreeUriMembersA(&uri);
			uriFreeQueryListA(queryList);
			continue;
			/* Failure */
		}
		for(i = 0; i < itemCount; i++) {
			if(!strcmp(queryList[i].key, "query")) {
				found_query = 1;
				break;
			}
		}
		if(found_query == 0) {
			printf("Content-type: application/json\r\n\r\n{}\n");
			uriFreeUriMembersA(&uri);
			uriFreeQueryListA(queryList);
			continue;
		}
		mysql_query(&mysql, queryList[i].value);

		//mysql_query(&mysql, "SELECT time, rate FROM labjack");
		//printf("Content-type: text/html\r\n\r\n%s\n", queryList[i].value);
		//return (0);
		//continue;

		if(mysql_field_count(&mysql) < 1) {
			printf("Content-type: application/json\r\n\r\n{}\n");
			uriFreeUriMembersA(&uri);
			uriFreeQueryListA(queryList);
			continue;
		}
		MYSQL_RES *res = mysql_store_result(&mysql);
		int numFields = mysql_num_fields(res);

		MYSQL_FIELD *fields = mysql_fetch_fields(res);
		int* formats = malloc(sizeof(int) * numFields);
		printf("Content-type: application/json\r\n\r\n");
		printf("{\"series\":[");
		for(i = 0; i < numFields-1; i++) {
			printf("\"%s\",", fields[i].name);
			switch(fields[i].type) {
				case MYSQL_TYPE_TINY:
				case MYSQL_TYPE_SHORT:
				case MYSQL_TYPE_LONG:
				case MYSQL_TYPE_INT24:
				case MYSQL_TYPE_LONGLONG:
				case MYSQL_TYPE_DECIMAL:
				case MYSQL_TYPE_NEWDECIMAL:
				case MYSQL_TYPE_FLOAT:
				case MYSQL_TYPE_DOUBLE:
					formats[i] = 1;
					break;
				default:
					formats[i] = 0;
			}
		}
		switch(fields[i].type) {
			case MYSQL_TYPE_TINY:
			case MYSQL_TYPE_SHORT:
			case MYSQL_TYPE_LONG:
			case MYSQL_TYPE_INT24:
			case MYSQL_TYPE_LONGLONG:
			case MYSQL_TYPE_DECIMAL:
			case MYSQL_TYPE_NEWDECIMAL:
			case MYSQL_TYPE_FLOAT:
			case MYSQL_TYPE_DOUBLE:
				formats[i] = 1;
				break;
			default:
				formats[i] = 0;
		}
		printf("\"%s\"],\n", fields[i].name);
		printf("\"data\":[");
		MYSQL_ROW row = mysql_fetch_row(res);
		while(row != NULL) {
			printf("[");
			for(i = 0; i < numFields-1; i++) {
				if(row[i] != NULL) {
					printf(quoteFormatComma[formats[i]], row[i]);
				}
				else {
					printf("null,");
				}
			}
			if(row[i] != NULL) {
				printf(quoteFormat[formats[i]], row[i]);
			}
			else {
				printf("null]");
			}
			row = mysql_fetch_row(res);
			if(row != NULL) {
				printf(",");
			}
		}
		printf("]}\n");
		mysql_free_result(res);
		uriFreeUriMembersA(&uri);
		uriFreeQueryListA(queryList);
		free(query);
		free(formats);
	}
	return 0;
}
