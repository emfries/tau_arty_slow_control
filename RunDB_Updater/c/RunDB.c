#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <string.h>

enum ACTUATOR {
	OTHER,
	GATEVALVE,
	BEAM,
	CLEANER,
	GIANTCLEANER,
	BUTTERFLY,
	DAGGER,
	TRAPDOOR,
	HOLD,
	COUNT
};

void removeCommentAndNewline(char* line);

char* strip(char* str);

int addEvent(MYSQL* mysql, int run_number, int time, char* actuator, char* value);

int addCalcEvent(MYSQL* mysql, int run_number, int storageTime, int giantclUpTime, int clUpTime, int countStartTime, int fillTime, int numDips, int giantCleanPos, int dagFillPos, int dagCleanPos, int dagHoldPos);

enum ACTUATOR getEnum(char * actuator);

int main(int argc, char** argv) {
	MYSQL mysql;
	mysql_init(&mysql);
	my_bool reconnect = 1;
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, &reconnect);
	if (!mysql_real_connect(&mysql,"localhost","daq","iucf1234","runlog_2016_2017",0,NULL,0)) {
		fprintf(stderr, "Cannot Connect!\n");
		return 1;
	}
	
	if(argc < 3) {
		fprintf(stderr, "Error! Usage: RunDB_Add filename run_number\n");
		return 2;
	}
	
	FILE* in;
	in = fopen(argv[1], "r");
	
	int runno = atoi(argv[2]);
	
	char query [1024];
	snprintf(query, 1024, "SELECT * FROM runFileLog WHERE run_number = %d", runno);
	mysql_query(&mysql, query);
	MYSQL_RES* res = mysql_store_result(&mysql);
	if(res == NULL) {
		fprintf(stderr, "NULL MySQL result!\n");
		return 3;
	}
	if(mysql_num_rows(res) != 0) {
		fprintf(stderr, "Entry for run %d already exists!\n", runno);
		return 4;
	}
	mysql_free_result(res);
	
	char* buf = NULL;
	size_t len = 0;
	
	char* entry;
	char* keyOrVal;
	char* lineSavPtr;
	char* keyvalSavPtr;
	
	char* strippedKey;
	char* strippedVal;
	
	
	//giantclUpTime, clUpTime, countStartTime, fillTime, numDips, giantCleanPos, dagFillPos, dagCleanPos, dagHoldPos
	int giantclUpTime = -1;
	int clUpTime = -1;
	int holdTime = -1;
	int countStartTime = -1;
	int fillTime = -1;
	int giantCleanPos = -1;
	int dagFillPos = -1;
	int dagCleanPos = -1;
	int dagHoldPos = -1;
	int dagPos = -1;
	int lastDagPos = -1;
	int numDips = 0;
	int afterHold = 0;
	
	int time = -1;
	
	while(getline(&buf, &len, in) > 0) {
		if(buf[0] != '#') {
			removeCommentAndNewline(buf);
			entry = strtok_r(buf, ",", &lineSavPtr);
			while(entry != NULL) {
				strippedKey = NULL;
				strippedVal = NULL;

				keyOrVal = strtok_r(entry, "=", &keyvalSavPtr);
				if(keyOrVal != NULL) {
					strippedKey = strip(keyOrVal);
				}

				keyOrVal = strtok_r(NULL, "=", &keyvalSavPtr);
				if(keyOrVal != NULL) {
					strippedVal = strip(keyOrVal);
				}
	
				keyOrVal = strtok_r(NULL, "=", &keyvalSavPtr);
				if(keyOrVal == NULL) {
					if(time == -1 && !strcmp(strippedKey, "T")) {
						time = atoi(strippedVal);
					}
					else if(time >= 0 && strlen(strippedKey) > 0 && strlen(strippedVal) > 0) {
						//printf("Time: %d Actuator: %s Value: %s\n", time, strippedKey, strippedVal);
						addEvent(&mysql, runno, time, strippedKey, strippedVal);
					}
					if(!strcmp(strippedKey, "COUNT")) { countStartTime = time; dagHoldPos = dagPos; afterHold = 1;}
					if(!strcmp(strippedKey, "DAGGER")) { lastDagPos = dagPos; dagPos = atoi(strippedVal);  if(afterHold && dagPos < lastDagPos) { numDips += 1; } }
					if(!strcmp(strippedKey, "GIANTCLEANER")) { if(atoi(strippedVal) < 10387500) { giantCleanPos = atoi(strippedVal); } }
					if(!strcmp(strippedKey, "GIANTCLEANER")) { if(atoi(strippedVal) == 10387500) { giantclUpTime = time; } }
					if(!strcmp(strippedKey, "CLEANER")) { if(!strcmp(strippedVal, "UP")) { clUpTime = time; } }
					if(!strcmp(strippedKey, "GATEVALVE")) { if(!strcmp(strippedVal, "CLOSED")) { fillTime = time; } }
					if(!strcmp(strippedKey, "HOLD")) { holdTime = time; dagCleanPos = dagPos; }
					if(time == 0) { dagFillPos = dagPos; }
				}
				else {
					printf("More than 2 tokens in key value pair!\n");
				}

				if(strippedKey != NULL) { free(strippedKey); }
				if(strippedVal != NULL) { free(strippedVal); }
				entry = strtok_r(NULL, ",", &lineSavPtr);
			}
//			printf("Found line: %s", buf);
		}
		time = -1;
	}
	
	if(countStartTime > 0) {
		addCalcEvent(&mysql, runno, countStartTime-holdTime, giantclUpTime, clUpTime, countStartTime, fillTime, numDips, giantCleanPos, dagFillPos, dagCleanPos, dagHoldPos);
	}
	else {
		printf("Not blinded; not adding calculated moves!\n");
		//addCalcEvent(&mysql, runno, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	/*printf("giantclUpTime: %d\n", giantclUpTime);
	printf("holdTime: %d\n", holdTime);
	printf("countStartTime: %d\n", countStartTime);
	printf("storageTime: %d\n", countStartTime-holdTime);
	printf("fillTime: %d\n", fillTime);
	printf("giantCleanPos: %d\n", giantCleanPos);
	printf("dagFillPos: %d\n", dagFillPos);
	printf("dagCleanPos: %d\n", dagCleanPos);
	printf("dagHoldPos: %d\n", dagHoldPos);
	printf("numDips: %d\n", numDips + 1);*/
	
	free(buf);
	mysql_close(&mysql);
	return 0;
}
	
void removeCommentAndNewline(char* line) {
	int i = 0;
	int len = strlen(line);
	for(i = 0; i < len; i++) {
		if(line[i] == '#' || line[i] == '\n') {
			line[i] = '\0';
			break;
		}
	}
	return;
}
	
char* strip(char* str) {
	int start = 0;
	int end = 0;
	int i = 0;
	int len = strlen(str);
	int newLen = len;
	char* stripped;
	for(i = 0; i < len; i++) {
		if(str[i] != ' ') {
			start = i;
			break;
		}
	}
	for(i = len-1; i >= 0; i--) {
		if(str[i] != ' ') {
			end = i;
			break;
		}
	}
	newLen = (end-start) + 1;
	stripped = malloc(sizeof(char) * (newLen + 1));
	stripped[newLen] = '\0';
	for(i = start; i <= end; i++) {
		stripped[i-start] = str[i];
	}
	return stripped;
}

int addEvent(MYSQL* mysql, int run_number, int time, char* actuator, char* value) {
	char query[1024];
	snprintf(query, 1024, "INSERT INTO runFileLog VALUES (%d, %d, \"%s\", \"%s\")", run_number, time, actuator, value);
	if(mysql_ping(mysql)) {
		fprintf(stderr, "Could not reconnect!\n");
		exit(100);
	}
	//printf("%s\n", query);
	if(mysql_query(mysql, query)) {
		fprintf(stderr, "Could not execute query!\n %s\n", query);
		exit(101);
	}
	return 0;
}

int addCalcEvent(MYSQL* mysql, int run_number, int storageTime, int giantclUpTime, int clUpTime, int countStartTime, int fillTime, int numDips, int giantCleanPos, int dagFillPos, int dagCleanPos, int dagHoldPos) {
	char query[2048];
	snprintf(query, 2048, "INSERT INTO actuatorMovesCalc VALUES (%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", run_number, storageTime, giantclUpTime, clUpTime, countStartTime, fillTime, numDips, giantCleanPos, dagFillPos, dagCleanPos, dagHoldPos);
	if(mysql_ping(mysql)) {
		fprintf(stderr, "Could not reconnect!\n");
		exit(100);
	}
	//printf("%s\n", query);
	if(mysql_query(mysql, query)) {
		fprintf(stderr, "Could not execute query!\n %s\n", query);
		exit(101);
	}
	return 0;
}

//int addCalcTimes(MYSQL* mysql, int run_number, 

enum ACTUATOR getEnum(char * actuator) {
	//OTHER, GATEVALVE, BEAM, CLEANER, GIANTCLEANER, BUTTERFLY, DAGGER, TRAPDOOR, HOLD, COUNT
	switch(actuator[0]) {
		case 'B':
			switch (actuator[1]) {
				case 'E':
					return BEAM;
				case 'U':
					return BUTTERFLY;
			}
		case 'C':
			switch (actuator[1]) {
				case 'L':
					return CLEANER;
				case 'O':
					return COUNT;
			}
		case 'D':
			return DAGGER;
		case 'G':
			switch (actuator[1]) {
				case 'A':
					return GATEVALVE;
				case 'I':
					return GIANTCLEANER;
			}
		case 'H':
			return HOLD;
		case 'T':
			return TRAPDOOR;
	}
	return OTHER;
}
	
	
	
	/*if(mysql_ping(&mysql)) {
		return 2;
		//printf("Bad Ping!\n");
	}

	mysql_query(&mysql, "SELECT * FROM runlog");

	if(mysql_field_count(&mysql) < 1) {
		printf("NO RESULTS\n");
		return 3;
	}
	
	MYSQL_RES *res = mysql_store_result(&mysql);
	int numFields = mysql_num_fields(res);
	MYSQL_FIELD *fields = mysql_fetch_fields(res);
	for(i = 0; i < numFields-1; i++) {
		printf("\"%s\",", fields[i].name);
	}
	printf("\"%s\"\n", fields[i].name);
	MYSQL_ROW row = mysql_fetch_row(res);
	while(row != NULL) {
		for(i = 0; i < numFields-1; i++) {
			if(row[i] != NULL) {
				printf("%s,", row[i]);
			}
			else {
				printf("null,");
			}
		}
		if(row[i] != NULL) {
			printf("%s\n", row[i]);
		}
		else {
			printf("null\n");
		}
		row = mysql_fetch_row(res);
		if(row != NULL) {
			printf("\n");
		}
	}
	mysql_free_result(res);
	return 0;
}*/
